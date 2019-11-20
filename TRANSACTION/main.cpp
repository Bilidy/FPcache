#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <vector>
using namespace std;

template <class Type>
Type stringToNum(const string& str) {
	istringstream iss(str);
	Type num;
	iss >> num;
	return num;
}

int main() {
	map<std::string, uint64_t> hashtable;
	uint64_t NO=0;
	ifstream fin;
	ofstream fout;
	string src_path;
	string dis_path;
	int gap=1500;
	cout << "Path:";
	cin >> src_path;
	cout << "Output file name:";
	cin >> dis_path;
	cout << "Gap:";
	cin >> gap;
	fin.open(src_path);
	fout.open(dis_path);
	uint64_t lasttimestamp = 0;
	string timestamp;
	string accsrc;
	string flag;
	string type;
	string itemnum;
	string size;
	string last;
	//int max = 200000;
	//fin >> timestamp >> accsrc >> flag >> type >> itemnum >> size >> last;
	
	string line;
	//while (!fin.eof()&&max--)
	vector<uint64_t> transaction;
		map<uint64_t,uint16_t> map;
	while (getline(fin, line))
	{
		istringstream sin(line);
		//fin >> timestamp >> accsrc >> flag >> type >> itemnum >> size >> last;
		vector<string> fields;
		string field;
		while (getline(sin, field, ',')) //将字符串流sin中的字符读入到field字符串中，以逗号为分隔符
		{
			fields.push_back(field); //将刚刚读取的字符串添加到向量fields中
		}
		timestamp = fields[0];
		accsrc = fields[1];
		flag = fields[2];
		type = fields[3];
		itemnum = fields[4];
		size = fields[5];
		last = fields[6];
		uint64_t currtimestamp;
		if (hashtable.find(itemnum) == hashtable.end()) {
			NO++;
			hashtable[itemnum] = NO;//提供映射
		};
		if (lasttimestamp==0)
		{
			lasttimestamp = stringToNum<uint64_t>(timestamp.c_str());
		}
		currtimestamp = stringToNum<uint64_t>(timestamp.c_str());
		if((currtimestamp- lasttimestamp)< gap)
		{
			if (map.find(hashtable[itemnum]) == map.end()) {
				map[hashtable[itemnum]] = 1;
				transaction.push_back(hashtable[itemnum]);
			}
			
		}
		else
		{
			auto it = transaction.begin();
			while (it != transaction.end())
			{
				fout << (*it)<<" ";
				it++;
			}
			fout << endl;
			transaction.clear();
			map.clear();
			map[hashtable[itemnum]] = 1;
			transaction.push_back(hashtable[itemnum]);
		}
		lasttimestamp = currtimestamp;
	}
	fin.close();
	fout.close();
	cout << "Finished";
}