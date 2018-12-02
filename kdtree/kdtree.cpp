#include <ctime>
#include <vector>
#include "time_utility.h"
#include "kdtree.h"
#include <flann/flann.hpp>
using namespace flann;

constexpr int data_size = 6000;
constexpr int query_size = 1000;
constexpr int nn = 2;

using ValType = DataType<double, nn>;
using ValMemType = ValType::data_type;

std::pair<std::vector<ValMemType>, std::vector<ValMemType>> GenerateData()
{
	srand(std::time(nullptr));
	std::vector<ValMemType> test_data(data_size);
	for (int i = 0; i < test_data.size(); ++i)
	{
		for (int dim = 0; dim < nn; ++dim)
		{
			test_data[i][dim] = rand() % 1000;
		}
	}

	std::vector<ValMemType> query_data(query_size);
	for (int i = 0; i < query_data.size(); ++i)
	{
		for (int dim = 0; dim < nn; ++dim)
		{
			query_data[i][dim] = rand() % 1000;
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
	Index<L2<float> > index(dataset, flann::KDTreeIndexParams(4));
	index.buildIndex();
	timer.EndTimer("FLANN BUILD TIME:");

	timer.StartTimer();
	index.knnSearch(query, indices, dists, 1, flann::SearchParams());
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
		if (EuclideanDistance(ret[i], ret_ref[i]) > 1e-6)
		{
			std::cout << i + 1 << "-th query didn't match." << std::endl;
			for (int j = 0; j < nn; ++j)
				std::cout << ret[i][j] << "\t" << ret_ref[i][j] << "\t" << query_data[i][j] << std::endl;
			root.Query(query_data[i]);
		}
	}
	timer.EndTimer("KDTREE QUERY RESULT CHECK: ");

	return 0;
}
