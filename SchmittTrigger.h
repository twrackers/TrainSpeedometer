#ifndef _SCHMITT_TRIGGER__H_
#define _SCHMITT_TRIGGER__H_

// Simulate a Schmitt trigger to determine whether a numeric value
// is greater or less than a reference value, with a hysteresis window
// to prevent spurious state changes when close to the reference.
//
// The object has a "hysteresis" value m_hy which defines a range of [-m_hy, +m_hy].
// The behavior of the object when method f(val) is called is as follows.
//   val > m_hy:         state becomes or remains true
//   -m_hy < val < m_hy: state does not change
//   val < -m_hy:        state becomes or remains false
template<typename T>
class SchmittTrigger {
  
  private:
    const long m_ref; // reference value
    const long m_hy;  // hysteresis value
    bool m_state;     // current output state
    
  public:

    // Constructor
    // Arguments:
    //   reference: define value around which hysteresis is centered
    //   hysteresis: define half-width of hysteresis range
    //   initial: set initial output state [default low (false)]
    SchmittTrigger(const T reference, const T hysteresis, bool initial = false) :
      m_ref((long) reference), 
      m_hy((long) hysteresis), 
      m_state(initial) 
      { 
      }

    // Get state of "Schmitt trigger".
    // Arguments:
    //   val: input value to "Schmitt trigger"
    // Returns:
    //   false if val is less than (m_ref - m_hy), 
    //     or has not been greater than (m_ref + m_hy)
    //     since last state change
    //   true if val is greater than (m_ref + m_hy),
    //     or has not been less than (m_ref - m_hy)
    //     since last state change
    bool f(const T val) {
      // Check value against hysteresis.  If value is in range
      // [m_ref - m_hy, m_ref + m_hy], state will not change.
      if ((long) val > (m_ref + m_hy)) {
        // State becomes true if value above range.
        m_state = true;
      } else if ((long) val < (m_ref - m_hy)) {
        // State becomes false if value below range.
        m_state = false;
      }
      // Return (possibly updated) state.
      return m_state;
    }
    
};

#endif
