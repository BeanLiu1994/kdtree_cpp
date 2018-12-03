#pragma once
#include <chrono>
#include <iostream>
#include <string>
//////////////////////////////////////////////////
// 获取时间间隔并返回(秒)
//    _printThis 为空: 不打印任何文字
//             不为空: 打印输入的字符串及使用时间
// 例如：
//    Timer<> a;
//    a.EndTimer("Solver Timer");//打印"Solver Timer elapsed time:  $deltaTime$ s."
//=======================
//    Timer<> a;
//    a.EndTimer(str, true);// 记录时间后重置计时器
//

template<typename _ty = std::chrono::microseconds>
class Timer
{
public:
	Timer(bool startNow = true)
	{
		if (startNow) StartTimer();
	}
	void StartTimer(const std::string& _printThis = std::string())
	{
		if (!_printThis.empty()) std::cout << _printThis << std::endl;
		start = std::chrono::system_clock::now();
	}
	double EndTimer(const std::string& _printThis, bool restartFlag)
	{
		auto seconds = EndTimer(_printThis);
		if (restartFlag)
			StartTimer();
		return seconds;
	}
	double EndTimer(const std::string& _printThis = std::string())
	{
		end = std::chrono::system_clock::now();
		duration = std::chrono::duration_cast<_ty>(end - start);
		seconds = (double(duration.count()) * _ty::period::num / _ty::period::den);
		std::string s = " elapsed time:  " + std::to_string(seconds) + "s";
		if (!_printThis.empty()) std::cout << _printThis << s << std::endl;
		//else std::cout << s << std::endl;
		return seconds;
	}
private:
	std::chrono::time_point<std::chrono::system_clock> start, end;
	_ty duration;
	double seconds;
};