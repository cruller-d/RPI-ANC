function filter = initFIRFilter (length)
	filter = struct ();
	filter.length = length;
	filter.inputFIFO = zeros(1, length);
	filter.weight = zeros(1, length);
endfunction