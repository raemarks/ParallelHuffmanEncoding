#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include <stdint.h>
#include <string>
#include <stdio.h>

#include "Constants.h"
#include "HuffmanInternalNode.h"
#include "HuffmanLeafNode.h"


class HuffmanTree
{
	private:
		HuffmanNode *_root;

		void writeTabs(FILE *outfile, int n)
		{
			for (int i = 0; i < n; i++) {fprintf(outfile, "+");}
		}

		void printTreeRec(HuffmanNode *nd, int depth, FILE *outfile)
		{
			writeTabs(outfile, depth);
			fprintf(outfile, "[");
			PrintNodeLabel(outfile, nd);
			fprintf(outfile, "]");

			if (nd == NULL) {
				fprintf(outfile, "\n");
				return;
			}
			else if (nd->IsLeaf()) {
				fprintf(outfile, "\n");
				return;
			}
			else {
				fprintf(outfile, "{");
				HuffmanInternalNode *in = (HuffmanInternalNode*) nd;
				fprintf(outfile, "\n");
				//if (in->GetLeftChild() != NULL) {
					printTreeRec(in->GetLeftChild(), depth+1, outfile);
				//}
				//if (in->GetRightChild() != NULL) {
					printTreeRec(in->GetRightChild(), depth+1, outfile);
				//}

				for (int i = 0; i < depth; ++i) {
					fprintf(outfile, "+");
				}
				fprintf(outfile, "}\n");
			}
		}

		void PrintNodeLabel(FILE *outfile, HuffmanNode *n)
		{
			if (n == NULL)
				fprintf(outfile, "NULL");
			else if (n->IsLeaf()) {
				HuffmanLeafNode *ln = (HuffmanLeafNode*) n;

				fprintf(outfile, "ln:%d", (int)ln->GetValue());
			}
			else {
				fprintf(outfile, "in");
			}
		}


	public:
		/* Copy constructor - just make it a shallow copy. */
		HuffmanTree(const HuffmanTree &ht) : _root(ht._root)
	{
		printf("In copy constructor for huffmantree\n");
	}

		HuffmanTree(const char value, const uint64_t weight) :
			_root(new HuffmanLeafNode(value, weight))
		{ }

		HuffmanNode *GetRoot()
		{
			return _root;
		}

		HuffmanTree& operator=(const HuffmanTree &other)
		{
			if (this != &other) {
				_root = other._root;
			}
			return *this;
		}

		void Print()
		{
			std::string filename = "tree_" + std::to_string((long long int) mpirank);
			FILE *outfile = fopen(filename.c_str(), "w");
			printTreeRec( _root, 0, outfile);
			fclose(outfile);
		}

		HuffmanTree(HuffmanTree *left, HuffmanTree *right) :
			_root(new HuffmanInternalNode(left->GetRoot(), right->GetRoot()))
		{
		}

		HuffmanTree(HuffmanInternalNode *root) : _root(root)
		{ }

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
				if (casted_node != NULL) {
					//add last's children to the stack
					nodes.push_back(casted_node->GetLeftChild());
					nodes.push_back(casted_node->GetRightChild());

					//having gotten all of the information out of
					//last, we can now delete the node
					delete casted_node;
				}
			}
		}

		uint64_t GetWeight()
		{
			return _root->GetWeight();
		}
};

#endif
