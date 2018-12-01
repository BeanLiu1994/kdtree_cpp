#pragma once
#include <array>
#include <memory>
#include <algorithm>
#include <functional>

template<typename ty, int dims>
struct DataType
{
	typedef ty value_type;
	constexpr static int dimensions = dims;

	std::array<ty, dims> data;

public:
	ty& operator[](size_t pos) {
		return data[pos];
	}	
	const ty& operator[](size_t pos) const {
		return data[pos];
	}
};

template<typename ValType>
inline bool dim_compare(const ValType& l, const ValType& r, size_t dim)
{
	return l[dim] < r[dim];
}

template<typename ValType>
struct KdNode
{
	KdNode* parent;
	ValType val;
	int split_dim;
	std::array<KdNode*, 2> children;

	KdNode(KdNode* Parent, ValType Val, int Split_dim)
		:parent(Parent), val(std::move(Val)), split_dim(Split_dim) {}
	KdNode(KdNode* Parent, ValType Val, int Split_dim, KdNode* left, KdNode* right)
		:parent(Parent), val(std::move(Val)), split_dim(Split_dim), children({left, right}) {}
};

template<typename ValType>
struct KdTree
{
	typedef KdNode<ValType> NodeType;

	NodeType* root = nullptr;

	KdTree() = default;
	KdTree(const KdTree&) = default;
	KdTree(KdTree&&) = default;

	KdTree(ValType data[], int size)
	{
		root = BuildKdTree(data, size);
	}
	~KdTree()
	{
		ReleaseKdTree(root);
	}

private:

	int ChooseSplitDim(ValType data[], int size)
	{
		//ʹ�÷�����Ϊ��������
		std::array<double, ValType::dimensions> split_judge;
		for (int dim = 0; dim < ValType::dimensions; ++dim)
		{
			double tmp1 = 0, tmp2 = 0;
			for (int i = 0; i < size; ++i)
			{
				tmp1 += 1.0 / (double)size * data[i][dim] * data[i][dim];
				tmp2 += 1.0 / (double)size * data[i][dim];
			}
			split_judge[dim] = tmp1 - tmp2 * tmp2; //����ά�ȵķ���
		}
		auto pos = std::max_element(split_judge.begin(), split_judge.end());

		//����ά������
		int split_dim = pos - split_judge.begin();

		sort(data, data + size,
			std::bind(dim_compare<ValType>, std::placeholders::_1, std::placeholders::_2, split_dim)
		);

		return split_dim;
	}

	NodeType* BuildKdTree(ValType data[], int size, NodeType* parent = nullptr)
	{
		//����ChooseSplitDim���򲢷ָ�
		if (size <= 0)
		{
			return nullptr;
		}
		else
		{
			int split = ChooseSplitDim(data, size); //�˺����ź��򣬷ָ�����
			size_t mid_split_index = size / 2;

			auto new_node = new NodeType(parent, data[mid_split_index], split);
			new_node->children[1] = BuildKdTree(data + mid_split_index + 1, mid_split_index - 1, new_node);
			new_node->children[0] = BuildKdTree(data, mid_split_index, new_node);
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
};