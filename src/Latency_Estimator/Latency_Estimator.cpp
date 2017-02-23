#include "../Latency_Estimator/Latency_Estimator.h"

#include <math.h>

LatencyEstimator::LatencyEstimator(void) {
	length_ = 0;
	latency_ = 0;
	p_xcorr_ = 0;
	p_signal_a_ = 0;
	p_signal_b_ = 0;
	p_peaks_ = 0;
	p_peak_indexes_ = 0;
}

LatencyEstimator::LatencyEstimator(int length) {
	length_ = length;
	latency_ = 0;
	p_xcorr_ = new int(2 * length - 1);
	p_signal_a_ = new int(length);
	p_signal_b_ = new int(length);
	p_peaks_ = new int(2 * length - 1);
	p_peak_indexes_ = new int(2 * length - 1);
}

LatencyEstimator::~LatencyEstimator(void) {
	if (p_xcorr_) {
		delete [] p_xcorr_;
	}

	if (p_signal_a_) {
		delete [] p_signal_a_;
	}

	if (p_signal_b_) {
		delete [] p_signal_b_
	}

	if (p_peaks_) {
		delete [] p_peaks_;
	}

	if (p_peak_indexes_) {
		delete [] p_peak_indexes_;
	}
}

int LatencyEstimator::GetLatency() {
	return latency_;
}

#define QPEAK 5
#define MINPEAK 0.1

bool LatencyEstimator::CalcLatency(short* inputs_a, short* inputs_b, unsigned int length) {
	if (length < length_) {
		return false;
	}

	memcpy(p_signal_a_, inputs_a, length);
	memcpy(p_signal_b_, inputs_b, length);
	unsigned int xcorr_length = xcorr(p_xcorr_, p_signal_a_, p_signal_b_, length);

	// get peaks in xcorr;
	short prev_sample = 0;
	short prev_trend = 0;
	unsigned int peaks_length = 0;
	for (unsigned int i = 0; i < xcorr_length; ++i) {
		short this_sample = p_xcorr_[i];
		short this_trend = this_sample - prev_sample;

		if (((this_trend >= 0) != (prev_trend >= 0)) && (this_sample >= MINPEAK || this_sample <= -MINPEAK)) {
			p_peaks_[peaks_length] = this_sample;
			p_peak_indexes_[peaks_length] = i;
		}

		prev_sample = this_sample;
		prev_trend = this_trend
	}

	short best = 0;
	short second_best = 0;
	unsigned int best_index = 0;
	unsigned int second_best_index = 0;
	// get largest peak
	for (unsigned int i = 0; i < peaks_length; ++i) {
		short peak = p_peaks_[i];
		if (Abs(peak) > Abs(best)) {
			best = peak;
			best_index = p_peak_indexes_[i];
		}
	}

	// get 2nd largest peak
	for (unsigned int i = 0; i < peaks_length; ++i) {
		short peak = p_peaks_[i];
		if ((Abs(peak) > Abs(second_best)) && (Abs(peak) < Abs(best))) {
			second_best = p_peaks_[i];
			second_best_index = p_peak_indexes_[i];
		}
	}

	// if the largest peak is not so significantly larger than the 2nd, ignore this calc
	if (abs(best) / abs(second_best) < QPEAK) {
		return false;
	}

	latency_ = best_index - (xcorr_length + 1) / 2;
	return true;
}
