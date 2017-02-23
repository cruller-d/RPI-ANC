#include "CBuffer.h"

void XCorr (short* result, short* f, short* g, unsigned length) {
  unsigned corr_length = 2 * length - 1;
  for (unsigned i = 0; i < corr_length; ++i) {
    result[i] = 0;
    if (i <= length) {
      for (unsigned j = 0; j < i + 1; ++j) {
        result[i] += f[j] * g[length - i - 1 + j];
      }
    } else {
      for (unsigned j = 0; j < corr_length - i; ++j) {
        result[i] += f[i - length + j] * g[j];
      }
    }
  }
  return corr_length;
}

short Abs(short input) {
  if(input >= 0) {
    return input;
  } else {
    return 0 - input;
  }
}
