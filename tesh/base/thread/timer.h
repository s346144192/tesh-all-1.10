#pragma once
#include <thread>
#include <condition_variable>
#include <atomic>
#include <windows.h>

typedef void(__cdecl* TimerFunc)(uint32_t, uint32_t);


class Timer {
private: 
	static LARGE_INTEGER freq;//����CPUʱ��Ƶ��
	LARGE_INTEGER startCount;//��¼��ʼʱ��
	LARGE_INTEGER endCount;//��¼����ʱ��

	TimerFunc _callback;	// ִ�к���
	std::thread* _thread;
	std::mutex _mutex;
	std::condition_variable _condition;
	std::atomic_bool _pause;
	std::atomic_bool _stop;
	std::atomic_bool _update;
	std::atomic_uint_fast32_t loopA;
	std::atomic_uint_fast32_t loopB;
	size_t _interval;	// ���ʱ�����

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
