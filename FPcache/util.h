#ifndef UTIL_H_
#define UTIL_H_
#define NODE_NUM_MAX 5
#include "lru.hpp"
#include "fpcache.hpp"
#include "ARCCache.h"
#include <stdlib.h>
double generateGaussianNoise(double mu, double sigma);
int radmGen(size_t low, size_t high, int);
int sampTheNext(float a, float rate, size_t curr, size_t M);

void uniAccess(LRUStack& lru, FPCache&fpcahe, ARCCache&accache,
	std::vector<Transaction>&transactions,
	std::vector<Transaction>&temptrans,
	size_t M,
	float rate,
	float a);

void uniAccess(LRUStack& lru,FPCache&fpcahe, ARCCache&accache,
	std::vector<Transaction>&transactions,
	std::vector<Transaction>&w_transactions,
	std::vector<Transaction>&temptrans,
	size_t M,
	float rate,
	float a	,
	size_t skew_jump_low,
	size_t skew_jump_high);

void skewWorkLoad(std::vector<Transaction>& skew_transactions, size_t skew_low, size_t skew_high);
#endif // !UTIL_H_