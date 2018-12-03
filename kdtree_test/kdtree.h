#pragma once

#include <array>
#include <memory>
#include <algorithm>
#include <functional>
#include <stack>
#include <vector>
#include <limits>
#include <assert.h>

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
	const int GetInd() const
	{
		return ind;
	}
	const size_t size() const
	{
		return dimensions;
	}
};

template<typename ValType1, typename ValType2>
inline double EuclideanDistance(const ValType1& p1, const ValType2& p2)
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
inline double EuclideanDistance(const std::array<ty, dims>& p1, const DataType<ty, dims>& p2)
{
	double ret = 0;
	for (int i = 0; i < dims; ++i)
	{
		ret += std::pow(p1[i] - p2[i], 2);
	}
	return std::sqrt(ret);
}

template<typename ty, int dims>
inline double EuclideanDistance(const DataType<ty, dims>& p1, const std::array<ty, dims>& p2)
{
	return EuclideanDistance(p2, p1);
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
		return QueryNearestNode(root, item, std::numeric_limits<double>::max());
	}

	std::string GenerateMatlabScript(std::array<double, 2> x_range, std::array<double, 2> y_range) const
	{
		std::string ret = "figure; hold on; axis equal;\n";
		std::sort(x_range.begin(), x_range.end());
		std::sort(y_range.begin(), y_range.end());
		GenerateMatlabScript_recu(root, x_range, y_range, ret);
		ret += "hold off;\n";
		return ret;
	}

private:
	int TreeHeight = 0;

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

	NodeType* BuildKdTree(typename ValType data[], int size, NodeType* parent = nullptr, int depth = 0)
	{
		TreeHeight = std::max(TreeHeight, depth);
		//利用ChooseSplitDim排序并分割
		if (size <= 0)
		{
			return nullptr;
		}
		else
		{
			int split = ChooseSplitDim(data, size); //此后已排好序，分割数据
			if (size == 1 && parent && split == parent->split_dim)
				split = (split + 1) % ValType::dimensions;
			size_t mid_split_index = size / 2;

			auto new_node = new NodeType(parent, data[mid_split_index], split);
			new_node->children[0] = BuildKdTree(data, mid_split_index, new_node, depth + 1);
			new_node->children[1] = BuildKdTree(data + mid_split_index + 1, size - mid_split_index - 1, new_node, depth + 1);

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

	std::pair<NodeType*, double> QueryNearestNode(NodeType* tree_root, const typename ValType::data_type& value, double minDistParent) const
	{
		if (!tree_root)
			return std::make_pair(nullptr, -1);

		std::stack<NodeType*> path;
		NodeType* nearest = tree_root;
		while (nearest)
		{
			path.push(nearest);
			if (value[nearest->split_dim] < nearest->val[nearest->split_dim])
				nearest = nearest->children[0];
			else
				nearest = nearest->children[1];
		}

		//由根开始到叶的path, 开始回溯
		nearest = path.top();
		double minDistNow = EuclideanDistance(value, nearest->val);

		while (!path.empty())
		{
			auto current = path.top();
			path.pop();

			double currentDistNow = EuclideanDistance(value, current->val);
			if (minDistNow >= currentDistNow)
			{
				minDistNow = currentDistNow;
				nearest = current;
			}

			double DistToSplitFace = value[current->split_dim] - current->val[current->split_dim];
			double CurrentRealMin = std::min(minDistNow, minDistParent);
			if (CurrentRealMin > std::abs(DistToSplitFace))
			{
				std::pair<NodeType*, double> ret;
				ret.second = -1;
				if (DistToSplitFace >= 0 && current->children[0] != nullptr)
				{
					ret = QueryNearestNode(current->children[0], value, CurrentRealMin);
				}
				else if (current->children[1] != nullptr)
				{
					ret = QueryNearestNode(current->children[1], value, CurrentRealMin);
				}
				if (ret.second >= 0 && minDistNow >= ret.second)
				{
					minDistNow = ret.second;
					nearest = ret.first;
				}
			}
		}

		return std::make_pair(nearest, minDistNow);
	}

	void GenerateMatlabScript_recu(NodeType* node, std::array<double, 2> x_range, std::array<double, 2> y_range, std::string& StringToAppend, int depth = 0) const
	{
		static_assert(ValType::dimensions == 2, "only support 2d.");

		if (!node)
			return;

		std::string LineColor = ",'Color',[" + std::to_string((double)depth / TreeHeight) + ", 0.3," + std::to_string(1 - (double)depth / TreeHeight) + "]";
		//画点
		StringToAppend += "scatter(" + std::to_string(node->val[0]) + "," + std::to_string(node->val[1]) + ",'ro');\n";
		//添加文字描述
		StringToAppend += "text(" + std::to_string(node->val[0] + 5) + "," + std::to_string(node->val[1]) + ",'" +
			std::to_string(node->val.GetInd()) + "_" + std::to_string(depth) +
			"');\n";
		//画分割线
		if (node->split_dim == 0)
		{
			StringToAppend += "line([" +
				std::to_string(node->val[0]) + "," +
				std::to_string(node->val[0]) +
				"],[" +
				std::to_string(y_range[0]) + "," +
				std::to_string(y_range[1]) +
				"]" + LineColor + ");\n";
			//递归左右子树
			GenerateMatlabScript_recu(node->children[0], { x_range[0], node->val[0] }, y_range, StringToAppend, depth + 1);
			GenerateMatlabScript_recu(node->children[1], { node->val[0], x_range[1] }, y_range, StringToAppend, depth + 1);
		}
		else
		{
			StringToAppend += "line([" +
				std::to_string(x_range[0]) + "," +
				std::to_string(x_range[1]) +
				"],[" +
				std::to_string(node->val[1]) + "," +
				std::to_string(node->val[1]) +
				"]" + LineColor + ");\n";
			//递归左右子树
			GenerateMatlabScript_recu(node->children[0], x_range, { y_range[0], node->val[1] }, StringToAppend, depth + 1);
			GenerateMatlabScript_recu(node->children[1], x_range, { node->val[1], y_range[1] }, StringToAppend, depth + 1);
		}
	}
};