function [output, err, w] = adaptiveFilter (input, reference, FIRLength, mju)
	w = rand(1, FIRLength) * 0.1;
	if length(input) > length(reference)
		l = length(reference);
	else
		l = length(input);
	endif

	output = zeros(1, l);
	err = [];

	filteredOutput = zeros(1, l);
	temp = 0;

	for i = 1 : l
		if i > FIRLength
			start = i - FIRLength;
		else
			start = 1;
		endif
		signalInFIR = [];
		for j = 1 : FIRLength
			if start + j > i
				signalInFIR(j) = 0;
			else
				signalInFIR(j) = input(start + j);
			endif
		end
		output(i) = w * signalInFIR';


		if i > 1
			temp = output(i - 1);
		else
			temp = 0;
		endif

		if i > 1
			filteredOutput(i) = 0.9 * filteredOutput(i - 1) + 0.1 * temp;
		endif
		err(i) = reference(i) - filteredOutput(i);


		
		err(i) = reference(i) - output(i);
		dw = [];
		for j = 1 : FIRLength
			dw(j) = 2 * mju * err(i) * signalInFIR(j);
		end
		w = w + dw;

		if mod(i, 500) == 0
			plot(reference, 'b');
			hold on;
			plot(output, 'r');
			plot(err, 'g');
			hold off;
		endif
	end
endfunction