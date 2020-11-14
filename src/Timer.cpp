#include <iostream>
#include <unistd.h>
#include <sys/timerfd.h>

#include "TimerDetector.h"
#include "Timer.h"

using namespace std;

namespace TimeWalker {
	
Timer::Timer(string name, bool absOrRelative, unsigned int delaySec, unsigned int delayNsec, unsigned int intervalSec, unsigned int intervalNsec, function<void()> callback):
	TimeWalker(name, TimeType::MONOTONIC, absOrRelative, delaySec, delayNsec, intervalSec, intervalNsec, callback){
}

bool Timer::Init(){
    if (!TimeWalker::Init()) {
        cout << "TimeWalker init failed" << endl;
        return false;
    }
	
    TimerDetector *tm = TimerDetector::GetDefaultDetector();
	tm->DetectTimer(shared_from_this());
	return true;
}

bool Timer::Init(std::shared_ptr<TimerDetector> timerDetector){
    if (!TimeWalker::Init()) {
        cout << "TimeWalker init failed" << endl;
        return false;
    }
	
    timerDetector->DetectTimer(shared_from_this());
	return true;        
}

bool Timer::Start(){
	struct itimerspec new_value = {0};
	
	if (m_callback == nullptr){
		cout << "m_callback is nullptr" << endl;
		return false;	
	}
    
    if (!m_inited && !Init()){
        cout << "Timer init failed" << endl;
		return false;
    }
    
	//Cause: Timer will stop when it_value is zero, No matter what the value of it_interval is
	if ((m_delaySec != 0)
		|| (m_delayNsec != 0)){
		new_value.it_value.tv_sec = m_delaySec;
		new_value.it_value.tv_nsec = m_delayNsec;
	} else {
		new_value.it_value.tv_sec = m_intervalSec;
		new_value.it_value.tv_nsec = m_intervalNsec;
	}
	
	new_value.it_interval.tv_sec = m_intervalSec;
	new_value.it_interval.tv_nsec = m_intervalNsec;
	
	if (timerfd_settime(m_id, m_absOrRelative?TFD_TIMER_ABSTIME:0, &new_value, NULL) != 0){
		perror("timerfd_settime failed");
		return false;
	}	
    
	m_actived = true;
	return true;
}

bool Timer::Start(bool absOrRelative, unsigned int delaySec, unsigned int delayNsec, unsigned int intervalSec, unsigned int intervalNsec, function<void()> callback){	
	if (m_actived){
        Stop();
	}
    
	m_absOrRelative = absOrRelative;
	m_delaySec = delaySec;
	m_delayNsec = delayNsec;
	m_intervalSec = intervalSec;
	m_intervalNsec = intervalNsec;
	m_callback = callback;
	
	return Start();
}

bool Timer::Stop(){
	if (!m_inited || !m_actived){
        return true;
    }
    
    struct itimerspec new_value = {0};
	
	if (timerfd_settime(m_id, 0, &new_value, NULL) != 0){
		perror("timerfd_settime failed");
		return false;
	}	
		
	return true;
}

Timer::~Timer(){

}
}