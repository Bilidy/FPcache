#pragma once
#include <fstream>
#include <vector>
#include <map>
#include <string>

using Item = std::string;
using Transaction = std::vector<Item>;
using namespace std;
void transactions(const char* src_path);
void transactions(const string src_path, std::vector<Transaction>& transactions);
void split(std::string srcstr, char splitby, std::vector<std::string> &_substr);
std::vector<std::string> split(std::string srcstr, char splitby);
void transactions(const string src_path, std::vector<Transaction> &transactions, int);