#include "transactions.h"

void split(std::string srcstr, char splitby, std::vector<std::string> &_substr) {
	if ("" != srcstr && splitby)
	{
		size_t pos = 0;
		while (true)
		{
			pos = srcstr.find(splitby);
			if (pos >= srcstr.size()) { 
				if (srcstr!="") {
					_substr.push_back(srcstr); 
				}break;
			}
			string distr = srcstr.substr(0, pos);
			srcstr = srcstr.substr(pos + 1, srcstr.size());
			if (distr!=""){
				_substr.push_back(distr);
			}
		}
	}
}

void transactions(const char* src_path) {
	std::ifstream fin;
	std::ofstream fout;

	fin.open(src_path);
	//std::map(std::pair<std::string,std::vector<string>>);
	std::map<std::string, std::map<std::string, size_t>> classifier;
	while (!fin.eof())
	{
		//156:usertable-user1362430418221793826
		std::string str;

		getline(fin, str);

		size_t pos = str.find_first_of(':');
		if (str == "")
		{
			break;
		}

		std::string sfd = str.substr(0, pos);

		pos = str.find_first_of('-');
		std::string userkey = str.substr(pos + 1, str.length());

		pos = userkey.find_first_of('r');
		std::string key = userkey.substr(pos + 1, str.length());

		std::map<std::string, std::map<std::string, size_t>>::iterator it;

		it = classifier.find(sfd);
		if (it != classifier.end())//find sfd in map
		{
			std::map<std::string, size_t>::iterator keymapit = (*it).second.find(key);
			if (keymapit != (*it).second.end()) {//find key in sdf map
				(*keymapit).second++;
			}
			else//specfied key not exist in key map
			{
				std::map<std::string, size_t> vec;
				(*it).second.insert(std::pair<std::string, size_t>(key, 1));//put new key in key map
			}
			//std::cout << key << std::endl;
		}
		else//specfied sfd not exist in sdf map
		{
			std::map<std::string, size_t> vec;
			vec.insert(std::pair<std::string, size_t>(key, 1));//put new key in key map
			classifier.insert_or_assign(sfd, vec);//put new sdf in sdf map
		}
		//classifier.at(sfd).push_back(key);
		//std::cout << str << std::endl;

	}
	fin.close();
	fout.open("trans.txt", 'w');
	fout.flush();
	std::map<std::string, std::map<std::string, size_t>>::iterator it;
	it = ++classifier.begin();

	size_t tag = 0;
	while (it != classifier.end())
	{
		bool flag = false;

		std::map<std::string, size_t>::iterator vecit;
		vecit = (*it).second.begin();

		while (vecit != (*it).second.end())
		{
			if ((*vecit).second > 0)
			{
				if (!flag)//the first
				{
					flag = true;
					fout << (*it).first;
					fout << '0' << tag << '	';
					fout << ((*vecit).first);
					(*vecit).second--;
				}
				else//not the first
				{
					flag = true;

					fout << ',';
					fout << ((*vecit).first);
					(*vecit).second--;
				}
			}
			vecit++;
		}

		fout.flush();
		if (!flag) {//not exist
			it++;
			tag = 0;
		}
		else {
			fout << '\n';
			tag++;
		}
		//fout.close();
	}
	fout.close();
}
void transactions(const string src_path, std::vector<Transaction> &transactions) {
	std::ifstream fin;
	std::ofstream fout;

	fin.open(src_path);
	
	
	while (!fin.eof())
	{
		std::string str;
		std::vector<std::string> _substr;
		Transaction trans;
		getline(fin, str);
		if (str == "")
		{
			break;
		}
		split(str, '	', _substr);
		split(_substr[1], ',', trans);
		transactions.push_back(trans);
	}
	fin.close();
}
