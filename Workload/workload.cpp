#include <stdio.h>
#include <memory>
#include <stdio.h>
#include <random>  
#include <string>
#include <fstream>
#include <iostream>

using std::string;
int numberOf1(int n)
{
	int count = 0;
	while (n) {
		++count;
		n = (n - 1) & n;
	}
	return count;
}
int radmGen(size_t low, size_t high, int) {

	static std::default_random_engine e;
	std::uniform_int_distribution<unsigned> u(low, high);
	/*
	std::default_random_engine random(time(NULL));
	std::uniform_int_distribution<int> dis1(low, high);
	*/
	return u(e);
}
double generateGaussianNoise(double mu, double sigma)
{
	const double epsilon = 2.2250738585072014e-308;
	const double two_pi = 2.0*3.14159265358979323846;

	static double z0, z1;
	static bool generate;
	generate = !generate;

	if (!generate)
		return z1 * sigma + mu;

	double u1, u2;
	do
	{
		u1 = rand() * (1.0 / RAND_MAX);
		u2 = rand() * (1.0 / RAND_MAX);
	} while (u1 <= epsilon);

	z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
	z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2);
	return z0 * sigma + mu;
}

void main()
{
	std::ofstream out;
	int transNum = 1000000;
	string patterns[10] = { "100001 100002 100003 100004 100005",
							"200001 200002 200003 200004 200005",
							"300001 300002 300003 300004 300005",
							"400001 400002 400003 400004 400005",
							"500001 500002 500003 500004 500005",
							"600001 600002 600003 600004 600005",
							"700001 700002 700003 700004 700005",
							"800001 800002 800003 800004 800005",
							"900001 900002 900003 900004 900005",
							"110001 110002 110003 110004 110005" };
	//string patterns[10] = {"q w e r t","y u i o p","a s d f g","h j k l !","z x c v b","n m @ # $","^ & * ( )","_ + { } |",": ' \" % ,","/ ? > < ."};
	out.open("workload.dat", std::ios::out | std::ios::app);
	for (size_t i = 0; i < transNum; i++)
	{
		
		int idx = radmGen(0, 15, 1);
		for (size_t j = 0; j < 16; j++) {
			if (j== idx)
			{
				if (idx != 0) {
					out << " ";
				}
				int patidx=abs(generateGaussianNoise(0, 6));
				if (patidx<=9&& patidx>=0)
				{
					string pattern = patterns[patidx];
					out << pattern;
				}
				else
				{
					for (size_t k = 0; k < 5; k++)
					{
						int item = radmGen(0, 20000, 1);
						out << item ;
						if (k!=4)
						{
							out << " ";
						}
					}
				}
				if (idx == 15)
				{
					out << std::endl;
					break;
				}
				else
				{
					out << " ";
				}
			}

			int item=radmGen(0, 20000, 1);
			out << item;
			if (j!=15)
			{
				out << " ";
			}
			else
			{
				out << std::endl;
			}
		}
	}
	out.close();
	
}
