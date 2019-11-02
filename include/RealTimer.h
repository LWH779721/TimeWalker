#ifndef __REALTIMETIMER_HPP__
#define __REALTIMETIMER_HPP__

#include "Timer.h"

namespace RightTimer {

/**
 * Real Time Timer
 **/
class RealTimer:
	public Timer {
	friend class TimerDetector;
public:
	RealTimer(string name);
	
	bool Init();
	
	bool Start(bool absOrRelative, unsigned int delaySec, unsigned int delayNsec, unsigned int intervalSec, unsigned int intervalNsec);
	
	bool Stop();
	
	bool Restart();
	
	virtual void Run() = 0;
	
	~RealTimer();
	
private:
	
};
};
	
#endif