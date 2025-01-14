#include "Speedometer.h"

// If TRACE or STREAMING are #define'd, they're in Speedometer.h

#if TRACE
#if STREAMING
#include <Streaming.h>
#endif
#endif

// I2C addresses of sensors
#define SENS_A 0x2A
#define SENS_B 0x2B

// GPIO pins of sensor-enable outputs (used to set I2C addresses of sensors)
#define ENA_A 5
#define ENA_B 4

// GPIO pins of sensor interrupt inputs
#define INTR_A 3
#define INTR_B 2

// (Pointers to) sensor objects
Sensor* sensA;
Sensor* sensB;

// Conversion factor
#define MI_PER_KM (0.62137119224)

// Timeouts (msec)
#define TIMEOUT_2ND 45455
#define TIMEOUT_CLEAR 1500

// Flags set when interrupts occur
volatile bool readyA = false;
volatile bool readyB = false;

// Interrupt service routines
void isr_A() {
  readyA = true;
}
void isr_B() {
  readyB = true;
}

// Constructor
Speedometer::Speedometer(E_Scale s) : 
  StateMachine(5, true),    // 5 msec real-time period
  m_spacing(127),           // sensor spaing (mm)
  m_sense_when(0L),         // time mark at beginning of interval measurement
  m_speed(0.0),             // most recent speed (km/hr or mi/hr)
  m_scale(s),               // scale factor (87, 150, 160, ...)
  m_window(NULL),           // (pointer to) current range window
  m_metric(s == eJP),       // metric or imperial speed
  m_state(eClear),          // finite state machine current state
  m_detA(false),            // sensor A detect
  m_detB(false),            // sensor B detect
  m_triggered(false),       // emitters triggered
  m_updated(false)          // measured speed updated
{

  // Set interrupt GPIO pins to INPUT_PULLUP,
  // create Sensor objects, attach interrupts to GPIO pins

  pinMode(INTR_A, INPUT_PULLUP);
  sensA = new Sensor(SENS_A, ENA_A, INTR_A, &readyA);
  attachInterrupt(digitalPinToInterrupt(INTR_A), isr_A, RISING);
  
  pinMode(INTR_B, INPUT_PULLUP);
  sensB = new Sensor(SENS_B, ENA_B, INTR_B, &readyB);
  attachInterrupt(digitalPinToInterrupt(INTR_B), isr_B, RISING);

}

bool Speedometer::update() {

  // Time to update state machine?
  if (StateMachine::update()) {

    // If not triggered, ...
    if (!m_triggered) {
      // ... trigger both sensors to pulse emitters
      // and set triggered status.
      sensA->trigger();
      sensB->trigger();
      m_triggered = true;
    } else {
      // ... otherwise, have both sensors completed range measurement?
      if (sensA->is_ready() && sensB->is_ready()) {
        // If so, clear triggered status, read sensors, and determine
        // if either range counts as a valid detection.
        m_triggered = false;
        uint32_t distA = sensA->get_distance();
        uint32_t distB = sensB->get_distance();
        m_detA = m_window->within((uint8_t) distA);
        m_detB = m_window->within((uint8_t) distB);
#if TRACE
#if STREAMING
        Serial << (int) m_detA * 100 << " " << (int) m_detB * 100 << " "
          << distA << " " << distB << endl;
#else
        Serial.print((int) m_detA * 100);
        Serial.print(" ");
        Serial.print((int) m_detB * 100);
        Serial.print(" ");
        Serial.print(distA);
        Serial.print(" ");
        Serial.println(distB);
#endif
#endif
      }
    }

    // Finite state machine logic: Action taken on this pass through
    // loop() depends on current value of m_state.  If state change required,
    // m_state is updated in this pass but not acted upon until next pass.

    if (m_state == eClear) {

      // Waiting for detection on only one of two sensors.  Detect on both
      // from this state is spurious and will be rejected.
      
      if (m_detA && m_detB) {
        // Spurious double-detect
#if TRACE
#if STREAMING
        Serial << millis() << " CLEARING 1" << endl;
#else
        Serial.print(millis());
        Serial.println(" CLEARING 1");
#endif
#endif
        m_state = eClearing;
      } else if (m_detA && !m_detB) {
        // Detect on only sensor A, mark the time and change state
        // to "Sensed A".
        uint32_t now = millis();
#if TRACE
#if STREAMING
        Serial << now << " SENSA" << endl;
#else
        Serial.print(now);
        Serial.println(" SENSA");
#endif
#endif
        m_sense_when = now;
        m_state = eSenseA;
      } else if (!m_detA && m_detB) {
        // Detect on only sensor B, mark the time and change state
        // to "Sensed B".
        uint32_t now = millis();
#if TRACE
#if STREAMING
        Serial << now << " SENSB" << endl;
#else
        Serial.print(now);
        Serial.println(" SENSB");
#endif
#endif
        m_sense_when = now;
        m_state = eSenseB;
      }
      
    } else if (m_state == eSenseA) {

      // Detection on sensor A, now watch only for detection on sensor B.
      
      // How much time has elapsed since detect on A?
      uint32_t now = millis();
      uint32_t elapsed = now - m_sense_when;
      // If detection on B...
      if (m_detB) {
        // ... calculate speed from elapsed time and flag as updated.
        m_speed = calcScaleSpeed(elapsed);
        m_updated = true;
#if TRACE
#if STREAMING
        Serial << now << " UPDATED 1 " << elapsed << " " << m_speed << endl;
#else
        Serial.print(now);
        Serial.print(" UPDATED 1 ");
        Serial.print(elapsed);
        Serial.print(" ");
        Serial.println(m_speed);
#endif
#endif
        // Now begin timeout period.
        m_sense_when = now;
        m_state = eUpdated;
      } else {
        // ... otherwise, clear measuring of interval if we've waited too long.
        if (elapsed > TIMEOUT_2ND) {
          m_speed = 0.0;
#if TRACE
#if STREAMING
          Serial << now << " ACTIVE 1" << endl;
#else
          Serial.print(now);
          Serial.println(" ACTIVE 1");
#endif
#endif
          m_state = eActive;
        }
      }
      
    } else if (m_state == eSenseB) {
      
      // Detection on sensor B, now watch only for detection on sensor A.
      
      // How much time has elapsed since detect on B?
      uint32_t now = millis();
      uint32_t elapsed = now - m_sense_when;
      // If detection on A...
      if (m_detA) {
        /// ... calculate speed from elapsed time and flag as updated.
        m_speed = calcScaleSpeed(elapsed);
        m_updated = true;
#if TRACE
#if STREAMING
        Serial << now << " UPDATED 2 " << elapsed << " " << m_speed << endl;
#else
        Serial.print(now);
        Serial.print(" UPDATED 2 ");
        Serial.print(elapsed);
        Serial.print(" ");
        Serial.println(m_speed);
#endif
#endif
        // Now begin timeout period.
        m_sense_when = now;
        m_state = eUpdated;
      } else {
        // ... otherwise, clear measuring of interval if we've waited too long.
        if (elapsed > TIMEOUT_2ND) {
          m_speed = 0.0;
#if TRACE
#if STREAMING
          Serial << now << " ACTIVE 2" << endl;
#else
          Serial.print(now);
          Serial.println(" ACTIVE 2");
#endif
#endif
          m_state = eActive;
        }
      }
      
    } else if (m_state == eUpdated) {

      // Speed has been measured, waiting for display to be updated from
      // sketch's loop() function.
      
      if (!m_updated) {
        // Display has been updated, can now begin wait for both sensors to
        // clear to no-detect status.
#if TRACE
        uint32_t now = millis();
#if STREAMING
        Serial << now << " " << m_detA << " " << m_detB << " ACTIVE 3" << endl;
#else
        Serial.print(now);
        Serial.print(" ");
        Serial.print(m_detA);
        Serial.print(" ");
        Serial.print(m_detB);
        Serial.println(" ACTIVE 3");
#endif
#endif
//        m_sense_when = now;
        m_state = eActive;
      }
      
    } else if (m_state == eActive) {

      // Waiting for both sensors to return to no-detect state.

      if (!m_detA && !m_detB) {
        // Sensors cleared, begin timeout period before restarting state machine.
        m_sense_when = millis();
#if TRACE
#if STREAMING
//        Serial << m_sense_when << " " << m_detA << " " << m_detB << " CLEARING 4" << endl;
#else
//        Serial.print(m_sense_when);
//        Serial.print(" ");
//        Serial.print(m_detA);
//        Serial.print(" ");
//        Serial.print(m_detB);
//        Serial.println(" CLEARING 4");
#endif
#endif
        m_state = eClearing;
      }

    } else if (m_state == eClearing) {

      // If either sensor detects during wait-for-clear state, restart timeout period.

      uint32_t now = millis();
      if (m_detA || m_detB) {
        // Uh-oh, sensor(s) detected during timeout period.
        m_state = eActive;
      } else if ((now - m_sense_when) > TIMEOUT_CLEAR) {
        // Timeout period completed, go back to initial state.
#if TRACE
#if STREAMING
        Serial << now << " CLEAR" << endl;
#else
        Serial.print(now);
        Serial.println(" CLEAR");
#endif
#endif
        m_state = eClear;
      }
      
    }
    
    // Turn on built-in LED if one sensor detected and waiting for other one.
    bool led_on = (m_state == eSenseA) || (m_state == eSenseB);
    digitalWrite(LED_BUILTIN, led_on ? HIGH : LOW);
    return true;
    
  }
  
  // We get here if it wasn't time for StateMachine to update.
  return false;
  
}

// Try to initialize both sensors.
int Speedometer::begin() {
  
  if (sensA->begin() | sensB->begin()) {
    return -1;
  }
  return 0;
  
}

// Set scale option.
void Speedometer::setScale(E_Scale s) {
  m_scale = s;
}

// Set metric/imperial speed choice.
void Speedometer::setMetric(bool m) {
  m_metric = m;
}

// Set window of ranges accepted for valid detection.
void Speedometer::setWindow(RangeWindow<uint8_t>* win) {
  m_window = win;
}

// Range is considered "inside" window.
bool Speedometer::inWindow(const uint8_t range) const {
  return m_window->within(range);
}

// Calculate speed from:
//   - elapsed time (msec)
//   - sensor spacing (mm)
//   - scale factor (87, 150, 160, ...)
//   - units (km/hr or mi/hr)
double Speedometer::calcScaleSpeed(const uint32_t dt_msec) const {
  double m_per_sec = (double) m_spacing / (double) dt_msec;
  double scale_speed = m_per_sec * (double) m_scale;
  double km_per_hr = scale_speed * 3.6;
  return km_per_hr * (m_metric ? 1.0 : MI_PER_KM);
}

// Return true exactly once if measured speed has been updated.
bool Speedometer::isUpdated() {
  if (m_updated) {
    m_updated = false;
    return true;
  } else {
    return false;
  }
}

// Get measured speed in currently selected units.
double Speedometer::getSpeed() {
  m_updated = false;
  return m_speed;
}
