#ifndef FIR_H
#define FIR_H

class FIR {
public:
  FIR();
  FIR(int length);
  ~FIR();
  int Tick(int input_sample);
  int* Tick(int* input_samples);
protected:
  int* weights;
  int* weight_length;
};

#endif
