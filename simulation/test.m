clear all
close all

_mju = 0.5;
_delay = 100;
_FIRLength = 64;
_skip = 10000;
_simLength = 44100 * 10;
_estimatorBufferLength = 1000;
_adaptiveFilterBufferLength = 100;
_minQ = 2;

streetRecording = wavread('Road with cars, different... _ Strassenverkehr mit Autos, ... (5684171).wav' );

input = [];
for i = 1 : _simLength
	if (i + _skip) < length(streetRecording)
		input(i) = streetRecording(i + _skip, 1);
	else
		input(i) = 0;
	endif
end

chn = initChannel(_delay, 0.5);
estimator = initLatencyEstimator(_estimatorBufferLength, _minQ);
filter = initFIRFilter(_FIRLength);
tuner = initAdaptiveFilterTuner(_adaptiveFilterBufferLength, _FIRLength, 1);

channelOutputSamples = [];#zeros(1, length(input));
filterOutputSamples = [];#zeros(1, length(input));
errSamples = [];#zeros(1, length(input));
delayedInputSamples = [];#zeros(1, length(input));

status = 0;
for i = 1 : length(input)
	inputSample = input(i);
	[chn, channelOutputSample] = channel(chn, inputSample);
	channelOutputSamples(i) = channelOutputSample;

	if status == 0
		[estimator, delay, q] = latencyEstimator(estimator, inputSample, channelOutputSample);
		if (q > _minQ)
			status = 1;
			f = initFIFO(delay + 1 - _FIRLength / 2);
			i
			delay
			q
		endif
	else
		[f, delayedInputSample] = FIFO(f, inputSample);
		delayedInputSamples(i) = delayedInputSample;
		[filter, filterOutputSample] = FIRFilter(filter, delayedInputSample);
		filterOutputSamples(i) = filterOutputSample;

		errSample = channelOutputSample - filterOutputSample;
		errSamples(i) = errSample;
		[tuner, dw] = adaptiveFilterTuner(tuner, delayedInputSample, errSample, 0);
		filter.weight += dw * _mju;
	endif

	if mod(i, 1000) == 0
		subplot(2, 1, 1);
		hold off;
		plot(channelOutputSamples, 'b');
		hold on;
		plot(filterOutputSamples, 'r');
		plot(errSamples, 'g');
		subplot(2, 1, 2);
		plot(filter.weight);
	endif
end

attenuation = sum(channelOutputSamples(1 : 40000).^2) / sum(filterOutputSamples(1 : 40000).^2);
