#ifndef HUFFMAN_INTERNAL_NODE
#define HUFFMAN_INTERNAL_NODE

#include <vector>
#include <cstdlib>

#include "HuffmanNode.h"

using std::vector;

class HuffmanInternalNode : public HuffmanNode
{
	private:
		HuffmanNode *_left_child;
		HuffmanNode *_right_child;

	public:
		HuffmanInternalNode(HuffmanNode *left_child,
				HuffmanNode *right_child) : HuffmanNode()
	{
		if (left_child != NULL && right_child != NULL)
		{
			_weight = left_child->GetWeight() + right_child->GetWeight();
		}
		_left_child = left_child;
		_right_child = right_child;
	}

		~HuffmanInternalNode() {}
		//internal nodes are not leaf nodes!
		virtual bool IsLeaf()
		{
			return false;
		}

		HuffmanNode *GetLeftChild() const
		{
			return _left_child;
		}

		HuffmanNode *GetRightChild() const
		{
			HuffmanNode *c;
			if (this == NULL) {
				printf("this is null!\n");
			}
			c = _right_child;
			return c;
		}

		void SetLeftChild(HuffmanNode *node)
		{
			_left_child = node;
		}

		void SetRightChild(HuffmanNode *node)
		{
			_right_child = node;
		}
};

#endif
