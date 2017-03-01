function [filter, outputSample] = FIRFilter (filter, inputSample)
	for i = filter.length : -1 : 2
		filter.inputFIFO(i) = filter.inputFIFO(i - 1);
	end
	filter.inputFIFO(1) = inputSample;
	outputSample = sum(filter.inputFIFO .* filter.weight);
endfunction