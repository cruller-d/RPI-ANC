#include "../Latency_Estimator/Latency_Estimator.h"

#include <math.h>

bool GetLatency(int length, short* signal_a, short* signal_b, int& latency, int& max_cross_xcorr);
int SumProuct(short* signal_samples, short* weights, int l);

LatencyEstimator::LatencyEstimator(void) {
	length_ = 0;
	latency_ = 0;
	xcorr_magnitude_ = 0;
}

LatencyEstimator::LatencyEstimator(int length) {
	length_ = length;
	latency_ = 0;
	xcorr_magnitude_ = 0;
}

LatencyEstimator::~LatencyEstimator(void) {

}

int LatencyEstimator::GetDelay() {
	return latency_;
}

int LatencyEstimator::GetMag() {
	return xcorr_magnitude_;
}

bool GetLatency(int length, short* signal_a, short* signal_b, int& latency, int& max_cross_xcorr);

bool LatencyEstimator::CalcDelay(short* inputs_a, short* inputs_b, int length) {
	int latency, mag;
	bool succ = GetLatency(length, inputs_a, inputs_b, latency, mag);
	return succ;
}

// get latency and max cross correlation value
bool GetLatency(int length, short * signal_a, short * signal_b, int & latency, int & max_cross_xcorr) {
  int max_tao = length - 1;
  int max_xcorr = 0;
  int abs_max_xcorr = 0;
  for (int tao = 1 - length; tao < max_tao; ++tao) {
    int xcorr = 0;
    int max_multiply = length - tao;
    short * p_signal_forward, * p_signal_backward;
    if (tao <= 0) {
    	p_signal_forward = signal_a;
    	p_signal_backward = signal_b + length - 1;
    } else {
    	p_signal_forward = signal_b;
    	p_signal_backward = signal_a + length - 1;
    }
    for (int i = 0; i < max_multiply; ++i) {
      xcorr += (*(p_signal_forward++)) * (*(p_signal_backward--));
    }
    if (fabs(xcorr) > abs_max_xcorr) {
      max_xcorr = xcorr;
      abs_max_xcorr = max_xcorr;
      latency = tao;
    }
  }
  max_cross_xcorr = max_xcorr;
  return true;
};
