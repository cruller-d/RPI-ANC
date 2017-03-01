function filter = initFIRFilter (length)
	filter = struct ();
	filter.length = length;
	filter.inputFIFO = zeros(1, length);
	filter.weight = rand(1, length);
	#filter.weight(length / 2) = 0.5;
endfunction