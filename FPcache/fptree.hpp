#ifndef FPTREE_HPP


#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <utility>

struct metaPattern
{
	double val;
	double accden;
	uint64_t size;
	uint16_t sup;
	uint16_t mean;
	uint16_t var;
};

using Item = std::string;
using Transaction = std::vector<Item>;
using TransformedPrefixPath = std::pair<std::vector<Item>, uint64_t>;
using Pattern = std::pair<std::set<Item>, uint64_t>;
using valuatedPattern = std::pair<std::set<Item>, metaPattern>;


struct FPNode {
	const Item item;
	uint64_t frequency;
	std::shared_ptr<FPNode> node_link;
	std::weak_ptr<FPNode> parent;
	std::vector<std::shared_ptr<FPNode>> children;

	FPNode(const Item&, const std::shared_ptr<FPNode>&);
};

struct FPTree {
	std::shared_ptr<FPNode> root;
	std::map<Item, std::shared_ptr<FPNode>> header_table;
	uint64_t minimum_support_threshold;

	FPTree(const std::vector<Transaction>&, uint64_t);

	bool empty() const;
};


std::set<Pattern> fptree_growth(const FPTree&);


#endif  // FPTREE_HPP
#define FPTREE_HPP