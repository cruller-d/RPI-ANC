function tuner = initAdaptiveFilterTuner (inputFIFOLength, FIRLength, sampleRatio)
	tuner = struct ();
	tuner.inputFIFOLength = inputFIFOLength;
	tuner.FIRLength = FIRLength;
	tuner.sampleRatio = sampleRatio;
	tuner.inputFIFO = zeros(1, inputFIFOLength);
endfunction