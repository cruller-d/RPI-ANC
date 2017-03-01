function fifo = initFIFO (length)
	fifo = struct ();
	fifo.length = length;
	fifo.buffer = zeros(1, length);
endfunction