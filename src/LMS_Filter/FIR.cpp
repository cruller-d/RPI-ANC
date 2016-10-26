#include "../LMS_Filter/FIR.h"


// Calculate inner product, essential element for FIR filter
int SumProuct(short* signal_samples, short* weights, int l) {
  int sum = 0;
  short* p_signal = signal_samples;
  short* p_weight = weights;
  for (int i = 0; i < l; ++i) {
    sum += (*(p_signal++)) * (*(p_weight++));
  }
  return sum;
}
