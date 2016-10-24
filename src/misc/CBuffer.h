#ifndef CBUFFER_H
#define CBUFFER_H

class CBuffer {
public:      // Note the 1 space indent!
  CBuffer();  // Regular 2 space indent.
  CBuffer(unsigned max_buffer_length);
  ~CBuffer();

  void PushBack(int val);
  int PopFront();
  int GetLength();

private:
  int* buffer_array_;
  int* head_;
  unsigned int length_;
  int max_buffer_length_;

  bool initialize();
};

#endif
