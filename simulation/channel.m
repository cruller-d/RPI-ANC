function [c, outputSample] = channel (c, inputSample)
	for i = c.inputFIFOLength : -1 : 2
		c.inputFIFO(i) = c.inputFIFO(i - 1);
	end
	c.inputFIFO(1) = inputSample;
	outputSample = c.outputMemory * (1 - c.IIRCoeff) + c.inputFIFO(c.inputFIFOLength) * c.IIRCoeff;
endfunction