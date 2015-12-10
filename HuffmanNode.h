#ifndef HUFFMAN_NODE_H
#define HUFFMAN_NODE_H

class HuffmanNode {
protected:
	uint64_t _weight;
public:

	HuffmanNode() : _weight(0) {}

	/* Gets weight of huffman node and children */
	virtual uint64_t GetWeight()
	{
		return _weight;
	}

	virtual ~HuffmanNode() {}

	/* Determines if node is a leaf or not */
	virtual bool IsLeaf() = 0;
};

#endif
