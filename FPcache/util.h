#ifndef UTIL_H_
#define UTIL_H_
#define NODE_NUM_MAX 5
#include "lru.hpp"
#include "fpcache.hpp"
double generateGaussianNoise(double mu, double sigma);
int radmGen(size_t low, size_t high, int);
int sampTheNext(float a, float rate, size_t curr, size_t M);
void uniAccess(LRUStack& lru,
	FPCache&fpcahe,
	std::vector<Transaction>&transactions,
	std::vector<Transaction>&temptrans,
	size_t M,
	float rate);
#endif // !UTIL_H_