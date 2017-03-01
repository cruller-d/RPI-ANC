function [estimator, delay, q] = latencyEstimator (estimator, sample1, sample2)

	for i = estimator.bufferLength : -1 : 2
		estimator.data1(i) = estimator.data1(i - 1);
		estimator.data2(i) = estimator.data2(i - 1);
	end
	estimator.data1(1) = sample1;
	estimator.data2(1) = sample2;

	delay = 0;
	q = 0;
	if estimator.currentLength < estimator.bufferLength
		estimator.currentLength += 1;
	else
		l = estimator.bufferLength;
		leftData = estimator.data1(1:l);
		rightData = estimator.data2(1:l);
		corrResult = xcorr(leftData, rightData);
		prev_sample = 0;
		prev_trend = 0;
		peaks = [];
		peakIndexes = [];
		for i = 1 : length(corrResult)
			this_sample = corrResult(i);
			this_trend = this_sample - prev_sample;

			if (this_trend >=0) != (prev_trend >=0) && (this_sample >= 0.01 || this_sample <= -0.01)
				peaks = [peaks, this_sample];
				peakIndexes = [peakIndexes, i];
			endif

			prev_sample = this_sample;
			prev_trend = this_trend;
		end

		best = 0;
		secondBest = 0;
		bestIndex = 0;
		secondBestIndex = 0;
		for i = 1 : length(peaks)
			if abs(peaks(i)) > abs(best)
				best = peaks(i);
				bestIndex = peakIndexes(i);
			endif
		end

		for i = 1: length(peaks)
			if (abs(peaks(i)) > abs(secondBest)) && (abs(peaks(i)) < abs(best))
				secondBest = peaks(i);
				secondBestIndex = peakIndexes(i);
			endif
		end

		if (best != 0) && (secondBest != 0)
			q = abs(best / secondBest);
			if q > estimator.q
				delay = bestIndex - 1 - (length(corrResult) + 1) / 2;
			endif
		endif
	endif
endfunction