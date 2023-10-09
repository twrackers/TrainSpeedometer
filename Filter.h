#ifndef _FILTER__H_
#define _FILTER__H_

// First-order low-pass filter to smooth data values from range sensors

template<typename T>
class Filter {

  private:
    const size_t m_nsamps;  // number of samples to average over
    T* m_buffer;            // (pointer to) buffer of samples

  public:
    // Constructor
    Filter(const size_t nsamps) : m_nsamps(nsamps), m_buffer(new T[nsamps]) {
      // Allocate and clear samples buffer.
      T* ptr = m_buffer;
      size_t n = m_nsamps;
      while (n--) {
        *ptr++ = (T) 0;
      }
    }

    // Add new sample to filter and calculate new filtered output.
    T filter(const T samp) {
      // Going to move current samples down one position,
      // discarding oldest sample and appending newest sample.
      T* sptr = m_buffer + 1;   // source pointer
      T* dptr = m_buffer;       // destination pointer
      size_t n = m_nsamps;      // number of samples
      T sum = (T) 0;            // running sum
      while (n--) {
        // Either move sample down, or append new one.
        *dptr = (n == 0) ? samp : *sptr++;
        // Add sample to running sum.
        sum += *dptr++;
      }
      // Return average of stored samples.
      return (sum / m_nsamps);
    }
  
};

#endif
