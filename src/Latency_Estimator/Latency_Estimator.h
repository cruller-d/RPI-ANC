#ifndef latency_ESTIMATOR
#define latency_ESTIMATOR

class LatencyEstimator {
public:
  LatencyEstimator();
  LatencyEstimator(int length);
  ~LatencyEstimator();
  bool CalcDelay(short* inputs_a, short* inputs_b, int length);
  int GetDelay();
  int GetMag();
protected:
  int length_;
  int latency_;
  int xcorr_magnitude_;
};

#endif
