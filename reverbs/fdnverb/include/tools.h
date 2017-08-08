#ifndef FDN_tools_h
#define FDN_tools_h

#include "Matrix.h"

namespace SSRverb {

extern Matrix hadamard(unsigned order, float gain = 1);

extern int prime_power(int x);

extern bool is_pow2(unsigned x);

} // namespace

#endif
