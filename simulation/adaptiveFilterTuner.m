function [tuner, dWeight] = adaptiveFilterTuner (tuner, inputSample, errSample, delay)
	for i = tuner.inputFIFOLength : -1 : 2
		tuner.inputFIFO(i) = tuner.inputFIFO(i - 1); 
	end
	tuner.inputFIFO(1) = inputSample;

	dWeight = [];
	for i = 1 : tuner.FIRLength
		delayedInputSample = 0;
		if i + delay <= tuner.inputFIFOLength
			delayedInputSample = tuner.inputFIFO(i + delay);
		endif
		dWeight(i) = errSample * delayedInputSample;
	end
endfunction