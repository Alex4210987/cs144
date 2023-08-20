#include "wrapping_integers.hh"
#include <iostream>

using namespace std;

Wrap32 Wrap32::wrap(uint64_t n, Wrap32 zero_point)
{
  uint32_t relative_seqno = (uint32_t)(n + zero_point.raw_value_) % (1ULL << 32);
  return Wrap32(relative_seqno);
}

uint64_t Wrap32::unwrap(Wrap32 zero_point, uint64_t checkpoint) const
{
  uint32_t offset=this->raw_value_ - zero_point.raw_value_;
  if(checkpoint<offset)
    return offset;
  return offset + (1ULL << 32)*(uint32_t)((checkpoint-offset+(1ULL<<31))/(1ULL << 32));
}