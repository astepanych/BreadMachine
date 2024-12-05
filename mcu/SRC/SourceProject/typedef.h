#ifndef TYPEDEF_H
#define TYPEDEF_H

#include "LittleBigEndian.h"

typedef LittleEndian<uint16_t> u16le;
typedef LittleEndian<uint32_t> u32le;
typedef LittleEndian<uint64_t> u64le;

typedef BigEndian<uint16_t> u16be;
typedef BigEndian<uint32_t> u32be;
typedef BigEndian<uint64_t> u64be;

#endif // TYPEDEF_H
