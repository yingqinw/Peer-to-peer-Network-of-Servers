#include <iostream>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <queue>

#include "reaper.h"

using namespace std;


extern mutex m;
extern condition_variable cv;
extern queue<shared_ptr<Connection>> q;
extern vector<shared_ptr<Connection>> connection_list;

void reaper_add_work(shared_ptr<Connection> c) {
  m.lock();
  q.push(c);
  cv.notify_all();
  m.unlock();
}

shared_ptr<Connection> reaper_wait_for_work() {
  unique_lock<mutex> l(m);

  while (q.empty()) {
    cv.wait(l);
  }
  shared_ptr<Connection> k = q.front();
  q.pop();

  return k;
}

void reaper(int* master_socket_fd){
    for(;;){
        shared_ptr<Connection> c = reaper_wait_for_work();
        
        if(c==nullptr){
            break;
        }
        c->read_thread_ptr->join();
        m.lock();
        //*mylog << "[" << get_timestamp_now() << "] [" << c->conn_number << "]\tReaper has joined with socket-reading thread" << endl;
        for(int i=0;i<connection_list.size();++i){
            if(connection_list[i]->conn_number == c->conn_number){
                connection_list.erase(connection_list.begin()+i);
                break;
            }
        }
        m.unlock();
    }
    for(;;){
        m.lock();
        if(connection_list.size()==0){
            m.unlock();
            break;
        }
        shared_ptr<Connection> c= connection_list.back();
        m.unlock();
        c->read_thread_ptr->join();
        m.lock();
        //*mylog << "[" << get_timestamp_now() << "] [" << c->conn_number << "]\tReaper has joined with socket-reading thread" << endl; 
        connection_list.pop_back();
        m.unlock();
    }
}