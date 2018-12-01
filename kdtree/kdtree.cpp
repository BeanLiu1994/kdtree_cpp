#include "kdtree.h"
#include <ctime>
#include <vector>
#include "time_utility.h"

int main()
{
	using ValType = DataType<double, 3>;
	srand(std::time(nullptr));
	std::vector<ValType> test_data(1000000);
	for (int i = 0; i < test_data.size(); ++i)
	{
		for (int dim = 0; dim < ValType::dimensions; ++dim)
		{
			test_data[i][dim] = rand() % 1000;
		}
	}
	Timer<> timer;
	KdTree<ValType> root(test_data.data(), test_data.size());
	timer.EndTimer("TIME FOR KDTREE BUILDING: ");

	return 0;
}