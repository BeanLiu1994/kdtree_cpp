#include "kdtree.h"
#include <functional>
#include <ctime>

using ValType = DataType<double, 2>;
using NodeType = KdNode<ValType>;
NodeType* root = nullptr;

bool dim_compare(const ValType& l, const ValType& r, size_t dim)
{
	return l[dim] < r[dim];
}

int ChooseSplitDim(ValType data[], int size)
{
	//使用方差作为评判依据
	std::array<double, ValType::dimensions> split_judge;
	for (int dim = 0; dim < ValType::dimensions; ++dim)
	{
		double tmp1 = 0, tmp2 = 0;
		for (int i = 0; i < size; ++i)
		{
			tmp1 += 1.0 / (double)size * data[i][dim] * data[i][dim];
			tmp2 += 1.0 / (double)size * data[i][dim];
		}
		split_judge[dim] = tmp1 - tmp2 * tmp2; //各个维度的方差
	}
	auto pos = std::max_element(split_judge.begin(), split_judge.end());
	
	//根据维度排序
	int split_dim = pos - split_judge.begin();

	sort(data, data + size, 
		std::bind(dim_compare, std::placeholders::_1, std::placeholders::_2, split_dim)
	);

	return split_dim;
}

NodeType* BuildKdTree(ValType data[], int size, NodeType* parent = nullptr)
{
	//利用ChooseSplitDim排序并分割
	if (size <= 0) 
	{
		return nullptr;
	}
	else
	{
		int split = ChooseSplitDim(data, size); //此后已排好序，分割数据
		size_t mid_split_index = size / 2;
		
		auto new_node = new NodeType(parent, data[mid_split_index], split);
		new_node->children[0] = BuildKdTree(data, mid_split_index, new_node);
		new_node->children[1] = BuildKdTree(data + mid_split_index + 1, mid_split_index - 1, new_node);
		return new_node;
	}
}

void ReleaseKdTree(NodeType* node = root)
{
	if (node)
	{
		ReleaseKdTree(node->children[0]);
		ReleaseKdTree(node->children[1]);
		delete node;
	}
}


int main()
{
	srand(std::time(nullptr));
	std::vector<ValType> test_data(6);
	for (int i = 0; i < test_data.size(); ++i)
	{
		for (int dim = 0; dim < ValType::dimensions; ++dim)
		{
			test_data[i][dim] = rand() % 1000;
		}
	}

	auto root = BuildKdTree(test_data.data(), test_data.size());

	ReleaseKdTree(root);

	return 0;
}