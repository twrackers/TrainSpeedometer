#ifndef _RANGE_WINDOW__H_
#define _RANGE_WINDOW__H_

#include "SchmittTrigger.h"

// Return true when a numeric value falls between a pair of constant values.
// Both ends of the range are defined with a hysteresis value to reduce 
// spurious state changes when the input value lingers near either end of the range.
template<typename T>
class RangeWindow {

  private:
    const SchmittTrigger<T>* m_schmitt_lo; // Schmitt object at low end of range
    const SchmittTrigger<T>* m_schmitt_hi; // Schmitt object at high end of range

  public:
    // Constructor
    RangeWindow(const T center, const T hwidth, const T hysteresis) :
    m_schmitt_lo(new SchmittTrigger<T>(center - hwidth, hysteresis)),
    m_schmitt_hi(new SchmittTrigger<T>(center + hwidth, hysteresis)) {}

    // Determine if value is within defined range, with hysteresis at
    // both ends of range.
    bool within(const T val) {
      // Is value greater than low end of range (with hysteresis)?
      bool lo = m_schmitt_lo->f(val);
      // Is value greater than high end of range (with hysteresis)?
      bool hi = m_schmitt_hi->f(val);
      // Return true if between low and high ends, false otherwise.
      return lo && !hi;
    }
};

#endif
