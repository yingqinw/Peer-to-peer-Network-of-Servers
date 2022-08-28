#ifndef _TIMER_H_
#define _TIMER_H_

#include "my_timestamp.h"
#include <string>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <thread>
#include <map>

using namespace std;

class Timer{
public:
	struct timeval* start_time;
	std::string target;
	std::string message_id;
	string session_id;

	int status = 0; // 0 - original, 1 - expired, 2 - received PONG, 3 - received TTLZERO
	int cnt = 0;
	int seq_no;
	map<string,string> header;
	string body;

	shared_ptr<thread> timer_thread;
	shared_ptr<mutex> m3;
	shared_ptr<condition_variable> cv3;

	Timer(struct timeval* t, std::string target, std::string message_id, string session_id, int cnt, int seq_no);

 	void add_work(map<string,string> header, string body);

    void wait_for_work();

};

#endif