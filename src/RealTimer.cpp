#include <iostream>
#include <unistd.h>
#include <sys/timerfd.h>

#include "RealTimer.h"

using namespace std;

namespace RightTimer {
	
RealTimer::RealTimer(string name):
	Timer(name){
}

bool RealTimer::Init(){
	m_timerfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
	if (m_timerfd < 0){
		perror("timerfd_create failed");
		return false;
	}
	
	return true;
}

bool RealTimer::Start(bool absOrRelative, unsigned int delaySec, unsigned int delayNsec, unsigned int intervalSec, unsigned int intervalNsec){
	struct itimerspec new_value = {0};
	
	m_absOrRelative = absOrRelative;
	m_delaySec = delaySec;
	m_delayNsec = delayNsec;
	m_intervalSec = intervalSec;
	m_intervalNsec = intervalNsec;
	
	new_value.it_value.tv_sec = m_delaySec;
	new_value.it_value.tv_nsec = m_delayNsec;
	new_value.it_interval.tv_sec = m_intervalSec;
	new_value.it_interval.tv_nsec = m_intervalNsec;
	
	if (timerfd_settime(m_timerfd, m_absOrRelative?TFD_TIMER_ABSTIME:0, &new_value, NULL) != 0){
		perror("timerfd_settime failed");
		return false;
	}	
	
	return true;
}

bool RealTimer::Stop(){
	struct itimerspec new_value = {0};
	
	if (timerfd_settime(m_timerfd, 0, &new_value, NULL) != 0){
		perror("timerfd_settime failed");
		return false;
	}	
		
	return true;
}

bool RealTimer::Restart(){
	struct itimerspec new_value = {0};
	
	new_value.it_value.tv_sec = m_delaySec;
	new_value.it_value.tv_nsec = m_delayNsec;
	new_value.it_interval.tv_sec = m_intervalSec;
	new_value.it_interval.tv_nsec = m_intervalNsec;
	
	if (timerfd_settime(m_timerfd, m_absOrRelative?TFD_TIMER_ABSTIME:0, &new_value, NULL) != 0){
		perror("timerfd_settime failed");
		return false;
	}
}

RealTimer::~RealTimer(){
	close(m_timerfd);
}
}
