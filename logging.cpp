#include "logging.h"
#include "my_timestamp.h"
#include "create_read_message.h"

#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

extern mutex m;
extern ostream *mylog_sayhello;
extern ostream *mylog_lsupdate;
extern ostream *mylog_ucastapp;

extern string host;
extern string port;
extern string max_ttl;

void log_receive_hello(const vector<string>& hello_message_params){
    m.lock();
    *mylog_sayhello << "[" << get_timestamp_now() << "] r SAYHELLO " << hello_message_params[0] << " " << "1" << " " << "-" << " " << "0" << endl;
    m.unlock();
}

void log_write_hello(string hello_message, shared_ptr<Connection> conn_ptr){
    m.lock();
    *mylog_sayhello << "[" << get_timestamp_now() << "] i SAYHELLO " << conn_ptr->neighbors_id << " " << "1" << " " << "-" << " " << "0" << endl;
    m.unlock();
}

void log_receive_lsupdate(map<string,string> info, string body, shared_ptr<Connection> conn_ptr){
    m.lock();
    string flood = "F";
    if(info["Flood"] == "0") flood = "-";
    *mylog_lsupdate << "[" << get_timestamp_now() << "] r LSUPDATE " << conn_ptr->neighbors_id << " " << info["TTL"] << " " << flood
    << " " << info["Content-Length"] << " " << info["MessageID"] << " " << info["OriginStartTime"] << " " << info["From"]
    << " (" << body << ")" << endl;
    m.unlock();
}

void log_write_lsupdate(string message, int flood, shared_ptr<Connection> conn_ptr){
    stringstream ss(message);
    string line;
    int cnt = 0;
    map<string,string> info;
    string body;
    while(getline(ss,line,'\n')){
        cnt++;
        if(cnt==1) continue;
        if(line.length()==1 && line[0]=='\r'){ 
            getline(ss,body);
            break;
        }
        stringstream ss1(line);
        string key, value;
        ss1>>key>>value;
        key = key.substr(0,key.length()-1);
        info[key] = value;  
    }
    m.lock();
    if(flood == 1){
        *mylog_lsupdate << "[" << get_timestamp_now() << "] d LSUPDATE " << conn_ptr->neighbors_id << " " << info["TTL"] << " " << "F"
        << " " << info["Content-Length"] << " " << info["MessageID"] << " " << info["OriginStartTime"] << " " << info["From"]
        << " (" << body << ")" << endl;
    }
    else{
        *mylog_lsupdate << "[" << get_timestamp_now() << "] i LSUPDATE " << conn_ptr->neighbors_id << " " << info["TTL"] << " " << "F"
        << " " << info["Content-Length"] << " " << info["MessageID"] << " " << info["OriginStartTime"] << " " << info["From"]
        << " (" << body << ")" << endl;
    }
    m.unlock();
}

void log_receive_ucastapp(map<string,string> ucastapp_header, string ucastapp_body, shared_ptr<Connection> conn_ptr){
    m.lock();
    int cnt = 0;
    for(int i=0;i<ucastapp_body.length();++i){
        if(ucastapp_body[i] == '\n') cnt++;
    }
    if(cnt<=1)
    {
        *mylog_ucastapp << "[" << get_timestamp_now() << "] r UCASTAPP " << conn_ptr->neighbors_id << " " << ucastapp_header["TTL"] << " " << "-"
        << " " << ucastapp_header["Content-Length"] << " " << ucastapp_header["MessageID"] << " " << ucastapp_header["From"] << " " << ucastapp_header["To"] << " "
        << ucastapp_header["Next-Layer"] << " " << ucastapp_body << endl;
    }
    else{
        map<string,string> content = parse_rdtdata_body(ucastapp_body);
        if(content["RDT-App-Number"] == "0")
            content["RDT-App-Number"] = "-";
        if(content["Content"] == "\n")
            content["Content"] = "\\n";
        *mylog_ucastapp << "[" << get_timestamp_now() << "] r UCASTAPP " << conn_ptr->neighbors_id << " " << ucastapp_header["TTL"] << " " << "-"
        << " " << ucastapp_header["Content-Length"] << " " << ucastapp_header["MessageID"] << " " << ucastapp_header["From"] << " " << ucastapp_header["To"] << " "
        << "2" << " " << content["Seq-Number"] << " " << content["RDT-App-Number"] << " " << content["RDT-Content-Length"] << " " << content["Content"] << endl;
    }
    m.unlock();
}

void log_write_ucastapp(string message, int forward, shared_ptr<Connection> conn_ptr){
    stringstream ss(message);
    string line;
    int cnt = 0;
    map<string,string> info;
    string body;
    while(getline(ss,line,'\n')){
        cnt++;
        if(cnt==1) continue;
        if(line.length()==1 && line[0]=='\r'){ 
            getline(ss,body);
            break;
        }
        stringstream ss1(line);
        string key, value;
        ss1>>key>>value;
        key = key.substr(0,key.length()-1);
        info[key] = value;  
    }
    m.lock();
    if(forward == 1){
        *mylog_ucastapp << "[" << get_timestamp_now() << "] f UCASTAPP " << conn_ptr->neighbors_id << " " << info["TTL"] << " " << "-"
        << " " << info["Content-Length"] << " " << info["MessageID"] << " " << info["From"] << " " << info["To"] << " "
        << info["Next-Layer"] << " " << body << endl;
    }
    else{
        *mylog_ucastapp << "[" << get_timestamp_now() << "] i UCASTAPP " << conn_ptr->neighbors_id << " " << info["TTL"] << " " << "-"
        << " " << info["Content-Length"] << " " << info["MessageID"] << " " << info["From"] << " " << info["To"] << " "
        << info["Next-Layer"] << " " << body << endl;
    }
    m.unlock();
}

void log_write_rdtdata(string message, int forward, shared_ptr<Connection> conn_ptr){
    stringstream ss(message);
    string line;
    int cnt = 0;
    map<string,string> info;
    string body;
    while(getline(ss,line,'\n')){
        cnt++;
        if(cnt==1) continue;
        if(line.length()==1 && line[0]=='\r'){ 
            getline(ss,body,'\0');
            break;
        }
        stringstream ss1(line);
        string key, value;
        ss1>>key>>value;
        key = key.substr(0,key.length()-1);
        info[key] = value;  
    }
    m.lock();
    map<string,string> content = parse_rdtdata_body(body);
    if(content["RDT-App-Number"] == "0")
        content["RDT-App-Number"] = "-";
    if(content["Content"] == "\n")
        content["Content"] = "\\n";
    //cerr << "Content is: " << content["Content"] << endl;
    if(forward == 1){
        *mylog_ucastapp << "[" << get_timestamp_now() << "] f UCASTAPP " << conn_ptr->neighbors_id << " " << info["TTL"] << " " << "-"
        << " " << info["Content-Length"] << " " << info["MessageID"] << " " << info["From"] << " " << info["To"] << " "
        << "2" << " " << content["Seq-Number"] << " " << content["RDT-App-Number"] << " " << content["RDT-Content-Length"] << " " << content["Content"] << endl;
    }
    else{
       *mylog_ucastapp << "[" << get_timestamp_now() << "] i UCASTAPP " << conn_ptr->neighbors_id << " " << info["TTL"] << " " << "-"
        << " " << info["Content-Length"] << " " << info["MessageID"] << " " << info["From"] << " " << info["To"] << " "
        << "2" << " " << content["Seq-Number"] << " " << content["RDT-App-Number"] << " " << content["RDT-Content-Length"] << " " << content["Content"] << endl;
    }
    m.unlock();


}


