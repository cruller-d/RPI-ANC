#include "CBuffer.h"

CBuffer::CBuffer(void) {
  max_buffer_length_ = 0;
  initialize();
  return;
}

CBuffer::CBuffer(unsigned max_length) {
  max_buffer_length_ = max_length;
  initialize();
  return;
}

CBuffer::~CBuffer(void) {
	if (p_buffer_array_) {
		delete [] p_buffer_array_;
	}
}

bool CBuffer::initialize() {
  if (max_buffer_length_) {
  	p_buffer_array_ = new int[max_buffer_length_];
  } else {
  	p_buffer_array_ = 0;
  }
  p_head_ = 0;
  length_ = 0;
  return false;
}

int CBuffer::PopFront() {
	int ret_val = p_buffer_array_[p_head_];
	p_head_ ++;
	p_head_ = p_head_ % max_buffer_length_;
	return ret_val;
}

void CBuffer::PushBack(int val) {
	p_buffer_array_[(p_head_ + length_) % max_buffer_length_] = val;

	if (length_ < max_buffer_length_) {
		length_ ++;
	}
}
