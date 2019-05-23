#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>

#include "TimerManage.hpp"

using namespace RightTimer;
using namespace std;

extern int errno;

TimerManage* TimerManage::tm = NULL;

TimerManage::TimerManage()
{
}

TimerManage* TimerManage::GetTimerManager()
{
	if (NULL == tm)
	{
		tm = new TimerManage();
	}
	
	return tm;
}

int TimerManage::Start()
{
	epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd == -1)
    {
        std::cout << "err when epoll create" << std::endl;
        return -1;
    }
	
	std::thread t(&TimerManage::ManageLoop, this); 
	t.detach();
	
	return 0;
}

int TimerManage::AddTimer(bool abs, int delay, int interval, timer_callback cb)
{
	struct epoll_event ev;
	int ret = -1;
	Timer *t = new Timer;
	
	t->Init(abs, delay, interval, cb);
	std::cout << "AddTimer:" << t->GetFd() << std::endl;
	ev.data.fd = t->GetFd();
    ev.events = EPOLLIN|EPOLLET;//监听读状态同时设置ET模式
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, t->GetFd(), &ev);//注册epoll事件
	if (ret == -1)
	{
		std::cout << "epoll add client err" << std::endl;
		return -1;
	}
	
	timers.insert(make_pair(t->GetFd(), t));
	
	return 0;
}

int TimerManage::RemoveTimer(map<int, Timer *>::iterator it)
{
	struct epoll_event ev;
	int ret = -1;
	
	std::cout << "RemoveTimer:" << it->first << std::endl;
	ev.data.fd = it->first;                    
	//ev.events = EPOLLIN | EPOLLET;     
    ret = epoll_ctl(epfd, EPOLL_CTL_DEL, it->first, &ev);
	if (ret == -1)
	{
		std::cout << "epoll del client err" << std::endl;
		return -1;
	}
	
	timers.erase(it);
	
	return 0;
}

void TimerManage::ManageLoop()
{	
	int nfds, i, fd;
	struct epoll_event events[10];
	uint64_t howmany;
	map <int, Timer *>::iterator it;
					
	while (1)
    {
        nfds = epoll_wait(epfd, events, 10, -1);
        if (nfds == -1)
        {
			if (errno == EINTR)
				continue;
			else
				std::cout << "failed when epoll wait" << std::endl;
				continue;
        }
                
        for (i = 0; i < nfds; i++)
        {
            if (events[i].events & EPOLLIN) 
            {                            
				fd = events[i].data.fd;
								
				if (read(fd, &howmany, sizeof(howmany)) != sizeof(howmany))
				{
					std::cout << "read error" << std::endl;
					continue;
				}
				
				it = timers.find(fd);
				if (it == timers.end())
				{
					std::cout << "can't found timer" << std::endl;
					continue;
				}

				it->second->RunCallback();
				if (it->second->GetRepeat() == false)
				{
					RemoveTimer(it);
					delete it->second;
				}
            }          
        }
    }
}

int TimerManage::Dump()
{
	std::cout << "Timers:" << timers.size() << std::endl;
	
	return 0;
}