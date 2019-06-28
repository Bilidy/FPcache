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