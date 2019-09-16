#ifndef UTIL_H_
#define UTIL_H_
#define NODE_NUM_MAX 5

#include "lru.hpp"
#include "fpcache.hpp"
#include "RR.h"
#include "ARCCache.h"


double generateGaussianNoise(double mu, double sigma);
int sampTheNext(float a, float rate, size_t curr, size_t M);
void uniAccess(LRUStack& lru, FPCache&fpcahe, ARCCache&accache, RR&random,
	std::vector<Transaction>&transactions,
	std::vector<Transaction>&temptrans,
	size_t M,
	float rate,
	float a,
	string outputfile,
	int _type);

void uniAccess(LRUStack& lru,FPCache&fpcahe, ARCCache&accache, RR&random,
	std::vector<Transaction>&transactions,
	std::vector<Transaction>&w_transactions,
	std::vector<Transaction>&temptrans,
	size_t M,
	float rate,
	float a	,
	size_t skew_jump_low,
	size_t skew_jump_high,
	float olddisWei,
	float newdisWei,
	string output,
	int _type);
void uniAccess(LRUStack& lru,
	FPCache&fpcahe,
	std::vector<Transaction>&transactions,
	std::vector<Transaction>&temptrans,
	float olddis,
	float newdis,
	size_t M,
	float rate,
	float a,
	string outputfile,
	int _type
);
void skewWorkLoad(std::vector<Transaction>& skew_transactions, size_t skew_low, size_t skew_high);
#endif // !UTIL_H_