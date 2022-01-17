#include "timer.h"
#include <iostream>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <string>
#include <sstream>

using namespace std;

Timer::Timer(struct timeval* t, std::string target, std::string message_id, string session_id, int cnt, int seq_no){
	this->start_time = t;
	this->target = target;
	this->message_id = message_id;
	this->session_id = session_id;
	this->cnt = cnt;
    this->seq_no = seq_no;
	m3 = make_shared<mutex>();
    cv3 = make_shared<condition_variable>();
}

void Timer::add_work(map<string,string> header, string body) { 
    m3->lock();
    this->header = header;
    this->body = body;
    cv3->notify_all();
    m3->unlock();
}

void Timer::wait_for_work(){ 
	unique_lock<mutex> l3(*m3);

    while (this->status == 0) {
        cv3->wait(l3);
    }


}