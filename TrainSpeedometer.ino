#include <StateMachine.h>

#include <SparkFun_Alphanumeric_Display.h>

#include "Speedometer.h"

// If TRACE or STREAMING are #define'd, they're in Speedometer.h

#if TRACE
#if STREAMING
#include <Streaming.h>
#endif
#endif

// Define GPIO pins for mode settings.
// All will be set to INPUT_PULLUP, so open connections read as HIGH.
// GPIO pin to select metric (km/hr) or imperial (mph)
#define METRIC_PIN 8
// GPIO pin to select Japan N scale (1:150) or US N scale (1:160)
#define SCALE_PIN 9
// GPIO pin to select one of two detect ranges
#define RANGE_PIN 10

// 14-segment 8-character display
HT16K33 display;

// Define Speedometer object with default scale.
Speedometer::E_Scale scale = Speedometer::eUS;
Speedometer meter(scale);

// All units in mm
// Center of range 1
#define CENTER_1 33
// Center of range 2
#define CENTER_2 66
// Half-width of both ranges
#define HWIDTH 12
// Hysteresis in detect thresholds
#define HYSTERESIS 7

// Define two RangeWindow objecs.
RangeWindow<uint8_t> range_win1(CENTER_1, HWIDTH, HYSTERESIS);  // near track
RangeWindow<uint8_t> range_win2(CENTER_2, HWIDTH, HYSTERESIS);  // far track

void setup() {

#if TRACE
  Serial.begin(115200);
#endif

  // Initialize I2C interface.
  Wire.begin();

  // Set GPIO modes.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(METRIC_PIN, INPUT_PULLUP);
  pinMode(SCALE_PIN, INPUT_PULLUP);
  pinMode(RANGE_PIN, INPUT_PULLUP);

  // Try to initialize pair of 4-character displays as single HT16K33 object.
  if (!display.begin(0x70, 0x71)) {
    // Display failed to init.
#if TRACE
#if STREAMING
    Serial << "display.begin() failed" << endl;
#else
    Serial.println("display.begin() failed");
#endif
#endif
  } else {
    // Display initialied, set to minimum brightness.
    display.setBrightness(0);   // range [0,15]
    display.print("DISP OK");
    delay(500);
  }

  // Try to initialize sensors for Speedometer object.
  if (!meter.begin()) {
    // Sensor(s) failed to init.  Display error message and halt.
#if TRACE
#if STREAMING
    Serial << "meter.begin() failed" << endl;
#else
    Serial.println("meter.begin() failed");
#endif
#endif
    display.print("SPD ERR");
    while (true) ;    // do not proceed
  } else {
    // Sensors initialized.
    display.print("SPD  OK");
    delay(500);
  }
  
  // Clear display for now.
  display.clear();

}

void loop() {

  // If Speedometer state updated...
  if (meter.update()) {
    
    // Set modes based on state of GPIO pins.
    // Non-connected pins are pulled up to HIGH.
    meter.setMetric(digitalRead(METRIC_PIN) == HIGH);
    bool jp_scale = digitalRead(SCALE_PIN) == HIGH;
    meter.setScale(jp_scale ? Speedometer::eJP : Speedometer::eUS);
    meter.setWindow(digitalRead(RANGE_PIN) == HIGH ? &range_win2 : &range_win1);
    
    // If measured speed has been updated...
    if (meter.isUpdated()) {
      // Write new speed to display.
      // For example, 123 km/hr is displayed as " 123KPH ", with small gap
      // between two 4-character display units.
      char str[9];
      sprintf(str, "%4d%s",
              (int) round(meter.getSpeed()), (jp_scale ? "KPH" : "MPH"));
      display.print(str);
    }
    
  }
         
}
