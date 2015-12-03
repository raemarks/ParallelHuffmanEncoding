#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include "HuffmanInternalNode.h"
#include "HuffmanLeafNode.h"
class HuffmanTree
{
	private:
		HuffmanNode *_root;

		void writeTabs(int n)
		{
			for (int i = 0; i < n; i++) {printf("+");}
		}

		void printTreeRec(HuffmanNode *nd, int depth)
		{
			writeTabs(depth);
			printf("[");
			PrintNodeLabel(nd);
			printf("]");

			if (nd == NULL) {
				printf("\n");
				return;
			}
			else if (nd->IsLeaf()) {
				printf("\n");
				return;
			}
			else {
				printf("{");
				HuffmanInternalNode *in = (HuffmanInternalNode*) nd;
				printf("\n");
				//if (in->GetLeftChild() != NULL) {
					printTreeRec(in->GetLeftChild(), depth+1);
				//}
				//if (in->GetRightChild() != NULL) {
					printTreeRec(in->GetRightChild(), depth+1);
				//}

				for (int i = 0; i < depth; ++i) {
					printf("+");
				}
				printf("}\n");
			}
		}

		void PrintNodeLabel(HuffmanNode *n)
		{
			if (n == NULL)
				printf("NULL");
			else if (n->IsLeaf()) {
				HuffmanLeafNode *ln = (HuffmanLeafNode*) n;

				if (ln->GetValue() == '\n')
					printf("ln:\\n");
				else if (ln->GetValue() == '\r')
					printf("ln:\\r");
				else
					printf("ln:%c", ln->GetValue());
			}
			else {
				printf("in");
			}
		}


	public:
		HuffmanTree(const char value, const int weight)
		{
			_root = new HuffmanLeafNode(value, weight);
		}

		HuffmanNode *GetRoot()
		{
			return _root;
		}

		void Print()
		{
			printTreeRec(_root, 0);
		}

		HuffmanTree(HuffmanTree *left, HuffmanTree *right)
		{
			_root = new HuffmanInternalNode(left->GetRoot(), right->GetRoot());
		}

		HuffmanTree(HuffmanInternalNode *root)
		{
			_root = root;
		}

		~HuffmanTree()
		{
			vector<HuffmanNode *> nodes{};
			nodes.push_back(_root);
			while (nodes.size() > 0)
			{
				HuffmanNode *last = nodes[nodes.size() - 1];
				nodes.pop_back();
				HuffmanInternalNode *casted_node =
					dynamic_cast<HuffmanInternalNode *>(last);

				//make sure that we're dealing with an actual
				//binary node
				if (casted_node != nullptr) {
					//add last's children to the stack
					nodes.push_back(casted_node->GetLeftChild());
					nodes.push_back(casted_node->GetRightChild());

					//having gotten all of the information out of
					//last, we can now delete the node
					delete casted_node;
				}
			}
		}

		int GetWeight()
		{
			return _root->GetWeight();
		}
};

#endif
