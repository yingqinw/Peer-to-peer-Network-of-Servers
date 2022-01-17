#ifndef _CREATE_READ_MESSAGE_H_
#define _CREATE_READ_MESSAGE_H_

#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include "connection.h"
#include "timer.h"

using namespace std;

// SAYHELLO
string create_hello_message();
vector<string> read_hello_message(shared_ptr<Connection> conn_ptr);

// LSUPDATE
int read_header_and_get_type(shared_ptr<Connection> conn_ptr);
string create_lsupdate_message(string ttl,string flood, string MessageID, string From, string OriginStartTime, string content_length, string body);
string read_lsupdate_message_body(int bytes, int fd);
map<string,string> read_lsupdate_message_header(shared_ptr<Connection> conn_ptr);
void create_and_flood_lsupdate(shared_ptr<Connection> conn_ptr);

// UCASTAPP
string create_ucastapp_message(map<string,string> ucastapp_header, string ucastapp_body);
string create_ucastapp_message(const string& to, const string& message, string& message_id, int ttl);
map<string,string> read_ucastapp_header(shared_ptr<Connection> conn_ptr);
string read_ucastapp_body(int bytes, int fd);

string create_rdtdata_body(char c, string seq_no, string app_no);
string create_rdtack_body(string seq_no, string app_no);
bool isACK(shared_ptr<Timer> t);
map<string,string> parse_rdtdata_body(string s);


#endif 