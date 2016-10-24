#ifndef DELAY_ESTIMATOR
#define DELAY_ESTIMATOR
#endif

class DelayEstimator {
public:
  DelayEstimator();
  ~DelayEstimator();
  bool CalcDelay(int* inputs_a, int* inputs_b, int length);
  int GetDelay();
  int GetMag();
protected:
  int length_;
  int delay_;
  int xcorr_magnitude_;
};
