#include <iostream>
#include <vector>

using namespace std;
int MaxDiff(const int *numbers, unsigned length);
int main() {
	int a[7] = { 9,3,18,11,4,22,17 };
	cout << MaxDiff(a, 7);
	int MaxDiff(const int *numbers, unsigned length);
	system("pause");
	return 0;
}

//
int MaxDiff(const int *numbers, unsigned length) {
	if (numbers == NULL || length < 2)return 0;

	int min = numbers[length-1];
	int resu = numbers[length - 2] - min;

	for (int i = length-2; i != -1; i--) {
		//检查前一车厢的值是不是最小值，是就更新最小值
		if (numbers[i + 1] < min) {
			min = numbers[i + 1];
		}

		//当前时间是最多人数差吗
		if ((numbers[i] - min) > resu) {
			resu = numbers[i] - min;
		}
	}

	return resu;
}