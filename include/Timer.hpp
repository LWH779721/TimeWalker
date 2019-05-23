#ifndef __TIMER_HPP__
#define __TIMER_HPP__

namespace RightTimer {
	class Timer;
	typedef int (*timer_callback)(Timer &timer);
	
	class Timer {
		private:
			int tfd;
			bool repeat;
			bool abs;
			timer_callback cb;
			int SetUpAbsTimer(int delay);
			int SetUpRelativeTimer(int delay, int interval);
		public:
			Timer();
			int Init(bool abs, int delay, int interval, timer_callback cb);
			int RunCallback();
			int GetFd();
			bool GetRepeat();
			~Timer();
	};
};
	
#endif