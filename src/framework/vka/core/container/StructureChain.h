#pragma once
#include <list>
#include "../core_common.h"
#include "../macros/macros.h"
#include "../utility/misc.h"


// from https://github.com/vcoda/magma
namespace vka
{

class StructureChain
{
  public:
	template <class StructureType>
	void addNode(const StructureType &node);

	VkBaseOutStructure *firstNode() const;

	VkBaseOutStructure *lastNode() const;

	uint32_t getSize() const;

	bool empty() const;

	VkBaseOutStructure *chainNodes() const;
	hash_t              getHash() const;

  private:
	class Node;
	std::list<Node> chain;
};

uint32_t StructureChain::getSize() const
{
	return VKA_COUNT(chain);
}

bool StructureChain::empty() const
{
	return chain.empty();
}
class StructureChain::Node
{
  public:
	template <class StructureType>
	Node(const StructureType &node);
	Node(const Node &node);
	Node(Node &&node) noexcept;
	~Node();
	VkBaseOutStructure *getNode() const;
	size_t              getSize() const
	{
		return size;
	}

  private:
	size_t size;
	void  *data;
};

template <class StructureType>
inline StructureChain::Node::Node(const StructureType &node) :
    size(sizeof(StructureType))
{
	static_assert(sizeof(StructureType) > sizeof(VkBaseInStructure),
	              "chain structure size is too little");
	static_assert(std::is_trivially_copyable<StructureType>::value,
	              "chain structure required to be trivially copyable");
	data = copyBinaryData(node);
}

inline StructureChain::Node::Node(const Node &node) :
    size(node.size)
{
	data = copyBinaryData(node.data, node.size);
}

inline StructureChain::Node::Node(Node &&node) noexcept :
    size(node.size),
    data(node.data)
{
	node.size = 0;
	node.data = nullptr;
}

inline StructureChain::Node::~Node()
{
	delete[] reinterpret_cast<char *>(data);
}

inline VkBaseOutStructure *StructureChain::Node::getNode() const
{
	return reinterpret_cast<VkBaseOutStructure *>(data);
}

template <class StructureType>
inline void StructureChain::addNode(const StructureType &node)
{
	chain.emplace_back(node);
}

inline VkBaseOutStructure *StructureChain::firstNode() const
{
	return chain.empty() ? nullptr : chain.begin()->getNode();
}

inline VkBaseOutStructure *StructureChain::lastNode() const
{
	return chain.empty() ? nullptr : chain.rbegin()->getNode();
}

VkBaseOutStructure *StructureChain::chainNodes() const
{
	if (chain.empty())
		return nullptr;
	auto head = chain.begin(), curr = head, next = head;
	while (++next != chain.end())
	{
		VkBaseOutStructure *node = curr->getNode();
		node->pNext              = next->getNode();
		curr                     = next;
	}
	VkBaseOutStructure *last = curr->getNode();
	last->pNext              = nullptr;
	return head->getNode();
}

hash_t StructureChain::getHash() const
{
	if (chain.empty())
		return 0;
	auto   node = chain.cbegin();
	hash_t hash = hashArray((const uint8_t *) node->getNode(), node->getSize());
	while (++node != chain.cend())
	{
		hash_t nodeHash = hashArray((const uint8_t *) node->getNode(), node->getSize());
		hashCombine(hash, nodeHash);
	}
	return hash;
}

#define VKA_SPECIALIZE_STRUCTURE_CHAIN_NODE(StructureType, structureType)         \
	template <>                                                                   \
	inline StructureType *vka::StructureChain::findNode<StructureType>() const    \
	{                                                                             \
		auto it = std::find_if(chain.begin(), chain.end(),                        \
		                       [](auto &it) {                                     \
			                       return (it.getNode()->sType == structureType); \
		                       });                                                \
		if (it != chain.end())                                                    \
			return reinterpret_cast<StructureType *>(it->getNode());              \
		return nullptr;                                                           \
	}

}
