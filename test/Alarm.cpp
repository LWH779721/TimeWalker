#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "Alarm.h"

using namespace TimeWalker;
using namespace std;

Alarm::Alarm(string name){
    m_schedule = std::make_shared<Schedule>(name);
	
	m_schedule->setCallback(bind(&Alarm::Run, this));
    m_schedule->at("2020-12-03 18:14:30");
}

void Alarm::Run(){
	struct timeval tv = {0};

	gettimeofday(&tv, NULL);

	cout << "time arvices " << tv.tv_sec <<	endl;
}

Alarm::~Alarm(){
}
	
int main(int argc, char **args){
	Alarm a("alarm");
	
	while (1){
		sleep(1);
	}
	
	return 0;
}
