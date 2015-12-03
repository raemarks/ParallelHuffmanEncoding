#ifndef HUFFMAN_NODE_H
#define HUFFMAN_NODE_H

class HuffmanNode {
protected:
	int _weight;
public:

	/* Gets weight of huffman node and children */
	virtual int GetWeight()
	{
		return _weight;
	}

	virtual ~HuffmanNode() {}

	/* Determines if node is a leaf or not */
	virtual bool IsLeaf() = 0;
};

#endif
