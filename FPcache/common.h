#ifndef _COMMON_H

#define _COMMON_H

#define WINDOW 64
#define STAY 110
#define OLD 0.95
#define NEW 0.05
#define TIME_SLICE 5
#define RECORDE_STEP 10000

#include <cstdlib>
#include <random>  
#include <time.h>  
#include <string>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <stdio.h>

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
int radmGen(size_t low, size_t high, int);
double radmGen(size_t low, size_t high, double);

double getSpatialMean(std::vector<metadata>& randmselect);
double getSpatialVan(std::vector<metadata>& randmselect, double mean);

double getTemporalMean(std::vector<metadata>& randmselect);
double getTemporalVan(std::vector<metadata>& randmselect, double mean);
#endif // !_COMMON_H