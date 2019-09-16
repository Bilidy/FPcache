#include "common.h"
int radmGen(size_t low, size_t high, int) {

	static std::default_random_engine e;
	std::uniform_int_distribution<unsigned> u(low, high);
	/*
	std::default_random_engine random(time(NULL));
	std::uniform_int_distribution<int> dis1(low, high);
	*/
	return u(e);
}
double radmGen(size_t low, size_t high, double) {
	std::default_random_engine random(time(NULL));
	std::uniform_real_distribution<double> dis2(0.0, 1.0);
	return dis2(random);
}
double getSpatialMean(std::vector<metadata>& randmselect) {
	double sum = 0;
	for (auto i = randmselect.begin(); i != randmselect.end(); i++)
	{
		sum += i->weidis;
	}
	return sum / randmselect.size();
}
double getSpatialVan(std::vector<metadata>& randmselect,double mean) {
	double sum = 0;
	for (auto i = randmselect.begin(); i != randmselect.end(); i++)
	{
		sum += (i->weidis-mean)*(i->weidis - mean);
	}
	return sqrt(sum/ randmselect.size());
}

double getTemporalMean(std::vector<metadata>& randmselect)
{
	double sum = 0;
	for (auto i = randmselect.begin(); i != randmselect.end(); i++)
	{
		sum += i->accnum;
	}
	return sum / randmselect.size();
}

double getTemporalVan(std::vector<metadata>& randmselect, double mean)
{
	double sum = 0;
	for (auto i = randmselect.begin(); i != randmselect.end(); i++)
	{
		sum += (i->accnum - mean)*(i->accnum - mean);
	}
	return sqrt(sum / randmselect.size());
}
