#pragma once
#ifndef _COMMON_H
#define _COMMON_H

#define WINDOW 64
#define STAY 110
#define OLD 0.3
#define NEW 0.7
#define RECORDE_STEP 10000


#include <string>
using Item = std::string;

struct Entry {
	Item item;
	uint64_t size;
};
struct metadata
{
	uint64_t size;
	uint64_t accnum;
	uint64_t weidis;
	uint64_t lastacc;
	uint64_t lastdis;
	uint64_t dis;
};
#endif // !_COMMON_H