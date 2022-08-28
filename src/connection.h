#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <queue>
#include <cstring>
#include <condition_variable>
#include <algorithm>

using namespace std;


class Connection{
public:
        int conn_number; /* -1 means that the connection is not initialized properly */
        int socket_fd; /* -1 means closed by connection-handling thread, -2 means close by console thread and connection may still be active */
        int kb_sent; /* number of bytes of response body written into the socket */
        int file_size; /* size of the response body in bytes, i.e., Content-Length */
        int orig_socket_fd;
        int self_id;
        string neighbors_id;

        bool is_duplicate = false;

        shared_ptr<thread> read_thread_ptr; /* shared pointer to a socket-reading thread */
        shared_ptr<thread> write_thread_ptr; /* shared pointer to a socket-writing thread */
       

        /* the next 3 objects are for the socket-reading thread to send work to the corresponding socket-writing thread */
        shared_ptr<mutex> m2; /* this is a "2nd-level" mutex */ 
        shared_ptr<condition_variable> cv2;
        queue<shared_ptr<string>> q2;

        Connection() : conn_number(-1), socket_fd(-1), neighbors_id("-1"),read_thread_ptr(NULL), write_thread_ptr(NULL), m2(NULL), cv2(NULL)  { }
        Connection(int c, int s, shared_ptr<thread> tr, shared_ptr<thread> tw) {
            conn_number = c;
            socket_fd = s;
            read_thread_ptr = tr;
            write_thread_ptr = tw;
            kb_sent = file_size = 0;
            m2 = make_shared<mutex>();
            cv2 = make_shared<condition_variable>();
            neighbors_id = "-1";
        }

        void add_work(shared_ptr<string> msg) { 
              m2->lock();
              q2.push(msg);
              cv2->notify_all();
              m2->unlock();
        }
        shared_ptr<string> wait_for_work() { 
            unique_lock<mutex> l(*m2);

            while (q2.empty()) {
                cv2->wait(l);
            }
            shared_ptr<string> k = q2.front();
            q2.pop();
            return k;
        }
};


#endif
