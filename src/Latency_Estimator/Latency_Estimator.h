#ifndef latency_ESTIMATOR
#define latency_ESTIMATOR

#include "../misc/MyCalc.h"

class LatencyEstimator {
public:
  LatencyEstimator();
  LatencyEstimator(int length);
  ~LatencyEstimator();
  bool CalcLatency(short* inputs_a, short* inputs_b, unsigned int length);
  int GetLatency();

protected:
  int length_;
  int latency_;
  long xcorr_magnitude_;
  short* p_xcorr_;
  short* p_signal_a_;
  short* p_singal_b_;
  short* p_peaks_;
  unsigned int* p_peak_indexes_;
};

#endif
