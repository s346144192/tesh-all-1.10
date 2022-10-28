#pragma once
#include <thread>
#include <condition_variable>
#include <atomic>
#include <windows.h>

typedef void(__cdecl* TimerFunc)(uint32_t, uint32_t);


class Timer {
private: 
	static LARGE_INTEGER freq;//本机CPU时钟频率
	LARGE_INTEGER startCount;//记录开始时间
	LARGE_INTEGER endCount;//记录结束时间

	TimerFunc _callback;	// 执行函数
	std::thread* _thread;
	std::mutex _mutex;
	std::condition_variable _condition;
	std::atomic_bool _pause;
	std::atomic_bool _stop;
	std::atomic_bool _update;
	std::atomic_uint_fast32_t loopA;
	std::atomic_uint_fast32_t loopB;
	size_t _interval;	// 间隔时间毫秒

	std::atomic_uint_fast32_t args1, args2;


	void RunFunc();
	void Runing();
public:
	Timer();
	~Timer();
	void Start(size_t interval,uint32_t runCount, TimerFunc Func, uint32_t a1, uint32_t a2,bool start_pause=false);
	void Update();
	void End();
	void Pause();
	void Resume();
};
