#include "CBuffer.h"

CBuffer::CBuffer(void) {
  length_ = 0;
  head_ = 0;
  max_buffer_length_ = 0;
  buffer_array_ = 0;
  return;
}

CBuffer::CBuffer(unsigned max_length) {
  length_ = 0;
  head_ = 0;
  max_buffer_length_ = max_length;
  buffer_array_ = 0;
  return;
}

bool CBuffer::initialize() {
  // to be done
  return false;
}
