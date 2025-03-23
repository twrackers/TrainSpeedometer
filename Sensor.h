#ifndef _SENSOR__H_
#define _SENSOR__H_

#include <Wire.h>

#include "ST_VL6180X.h"

#include "Filter.h"

// Distance to be returned if sensor returns no valid range measurement.
#define NO_READING 0xFFFFFFFFL

class Sensor {

  private:
    const VL6180X* m_sensor;            // (pointer to) sensor object
    const Filter<uint32_t>* m_filter;   // (pointer to) low-pass filter
    bool* m_ready;                      // (pointer to) is-ready flag
    const byte m_addr;                  // I2C address
    const byte m_gpio0;                 // GPIO pin for enable to sensor
    const byte m_gpio1;                 // GPIO pin for interrupt from sensor
    uint32_t m_dist;                    // measured distance in mm

  public:
    Sensor(
      const byte addr, const byte ena_pin, const byte intr_pin, const bool* ready_flag
    );
    void setupInterruptHandler(
      const uint8_t irq_pin, void (*irq_func)(), const int value
    );
    bool begin();
    int trigger();
    bool is_ready() const;
    uint32_t get_distance();
  
};

#endif
