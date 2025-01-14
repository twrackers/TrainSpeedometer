#ifndef _SPEEDOMETER__H_
#define _SPEEDOMETER__H_

#include <StateMachine.h>

#include "Sensor.h"

#include "RangeWindow.h"

#define TRACE 0
#define STREAMING 0

class Speedometer : public StateMachine {

  public:
    // Define model scales, such as 160 for US N scale of 1:160
    enum E_Scale {
      eUK = 148, eJP = 150, eUS = 160
    };
  
  private:
    // Separation of sensors (mm)
    const int m_spacing;
    // Time of most recent first-detect (msec)
    uint32_t m_sense_when;
    // Measured speed (scale mi/hr or km/hr)
    double m_speed;
    // Model scale (1:148, 1:150, or 1:160)
    E_Scale m_scale;
    // (Pointer to) Window of range
    RangeWindow<uint8_t>* m_window;
    // Metric (km/hr) or imperial (mi/hr)
    bool m_metric;
    // State of finite state machine
    enum E_State {
      eClear,           // waiting for sense on sensor A or B
      eSenseA,          // sensed on A, waiting for B
      eSenseB,          // sensed on B, waiting for A
      eUpdated,         // speed measured, waiting for display to update
      eActive,          // waiting for both sensors to clear
      eClearing         // waiting for timeout after sensors clear
    } m_state;
    bool m_detA;        // sensor A detected
    bool m_detB;        // sensor B detected
    bool m_triggered;   // sensor emitters triggered
    bool m_updated;     // speed measure updated

  public:
    Speedometer(E_Scale s = eJP);
    virtual bool update();
    int begin();
    void setScale(E_Scale s);
    void setMetric(bool m);
    void setWindow(RangeWindow<uint8_t>* win);
    bool inWindow(const uint8_t range) const;
    bool within(const uint8_t range) const;
    double calcScaleSpeed(const uint32_t dt_msec) const;
    bool isUpdated();
    double getSpeed();
  
};

#endif
