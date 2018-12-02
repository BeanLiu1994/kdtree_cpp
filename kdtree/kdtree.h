#pragma once

#include <array>
#include <memory>
#include <algorithm>
#include <functional>
#include <stack>
#include <vector>

template<typename ty, int dims>
struct DataType
{
	typedef std::array<ty, dims> data_type;
	typedef ty value_type;
	constexpr static int dimensions = dims;

	const data_type* data;
	int ind;

	DataType() = default;
	DataType(const DataType&) = default;
	DataType(DataType&&) noexcept = default;
	DataType& operator=(DataType rhs)
	{
		this->swap(rhs);
		return *this;
	}

	DataType(const data_type vec[], int pos)
		:data(vec), ind(pos) {}
public:
	const data_type& getData() const
	{
		return data[ind];
	}
	const ty& operator[](size_t pos) const
	{
		return getData()[pos];
	}
	void swap(DataType& rhs) 
	{
		using std::swap;
		if (this->data != rhs.data)
			throw std::runtime_error("swap elements from different array is not allowed.");
		swap(this->ind, rhs.ind);
	}
}; 

template<typename ValType1, typename ValType2>
double EuclideanDistance(const ValType1& p1, const ValType2& p2)
{
	static_assert(ValType1::dimensions == ValType2::dimensions, "dimension doesn't match.");
	double ret = 0;
	for (int i = 0; i < ValType1::dimensions; ++i)
	{
		ret += std::pow(p1[i] - p2[i], 2);
	}
	return std::sqrt(ret);
}

template<typename ty, int dims>
double EuclideanDistance(const std::array<ty, dims>& p1, const DataType<ty, dims>& p2)
{
	double ret = 0;
	for (int i = 0; i < dims; ++i)
	{
		ret += std::pow(p1[i] - p2[i], 2);
	}
	return std::sqrt(ret);
}

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
		:parent(Parent), val(std::move(Val)), split_dim(Split_dim), children({ left, right }) {}
};

template<typename ValType>
struct KdTree
{
	typedef KdNode<ValType> NodeType;

	NodeType* root = nullptr;

	KdTree() = default;
	KdTree(const KdTree&) = default;
	KdTree(KdTree&&) = default;

	KdTree(const typename ValType::data_type data[], int size)
	{
		std::vector<ValType> data_ref;
		for (int i = 0; i < size; ++i)
		{
			data_ref.emplace_back(data, i);
		}

		root = BuildKdTree(data_ref.data(), size);
	}
	~KdTree()
	{
		ReleaseKdTree(root);
	}

	std::pair<NodeType*, double> Query(const typename ValType::data_type& item)
	{
		return QueryNearestNode(root, item);
	}

private:

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
			std::bind(dim_compare<ValType>, std::placeholders::_1, std::placeholders::_2, split_dim)
		);

		return split_dim;
	}

	NodeType* BuildKdTree(typename ValType data[], int size, NodeType* parent = nullptr)
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
			new_node->children[1] = BuildKdTree(data + mid_split_index + 1, size - mid_split_index - 1, new_node);

			//std::cout << "当前节点 " << new_node->val.ind << " , 分割维度: " << new_node->split_dim;
			//if (new_node->children[0])
			//	std::cout << " , 左 " << new_node->children[0]->val.ind;
			//if (new_node->children[1])
			//	std::cout << " , 右 " << new_node->children[1]->val.ind;
			//std::cout << std::endl;
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

	std::pair<NodeType*, double> QueryNearestNode(NodeType* tree_root, const typename ValType::data_type& value) const
	{
		std::stack<NodeType*> path;
		NodeType* p = tree_root;
		while (p)
		{
			path.push(p);
			if (value[p->split_dim] < p->val[p->split_dim])
				p = p->children[0];
			else
				p = p->children[1];
		}
		//由根开始到叶的path, 开始回溯
		p = path.top();
		double minDistNow = EuclideanDistance(value, p->val);
		while (!path.empty())
		{
			auto current = path.top();
			path.pop();

			double currentDistNow = EuclideanDistance(value, current->val);
			if (minDistNow > currentDistNow)
			{
				minDistNow = currentDistNow;
				p = current;
			}

			double DistToSplitFace = value[current->split_dim] - current->val[current->split_dim];
			if (minDistNow > std::abs(DistToSplitFace))
			{
				NodeType* min_node_subtree;
				double min_distance_subtree = -1;
				if (DistToSplitFace > 0 && current->children[0] != nullptr)
					std::tie(min_node_subtree, min_distance_subtree) = QueryNearestNode(current->children[0], value);
				else if (current->children[1] != nullptr)
					std::tie(min_node_subtree, min_distance_subtree) = QueryNearestNode(current->children[1], value);
				if (min_distance_subtree > 0 && minDistNow > min_distance_subtree)
				{
					minDistNow = min_distance_subtree;
					p = min_node_subtree;
				}
			}
		}

		return std::make_pair(p, minDistNow);
	}
};