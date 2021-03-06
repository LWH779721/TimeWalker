#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/prctl.h>
#include <errno.h>

#include "TimeWalker.h"
#include "TimerDetector.h"

using namespace std;

extern int errno;

namespace TimeWalker {

TimerDetector* TimerDetector::defaultDetector = NULL;

TimerDetector::TimerDetector():
	m_epfd(-1){
}

TimerDetector* TimerDetector::GetDefaultDetector(){
	if (NULL == defaultDetector){
		defaultDetector = new TimerDetector();
		bool ret = defaultDetector->init();
		if (ret == false){
			delete defaultDetector;
			defaultDetector = NULL;
		}
	}
	
	return defaultDetector;
}

bool TimerDetector::init(){
	m_epfd = epoll_create1(EPOLL_CLOEXEC);
    if (m_epfd == -1){
		cout << "err when epoll create" << endl;
        return false;
    }
	
	m_thread = std::thread(&TimerDetector::ManageLoop, this);
	
	return true;
}

bool TimerDetector::DetectTimer(std::shared_ptr<TimeWalker> timer){
	struct epoll_event ev;
	int ret = -1;

	std::cout << "DetectTimer:" << timer->m_id << std::endl;
	m_timers.insert(make_pair(timer->m_id, timer));
	
	ev.data.fd = timer->m_id;
    ev.events = EPOLLIN|EPOLLET;
    ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, timer->m_id, &ev);
	if (ret == -1){
		std::cout << "epoll add client err" << std::endl;
		return false;
	}
	
	return true;
}

int TimerDetector::RemoveTimer(map<int, std::shared_ptr<TimeWalker>>::iterator it){
	struct epoll_event ev;
	int ret = -1;
	
	std::cout << "RemoveTimer:" << it->first << std::endl;
	ev.data.fd = it->first;                    
	//ev.events = EPOLLIN | EPOLLET;     
    ret = epoll_ctl(m_epfd, EPOLL_CTL_DEL, it->first, &ev);
	if (ret == -1){
		std::cout << "epoll del client err" << std::endl;
		return -1;
	}
	
	m_timers.erase(it);
	
	return 0;
}

bool TimerDetector::PauseTimer(int timerfd){
	map <int, std::shared_ptr<TimeWalker>>::iterator it;
	
	it = m_timers.find(timerfd);
	if (it == m_timers.end()){
		cout << "can't found timer " << timerfd << endl;
		return false;
	}
	
	return it->second->stop();
}

bool TimerDetector::StopTimer(int timerfd){
	struct epoll_event ev;
	int ret = -1;
	map <int, std::shared_ptr<TimeWalker>>::iterator it;
	
	it = m_timers.find(timerfd);
	if (it == m_timers.end()){
		cout << "can't found timer " << timerfd << endl;
		return false;
	}

	std::cout << "StopTimer:" << it->first << std::endl;
	ev.data.fd = it->first;    
    ret = epoll_ctl(m_epfd, EPOLL_CTL_DEL, it->first, &ev);
	if (ret == -1){
		cout << "epoll del client err" << endl;
		return false;
	}
	
	m_timers.erase(it);
	return true;
}

void TimerDetector::ManageLoop(){	
	int nfds, i, fd;
	struct epoll_event events[10];
	uint64_t howmany;
	map <int, std::shared_ptr<TimeWalker>>::iterator it;
	
	prctl(PR_SET_NAME, "TimerDetector");
	
	while (1){
        nfds = epoll_wait(m_epfd, events, 10, -1);
        if (nfds == -1){
			if (errno == EINTR)
				continue;
			else
				std::cout << "failed when epoll wait" << std::endl;
				continue;
        }
        
        for (i = 0; i < nfds; ++i){
            if (events[i].events & EPOLLIN){                            
				fd = events[i].data.fd;
								
				if (read(fd, &howmany, sizeof(howmany)) != sizeof(howmany)){
					std::cout << "read error" << std::endl;
					continue;
				}
				
				it = m_timers.find(fd);
				if (it == m_timers.end()){
					std::cout << "can't found timer " << fd << std::endl;
					continue;
				}
				
				cout << it->second->m_name << " Timer end" << endl;
				it->second->m_execTimes++;
				it->second->m_callback();
            }          
        }
    }
}

int TimerDetector::Dump(){
	map <int, std::shared_ptr<TimeWalker>>::iterator it;
	
	cout << "Timers:" << m_timers.size() << endl;
	it = m_timers.begin();
	while (it != m_timers.end()){
		cout << "Timer " << it->first << " Name " << it->second->m_name << endl;	
		++it;
	}
	
	return 0;
}

TimerDetector::~TimerDetector(){
	if (m_thread.joinable()){
		m_thread.join();	
	}
}

}
