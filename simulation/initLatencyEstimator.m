function obj = initLatencyEstimator (length, q)
	obj = struct();
	obj.q = q;
	obj.bufferLength = length;
	obj.data1 = zeros(1, length);
	obj.data2 = zeros(1, length);
	obj.currentLength = 0;
endfunction