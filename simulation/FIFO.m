function [f, outputSample] = FIFO (f, inputSample)
	for i = f.length : -1 : 2
		f.buffer(i) = f.buffer(i - 1);
	end
	f.buffer(1) = inputSample;
	outputSample = f.buffer(f.length);
endfunction