clear all
close all

_mju = 0.5;
_primaryChannelDelay = 200;
_secondaryChannelDelay = 100;
_FIRLength = 64;
_skip = 10000;
_simLength = 44100 * 1;
_estimatorBufferLength = 1000;
_adaptiveFilterBufferLength = 100;
_minQ = 2;
_noiseLevel = 0.01;
_testLevel = 0.1;

streetRecording = wavread('Road with cars, different... _ Strassenverkehr mit Autos, ... (5684171).wav' );

input = [];
for i = 1 : _simLength
	if (i + _skip) < length(streetRecording)
		input(i) = streetRecording(i + _skip, 1);
	else
		input(i) = 0;
	endif
end

primaryChannel = initChannel(_primaryChannelDelay, 1);
secondaryChannel = initChannel(_secondaryChannelDelay, 1);
estimator = initLatencyEstimator(_estimatorBufferLength, _minQ);
mailFilter = initFIRFilter(_FIRLength);
secondaryFilter = initFIRFilter(_FIRLength);
tuner = initAdaptiveFilterTuner(_adaptiveFilterBufferLength, _FIRLength, 1);

primaryChannelOutputSamples = [];
secondaryChannelOutputSamples = [];
speakerOutputSamples = [];
micInputSamples = [];

status = 0;
cnt = 0;
for i = 1 : length(input)
	inputSample = input(i);
	[primaryChannel, primaryChannelOutputSample] = channel(primaryChannel, inputSample);
	primaryChannelOutputSamples(i) = primaryChannelOutputSample;
	micNoiseSample = (rand() - 0.5) * _noiseLevel;
	
	# STATE MACHINE
	if status == 0 # measure latency for secondary channel
		# stim
		testSample = (rand() - 0.5) * _testLevel;
		speakerOutputSamples(i) = testSample;

		# test sample propagates through secondary channel
		[secondaryChannel, secondaryChannelOutputSample] = channel(secondaryChannel, speakerOutputSamples(i));
		secondaryChannelOutputSamples(i) = secondaryChannelOutputSample;

		micInputSamples(i) = primaryChannelOutputSamples(i) + secondaryChannelOutputSamples(i) + micNoiseSample;
		# measure latency
		[estimator, delay, q] = latencyEstimator(estimator, testSample, micInputSamples(i));
		
		cnt += 1;
		if (q > _minQ)
			cnt
			status = 1;
			cnt = 0;
			sqSignal = 0;
			sqErr = 0.001;
			secondaryDelayLine = initFIFO(delay + 1 - _FIRLength / 2);
			delay
			q
		endif
	elseif status == 1 # train filter to simulate secondary channel
		testSample = (rand() - 0.5) * _testLevel;
		speakerOutputSamples(i) = testSample;

		# stim goes through delay line and then adaptive filter
		[secondaryDelayLine, delayedTestSample] = FIFO(secondaryDelayLine, testSample);
		[secondaryFilter, pseudoSecondarySample] = FIRFilter(secondaryFilter, delayedTestSample);

		# test sample propagates through secondary channel
		[secondaryChannel, secondaryChannelOutputSample] = channel(secondaryChannel, speakerOutputSamples(i));
		secondaryChannelOutputSamples(i) = secondaryChannelOutputSample;
		micInputSamples(i) = primaryChannelOutputSamples(i) + secondaryChannelOutputSamples(i) + micNoiseSample;
		
		errSample = micInputSamples(i) - pseudoSecondarySample;

		[tuner, dw] = adaptiveFilterTuner(tuner, delayedTestSample, errSample, 0);
		secondaryFilter.weight += dw * _mju;

		sqSignal = sqSignal * 0.9 + testSample ^2 * 0.1;
		sqErr = sqErr * 0.9 + errSample ^2 * 0.1;

		cnt += 1;
		if cnt > 1000 && sqSignal / sqErr > 1000
			status = 2;
			cnt = 0;
			break;
		endif
	elseif status == 2 # measure latency for primary channel
		# input signal goes through delay line and filtered
		[secondaryDelayLine, delayedInputSample] = FIFO(secondaryDelayLine, inputSample);
		[secondaryFilter, pseudoSecondarySample] = FIRFilter(secondaryFilter, delayedInputSample);
		[estimator, remainderDelay, q] = latencyEstimator(estimator, pseudoSecondarySample, primaryChannelOutputSample + micNoiseSample);
		
		speakerOutputSamples(i) = 0;
		[secondaryChannel, secondaryChannelOutputSample] = channel(secondaryChannel, speakerOutputSamples(i));
		secondaryChannelOutputSamples(i) = secondaryChannelOutputSample;
		micInputSamples(i) = primaryChannelOutputSamples(i) + secondaryChannelOutputSamples(i) + micNoiseSample;
		
		cnt += 1;
		if (q > _minQ)
			cnt
			remainderDelay
			q
			status = 3;
			cnt = 0;
			sqSignal = 0;
			sqErr = 0.001;
			mainDelayLine = initFIFO(remainderDelay + 1 - _FIRLength / 2);
		endif
	elseif status == 3 # train filter according to outside signals passing through primary channel
		[mainDelayLine, delayedInputSample] = FIFO(mainDelayLine, inputSample + micNoiseSample);
		[mailFilter, filterOutputSample] = FIRFilter(mainFilter, delayedInputSample);

		errSample = primaryChannelOutputSample - filterOutputSample;
		[tuner, dw] = adaptiveFilterTuner(tuner, delayedInputSample, errSample, 0);
		mailFilter.weight += dw * _mju;
		sqSignal = sqSignal * 0.9 + testSample ^2 * 0.1;
		sqErr = sqErr * 0.9 + errSample ^2 * 0.1;

		speakerOutputSamples(i) = 0;
		[secondaryChannel, secondaryChannelOutputSample] = channel(secondaryChannel, speakerOutputSamples(i));
		secondaryChannelOutputSamples(i) = secondaryChannelOutputSample;
		cnt += 1;
		if cnt > 1000 && sqSignal / sqErr > 1000
			status = 4;
			cnt = 0;
		endif
	else
		[mainDelayLine, delayedInputSample] = FIFO(mainDelayLine, inputSample + micNoiseSample);
		[mailFilter, filterOutputSample] = FIRFilter(mainFilter, delayedInputSample);
		speakerOutputSamples(i) = filterOutputSample * -1;
		[secondaryChannel, secondaryChannelOutputSample] = channel(secondaryChannel, speakerOutputSamples(i));
		secondaryChannelOutputSamples(i) = secondaryChannelOutputSample;
	endif
	mixSamples(i) = primaryChannelOutputSamples(i) + secondaryChannelOutputSamples(i);

	if mod(i, 100) == 0
		subplot(2, 3, 1);
		plot(primaryChannelOutputSamples);
		subplot(2, 3, 2);
		plot(secondaryChannelOutputSamples);
		subplot(2, 3, 3);
		plot(speakerOutputSamples);
		subplot(2, 3, 4);
		plot(mixSamples);
		subplot(2, 3, 5);
		plot(mailFilter.weight);
		subplot(2, 3, 6);
		plot(secondaryFilter.weight);
	endif
end

		subplot(2, 3, 1);
		plot(primaryChannelOutputSamples);
		subplot(2, 3, 2);
		plot(secondaryChannelOutputSamples);
		subplot(2, 3, 3);
		plot(speakerOutputSamples);
		subplot(2, 3, 4);
		plot(mixSamples);
		subplot(2, 3, 5);
		plot(mailFilter.weight);
		subplot(2, 3, 6);
		plot(secondaryFilter.weight);
