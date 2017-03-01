function chn = initChannel (delay, IIRCoeff)
	chn = struct ();
	chn.delay = delay;
	chn.IIRCoeff = IIRCoeff;
	chn.inputFIFOLength = delay + 1;
	chn.inputFIFO = zeros(1, delay + 1);
	chn.outputMemory = 0;
endfunction