#include <ctime>
#include <vector>
#include "time_utility.h"
#include "kdtree.h"

#define FLANN_USE_CUDA
#include <flann/flann.hpp>
using namespace flann;

constexpr int data_size = 50000;
constexpr int query_size = 100000;
constexpr int nn = 2;
constexpr int mod_n = 1000;

using ValType = DataType<double, nn>;
using ValMemType = ValType::data_type;

std::pair<std::vector<ValMemType>, std::vector<ValMemType>> GenerateData()
{
	static_assert(query_size > data_size, "data is treated as first part of query test.");
	//srand(std::time(nullptr));
	srand(1);
	std::vector<ValMemType> test_data(data_size);
	for (int i = 0; i < test_data.size(); ++i)
	{
		for (int dim = 0; dim < nn; ++dim)
		{
			test_data[i][dim] = rand() % mod_n;
		}
	}

	std::vector<ValMemType> query_data(test_data);
	for (int i = test_data.size(); i < query_size; ++i)
	{
		query_data.emplace_back();
		for (int dim = 0; dim < nn; ++dim)
		{
			query_data.back()[dim] = rand() % mod_n;
		}
	}
	return std::make_pair(test_data, query_data);
}

std::vector<ValType> mine_check(const std::vector<ValMemType>& test_data, const std::vector<ValMemType>& query_data)
{
	Timer<> timer;
	KdTree<ValType> root(test_data.data(), test_data.size());
	timer.EndTimer("TIME FOR KDTREE BUILDING: ");

	timer.StartTimer();
	std::vector<ValType> ret;
	for (int i = 0; i < query_data.size(); ++i)
	{
		auto test = root.Query(query_data[i]);
		ret.push_back(test.first->val);
	}
	timer.EndTimer("TIME FOR KDTREE QUERYING: ");

	return ret;
}


std::vector<ValType> run_flann(const std::vector<ValMemType>& test_data, const std::vector<ValMemType>& query_data)
{
	Matrix<float> dataset(new float[data_size*nn], data_size, nn);
	for (int i = 0; i < test_data.size(); ++i)
		for (int dim = 0; dim < ValType::dimensions; ++dim)
			dataset[i][dim] = test_data[i][dim];

	Matrix<float> query(new float[query_size*nn], query_size, nn);
	for (int i = 0; i < query_data.size(); ++i)
		for (int dim = 0; dim < ValType::dimensions; ++dim)
			query[i][dim] = query_data[i][dim];

	Matrix<int> indices(new int[query.rows], query.rows, 1);
	Matrix<float> dists(new float[query.rows], query.rows, 1);

	Timer<> timer;
	KDTreeCuda3dIndex<L2<float> > index(dataset, flann::KDTreeCuda3dIndexParams(1));
	index.buildIndex();
	timer.EndTimer("FLANN BUILD TIME:");

	timer.StartTimer();
	index.knnSearchGpu(query, indices, dists, 1, flann::SearchParams(-1));
	timer.EndTimer("FLANN QUERY TIME:");

	std::vector<ValType> ret;
	for (int i = 0; i < query_size; ++i)
	{
		ret.emplace_back(test_data.data(), indices[i][0]);
	}

	delete[] dataset.ptr();
	delete[] query.ptr();
	delete[] indices.ptr();
	delete[] dists.ptr();

	return ret;
}

int main()
{
	std::vector<ValMemType> test_data, query_data;
	std::tie(test_data, query_data) = GenerateData();

	std::cout << "data size: \t" << test_data.size() << std::endl;
	std::cout << "query size: \t" << query_data.size() << std::endl;

	std::vector<ValType> ret_ref = run_flann(test_data, query_data);
	std::vector<ValType> ret = mine_check(test_data, query_data);

	KdTree<ValType> root(test_data.data(), test_data.size());
	Timer<> timer;
	for (int i = 0; i < query_data.size(); ++i)
	{
		if (std::abs(EuclideanDistance(ret[i], query_data[i]) - EuclideanDistance(ret_ref[i], query_data[i])) > 1e-6)
		{
			//static std::string DrawScript = root.GenerateMatlabScript({ 0,mod_n }, { 0,mod_n });
			std::cout << i + 1 << "-th query didn't match." << std::endl;
			for (int j = 0; j < nn; ++j)
				std::cout << ret[i][j] << "\t" << ret_ref[i][j] << "\t" << query_data[i][j] << std::endl;
			root.Query(query_data[i]);
		}
	}
	timer.EndTimer("KDTREE QUERY RESULT CHECK: ");

	return 0;
}
