#pragma once
#include <array>
#include <memory>

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

private:
	NodeType * BuildKdTree(ValType data[])
	{

	}
	void ReleaseKdTree()
	{

	}
};