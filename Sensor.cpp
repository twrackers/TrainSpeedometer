#include "Sensor.h"

#define TRACE 0

// Constructor
Sensor::Sensor(
    const byte addr, 
    const byte ena_pin, 
    const byte intr_pin,
    const bool* ready_flag
) : 
    m_sensor(new VL6180X(&Wire, ena_pin)),
    m_filter(new Filter<uint32_t>(10)),
    m_ready(ready_flag),
    m_addr(addr),
    m_gpio0(ena_pin),
    m_gpio1(intr_pin),
    m_dist(NO_READING)
{
}

// Set up interrupt handler for sensor's GPIO1 pin.
void Sensor::setupInterruptHandler(
    const uint8_t irq_pin, 
    void (*irq_func)(), 
    const int value
) {
    attachInterrupt(digitalPinToInterrupt(irq_pin), irq_func, value);
}

// Initialize sensor and set options.
// Returns true only if all steps succeed, false otherwise.
bool Sensor::begin() {
    
    m_sensor->begin();
    m_sensor->VL6180x_On();
    
    if (m_sensor->InitSensor(m_addr) != 0) {
#if TRACE
        Serial.println("ERROR: InitSensor");
#endif
        return false;
    }
    if (m_sensor->Present() != 1) {
#if TRACE
        Serial.println("ERROR: Present");
#endif
        return false;
    }
    if (m_sensor->Prepare() != 0) {
#if TRACE
        Serial.println("ERROR: Prepare");
#endif
        return false;
    }
    if (m_sensor->SetupGPIO1(GPIOx_SELECT_GPIO_INTERRUPT_OUTPUT, GPIOx_SELECT_GPIO_INTERRUPT_OUTPUT) != 0) {
#if TRACE
        Serial.println("ERROR: SetupGPIO1");
#endif
        return false;
    }
    if (m_sensor->RangeConfigInterrupt(CONFIG_GPIO_INTERRUPT_NEW_SAMPLE_READY) != 0) {
#if TRACE
        Serial.println("ERROR: RangeConfigInterrupt");
#endif
        return false;
    }
    if (m_sensor->RangeSetMaxConvergenceTime(8) != 0) {
#if TRACE
        Serial.println("ERROR: RangeSetMaxConvergenceTime");
#endif
        return false;
    }
    if (m_sensor->FilterSetState(0) != 0) {
#if TRACE
        Serial.println("ERROR: FilterSetState");
#endif
        return false;
    }
    if (m_sensor->DMaxSetState(0) != 0) {
#if TRACE
        Serial.println("ERROR: DMaxSetState");
#endif
        return false;
    }
    
    return true;
    
}

// Trigger to pulse laser and begin range measurement.
int Sensor::trigger() {
    if (!(*m_ready)) {
        return -1;
    }
    *m_ready = false;
    m_dist = NO_READING;
    return m_sensor->RangeStartSingleShot();
}

// Return true if sensor is ready to do another single-shot measurement.
bool Sensor::is_ready() const {
    return *m_ready;
}

// Get range measurement if one is available.
// If a measurement is available, it will be passed through low-pass filter
// before it is returned.
// If error occurred or no measurement available, NO_READING is returned.
uint32_t Sensor::get_distance() {
    VL6180x_RangeData_t data;
    int rc = m_sensor->RangeGetMeasurementIfReady(&data);
    if (rc == 0) {
        m_dist = m_filter->filter(data.range_mm);
        return m_dist;
    } else {
        return (uint32_t) NO_READING;
    }
}
