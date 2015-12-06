#ifndef HUFFMAN_LEAF_NODE
#define HUFFMAN_LEAF_NODE

#include "HuffmanNode.h"
class HuffmanLeafNode : public HuffmanNode
{
private:
	char _value;
public:
	HuffmanLeafNode(char value, const int frequency) : HuffmanNode()
	{
		_value = value;
		_weight = frequency;
		if (value < 0) {
			//printf("VALUE IS NEGATIVE %d\n", (int)value);
		}
	}

	~HuffmanLeafNode() {}

	const char &GetValue()
	{
		return _value;
	}

	virtual bool IsLeaf()
	{
		return true;
	}
};

#endif
