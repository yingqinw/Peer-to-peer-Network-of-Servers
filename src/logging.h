#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <string>
#include <memory>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <map>

#include "connection.h"

using namespace std;

void log_receive_hello(const vector<string>& hello_message_params);
void log_write_hello(string hello_message, shared_ptr<Connection> conn_ptr);

void log_receive_lsupdate(map<string,string> info, string body, shared_ptr<Connection> conn_ptr);
void log_write_lsupdate(string message, int flood, shared_ptr<Connection> conn_ptr);

void log_receive_ucastapp(map<string,string> ucastapp_header, string ucastapp_body, shared_ptr<Connection> conn_ptr);
void log_write_ucastapp(string message, int forward, shared_ptr<Connection> conn_ptr);

void log_write_rdtdata(string message, int forward, shared_ptr<Connection> conn_ptr);


#endif
