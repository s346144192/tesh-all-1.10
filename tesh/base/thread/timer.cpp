#include <chrono>
#include <base/thread/timer.h>

using namespace std;

LARGE_INTEGER Timer::freq;

Timer::Timer() {
	static BOOL success = QueryPerformanceFrequency(&freq);
	_callback = NULL;
	_thread = nullptr;
	_pause = false;
	_stop = false;
	loopA = 0;
	loopB = 0;
}

Timer::~Timer() {
	End();
}


void Timer::RunFunc() {
	if (!_callback) {
		return;
	}
	(*_callback)(args1.load(), args2.load());
}

void Timer::Runing() {
	int runCount = 0;
	double timeCount = 0.;
	double current_interval = 0.;
	while (!_stop) {
		std::unique_lock<std::mutex> lock(_mutex);
		current_interval = (double)_interval;
		QueryPerformanceCounter(&startCount);//开始计时
		do {
			while (_pause)
			{
				_condition.wait(lock); // Unlock _mutex and wait to be notified
			}
			if (_update) {
				QueryPerformanceCounter(&startCount);//开始计时
				_update = false;
			}
			QueryPerformanceCounter(&endCount);
			timeCount = ((double)endCount.QuadPart - (double)startCount.QuadPart)*1000 / (double)freq.QuadPart;
		} while (!_stop && timeCount < current_interval);
		if (!_stop && (loopA< loopB || loopB==0)) {
			loopA++;
			RunFunc();
		}
		
	}
	
}
void Timer::Start(size_t interval, uint32_t runCount, TimerFunc Func, uint32_t a1, uint32_t a2,bool start_pause) {
	End();
	_interval = interval;
	loopA = 0;
	loopB = runCount;
	_callback = Func;
	args1 = a1;
	args2 = a2;
	if (_thread == nullptr)
	{
		_pause = start_pause;
		_stop = false;
		_update = false;
		_thread = new thread(&Timer::Runing, this);
		
	}
}
void Timer::Update() {
	if (_thread != nullptr) {
		_update = true;
		_pause = false;
		_condition.notify_all();
	}
}
void Timer::End() {
	if (_thread != nullptr)
	{
		_update = false;
		_pause = false;
		_stop = true;

		_condition.notify_all(); 
		if (_thread->joinable()) {
			_thread->join();
		}
		delete _thread;
		_thread = nullptr;
	}

}
void Timer::Pause() {
	if (_thread != nullptr) {
		_update = false;
		_pause = true;
	}
}
void Timer::Resume() {
	if (_thread != nullptr) {
		_pause = false;
		_condition.notify_all();
	}

}