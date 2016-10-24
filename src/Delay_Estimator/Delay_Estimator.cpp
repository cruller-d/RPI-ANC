#include "Delay_Estimator.h"
#include <math.h>

// get latency and max cross correlation value
bool GetLatency(int length, int* signal_a, int* signal_b, int& latency, int& max_cross_xcorr) {
  int max_tao = 2 * length - 1;
  int max_xcorr = 0;
  int abs_max_xcorr = 0;
  for (int tao = 0; tao < max_tao; ++tao) {
    int xcorr = 0;
    int max_multiply = tao + 1;
    int* p_signal_a = signal_a;
    int* p_signal_b = signal_b + length - 1;
    for (int i = 0; i < max_multiply; ++i) {
      xcorr += (*(p_signal_a++)) * (*(p_signal_b--));
    }
    if (fabs(xcorr) > abs_max_xcorr) {
      max_xcorr = xcorr;
      abs_max_xcorr = max_xcorr;
      latency = tao - length;
    }
  }
  max_cross_xcorr = max_xcorr;
  return true;
};

// Calculate inner product, essential element for FIR filter
int SumProuct(int* signal_samples, int* weights, int l) {
  int sum = 0;
  int* p_signal = signal_samples;
  int* p_weight = weights;
  for (int i = 0; i < l; ++i) {
    sum += (*(p_signal++)) * (*(p_weight++));
  }
  return sum;
}

