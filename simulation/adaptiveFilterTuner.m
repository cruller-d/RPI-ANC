function [tuner, dWeight] = adaptiveFilterTuner (tuner, inputSample, errSample, delay)
	for i = tuner.inputFIFOLength : -1 : 2
		tuner.inputFIFO(i) = tuner.inputFIFO(i - 1); 
	end
	tuner.inputFIFO(1) = inputSample;

	dWeight = [];
	for i = 1 : tuner.FIRLength
		delayedInputSample = 0;
		dn1 = 0;
		dp1 = 0;
		if i + delay <= tuner.inputFIFOLength
			delayedInputSample = tuner.inputFIFO(i + delay);
		endif
		if i + delay + 1 <= tuner.inputFIFOLength
			dp1 = tuner.inputFIFO(i + delay + 1);
		endif
		if i + delay - 1 >= 1
			dn1 = tuner.inputFIFO(i + delay - 1);
		endif
		dWeight(i) = errSample * (delayedInputSample + dn1 + dp1) / 3;
	end
endfunction