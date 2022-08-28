/* C++ standard include files first */
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <map>
#include <fstream>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <set>

/* C system include files next */
#include <arpa/inet.h>
#include <netdb.h>

/* C standard include files next */
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>

/* your own include last */
#include "my_socket.h"
#include "my_readwrite.h"
#include "my_timestamp.h"
#include "parse_ini_file.h"
#include "util.h"
#include "connection.h"
#include "logging.h"
#include "reaper.h"
#include "create_read_message.h"
#include "timer.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <openssl/md5.h>

using namespace std;

/* Define global variables */

mutex m;
condition_variable cv;
queue<shared_ptr<Connection>> q;

ostream *mylog_sayhello = NULL;
ostream *mylog_lsupdate = NULL;
ostream *mylog_ucastapp = NULL;
ofstream myfile;

string host;
string port;
string max_ttl;
static int sleeping_time;
static int next_conn_number = 0;
static int next_timer_number = 0;
int message_expiration_time = 0;

struct Message{
    string from;
    int seq_no;
    string content;
    int app_no;
    Message(string from, string content, int seq_no, int app_no){
        this->from = from;
        this->content = content;
        this->seq_no = seq_no;
        this->app_no = app_no;
    }
};

class EchoJob {
public:
    //int job_number; /* sequence number -- may not be useful */
    string peer_nodeid; /* which node you are running RDT-3.0 with */
    string app_no; /* 98 or 99 */
    string message; /* the line you are supposed to echo back */
    bool done = false;
    shared_ptr<thread> thread_ptr; /* the thread that's working on this job */
    EchoJob(string peer_nodeid, string app_no, string message, shared_ptr<thread> thread_ptr){
        this->peer_nodeid = peer_nodeid;
        this->app_no = app_no;
        this->message = message;
        this->thread_ptr = thread_ptr;
    }
};

vector<shared_ptr<Connection>> connection_list; // list of connections

vector<shared_ptr<Timer>> timer_list; // list of timers

set<string> received_message_ids; // Message cache

map<string,string> lastest_update; // not used temporarily

map<string,vector<string>> adj_list; // adjacency list for network topolpgy

map<string,string> forwarding_table; // every node's predecessor, updated upon running bfs

vector<Message> received_message;

vector<shared_ptr<EchoJob>> echo_jobs;


/* Helper functions */

shared_ptr<Connection> find_connection_by_number(int num);

vector<int> count_neighbors();

void bfs_update();

int update_adj_list(const string& from, string& lsupdate_body);

void create_and_flood_lsupdate(shared_ptr<Connection> conn_ptr);

set<string> get_neighbors_ids(const string& ini_filename);

void neighbors(int* master_socket_fd, const string& ini_filename);

int get_traceroute_type(string message);

/* Real Functions */

void print_rdtsend_timeout_message(string seq_no, string app_no){
    cout << "RDT sender timeout for Seq-Number: " << seq_no <<  " and RDT-App-Number: " <<  app_no << endl;
}

int find_receiver_for_rdtsend(string target, string app_no){
    for(int i=0;i<received_message.size();++i){
        if(received_message[i].from == target && to_string(received_message[i].app_no) == app_no){
            return i;
        }
    }
    return -1;
}

int find_receiver_for_echoapp(string target, string app_no){
    for(int i=0;i<echo_jobs.size();++i){
        if(echo_jobs[i]->peer_nodeid == target && echo_jobs[i]->app_no == app_no){
            return i;
        }
    }
    return -1;
}


// This function comes with lock acquired
void udt_send(string target, string body, bool is_udtsend, string& message_id, int ttl){
    if(target == port){
        if(is_udtsend) cout << "Cannot use udtsend command to send message to yourself." << endl;
        m.unlock();
        return;
    }
    vector<int> neighbors = count_neighbors();
    if(neighbors.size()==0){
        if(is_udtsend) cout << target << " is not reachable" << endl;
        m.unlock();
        return;
    }
    bfs_update();
    if(forwarding_table.find(target) == forwarding_table.end()){
        if(is_udtsend) cout << target << " is not reachable" << endl;
        m.unlock();
        return;
    }

    string ucastapp_message = create_ucastapp_message(target,body,message_id, ttl);
    string receiver = forwarding_table[target];
    for(int i=0;i<neighbors.size();++i){
        if(connection_list[neighbors[i]]->neighbors_id == receiver){
            connection_list[neighbors[i]]->add_work(make_shared<string>(ucastapp_message));
            break;
        }
    }
}

void timer(shared_ptr<Timer> timer){
    m.lock();
    m.unlock();
    while(timer->cnt>0){
        timer->cnt--;
        usleep(250000);
        if(timer->status != 0) {
            return;
        }
    }
    m.lock();
    timer->status = 1;
    timer->cv3->notify_all();
    m.unlock();
}


void traceroute(string target){
    int session_id = ++next_timer_number;
    for(int i=1;i<=to_int(max_ttl);++i){
        m.lock();
        bfs_update();
        
        struct timeval start_time;
        gettimeofday(&start_time, NULL);
        string message_id;
        string body = "353UDT/1.0 PING " + to_string(session_id);
        udt_send(target,body,false,message_id,i); // sending message out :D

        int cnt = message_expiration_time * 4;
        shared_ptr<Timer> t = make_shared<Timer>(Timer(&start_time,target,message_id,to_string(session_id),cnt, -1));
           
        shared_ptr<thread> timer_thread = make_shared<thread>(thread(timer,t));
        t->timer_thread = timer_thread;
        timer_list.emplace_back(t);
        m.unlock();
        t->wait_for_work();
        if(t->status == 1){ // expired
            cout << i << " - *" << endl;
        }else{ // either received PONG or TTLZERO
            struct timeval now;
            gettimeofday(&now, NULL);
            double elasped = timestamp_diff_in_seconds(&start_time, &now);
            cout << i << " - " << t->header["From"] << ", " << elasped << endl;
            if(t->status == 2)// PONG
            {
                cout << t->target << " is reached in " << i << " steps" << endl;
                timer_list.back()->timer_thread->join();
                return;
            } 
                
        }
        
        timer_list.back()->timer_thread->join();
        usleep(3000000); // sleep for 3 seconds
    }
    cout << "traceroute:" << target << " not reached after " <<  max_ttl << " steps" << endl;
}

void rdt_send(string target, string body, string app_no){
    if(body.length()==0 || body[body.length()-1]!='\n') body+="\n";
    int seq_no = 0;
    for(int i=0;i<body.length();++i){
        struct timeval start_time;
        gettimeofday(&start_time, NULL);
        string message_id;
        string rdt_pack = create_rdtdata_body(body[i], to_string(seq_no), app_no);
        udt_send(target,rdt_pack,false,message_id,to_int(max_ttl)); // sending message out :D

        int cnt = message_expiration_time * 4;
        shared_ptr<Timer> t = make_shared<Timer>(Timer(&start_time,target,message_id,"-1",cnt,seq_no));
           
        shared_ptr<thread> timer_thread = make_shared<thread>(thread(timer,t));
        t->timer_thread = timer_thread;
        timer_list.emplace_back(t);

        bool done_waiting = false;
        while(!done_waiting){
            t->wait_for_work();
            if(t->status == 1){ // expired
                print_rdtsend_timeout_message(to_string(seq_no), app_no); // app_no = 0 for now
                t->timer_thread->join();
                timer_list.pop_back();

                udt_send(target,rdt_pack,false,message_id,to_int(max_ttl)); // re-sending the message
                gettimeofday(&start_time, NULL);
                t = make_shared<Timer>(Timer(&start_time,target,message_id,"-1",cnt,seq_no));
           
                timer_thread = make_shared<thread>(thread(timer,t));
                t->timer_thread = timer_thread;
                timer_list.emplace_back(t);

            }else if(isACK(t)){ // receives acknowledgement
                if(t->seq_no == seq_no){
                    done_waiting = true;
                }
            }
        }
        t->timer_thread->join();
        struct timeval now;
        gettimeofday(&now, NULL);
        double time_elaseped = timestamp_diff_in_seconds(&start_time, &now);
        if(time_elaseped < 1.0){
            usleep(1000000 - time_elaseped * 1000000.0);
        }

        seq_no = !seq_no;
    }

    if(app_no == "0")
        cout << "rdtsend: '" << body.substr(0,body.length()-1) << "' to " << target << " have been sent and acknowledged" << endl;
}

string rdt_receive_a_line(string target, string app_no){
    for(;;){
        usleep(250000);
        m.lock();
        for(int i=0;i<echo_jobs.size();++i){
            if(echo_jobs[i]->peer_nodeid == target && echo_jobs[i]->app_no == app_no && echo_jobs[i]->done == true){
                string to_return = echo_jobs[i]->message;
                echo_jobs.erase(echo_jobs.begin()+i);
                m.unlock();
                return to_return;
            }
        }
        m.unlock();
    }

    return "-1"; // should never gets here
}

void echoapp_client(string target){
    for(;;){
        cout << "echoapp>";
        cout.flush();
        string message;
        getline(cin,message);
        if(message.length()>40){
            cout << "line too long (40 characters max); please try again" << endl;
            continue;
        }
        rdt_send(target,message,"98");
        string echoed_line = rdt_receive_a_line(target,"99");
        if(echoed_line == "\n"){
            break;
        }
        cout << " echoed '" << echoed_line.substr(0,echoed_line.length()-1) << "' received from " << target << endl;
    }
    cout << "echoapp: terminated" << endl;
}

void console(int* master_socket_fd){
    for(;;){
        cout << port << "> ";
        cout.flush();
        string cmd;
        getline(cin,cmd);
        if(cmd=="") continue;
        else if(cmd=="neighbors"){
            m.lock();
            vector<int> neighbors = count_neighbors();
            if(neighbors.size()==0){
                cout << port << " has no active neighbors." << endl;
            }else{
                cout << "Active neighbors of " << port << ":" << endl;
                cout << "\t";
                for(int i=0;i<neighbors.size();++i){
                    if(i!=(int)neighbors.size()-1)
                        cout << connection_list[neighbors[i]]->neighbors_id << ",";
                    else
                        cout << connection_list[neighbors[i]]->neighbors_id;
                }
                cout << endl;
            }
            m.unlock();
        }
        else if(cmd=="quit"){
            break;
        }
        else if(cmd=="netgraph"){
            m.lock();
            bfs_update();
            vector<int> neighbors = count_neighbors();
            if(neighbors.size()==0){
                cout << port << " has not active neighbors" << endl;
                m.unlock();
                continue;
            }
            for(auto it: adj_list){
                cout << it.first << ": ";
                for(int i=0;i<it.second.size();++i){
                    if(i!=(int)it.second.size()-1)
                        cout << it.second[i] << ",";
                    else
                        cout << it.second[i];
                }
                cout << endl;
            }
            m.unlock();
        }
        else if(cmd == "forwarding"){
            m.lock();
            bfs_update();
            vector<int> neighbors = count_neighbors();
            if(neighbors.size()==0){
                cout << port << " has an empty forwarding table" << endl;
                m.unlock();
                continue;
            }
            if(forwarding_table.empty() || forwarding_table.size() == 0){
                cout << port << " has an empty forwarding table" << endl;
            }else{
                for(auto it: forwarding_table){
                    cout << it.first << ": " << it.second << endl;
                }
            }
            m.unlock();
        }
        else if(cmd.find("udtsend")!=string::npos){
            m.lock();
            stringstream ss(cmd);
            string type, target, body;
            ss>>type>>target>>body;
            udt_send(target,body,true,type, to_int(max_ttl));
            m.unlock();

        }
        else if(cmd.find("traceroute") != string::npos){
            m.lock();
            stringstream ss(cmd);
            string type, target;
            ss>>type>>target;
            if(target == port){
                cout  << "Cannot traceroute to yourself." << endl;
                m.unlock();
                continue;
            }
            m.unlock();
            traceroute(target);
        }
        else if(cmd.find("rdtsend")!=string::npos){
            m.lock();
            stringstream ss(cmd);
            string type, target, message;
            ss>>type>>target>>message;
            bfs_update();
            if(target == port){
                cout << "Cannot use rdtsend command to send message to yourself" << endl;
                m.unlock();
                continue;
            }
            if(forwarding_table.find(target) == forwarding_table.end()){
                cout << target << " is not reachable" << endl;
                m.unlock();
                continue;
            }
            m.unlock();
            rdt_send(target, message, "0");
        }
        else if(cmd.find("echoapp")!=string::npos){
            m.lock();
            stringstream ss(cmd);
            string type, target;
            ss>>type>>target;
            bfs_update();
            if(target == port){
                cout << "Cannot use echoapp command to send message to yourself" << endl;
                m.unlock();
                continue;
            }
            if(forwarding_table.find(target) == forwarding_table.end()){
                cout << target << " is not reachable" << endl;
                m.unlock();
                continue;
            }
            m.unlock();
            echoapp_client(target);
        }
        else{
            cout << "Command not recognized.  Valid commands are:\n";
            cout << "\techoapp target\n";
            cout << "\tforwarding\n";
            cout << "\tneighbors\n";
            cout << "\tnetgraph\n";
            cout << "\trdtsend target message\n";
            cout << "\ttraceroute target\n";
            cout << "\tquit\n";
            cout.flush();
        }
    }
    m.lock();
    vector<int> neighbors = count_neighbors();
    for(int i=0;i<neighbors.size();++i){
        shared_ptr<Connection> c = connection_list[neighbors[i]];
        if(c->socket_fd>=0){
            shutdown(c->socket_fd,SHUT_RDWR);
            close(c->socket_fd);
            c->socket_fd = -2;
        }
    }
    shutdown(*master_socket_fd, SHUT_RDWR);
    close(*master_socket_fd);
    *master_socket_fd = -1;
    m.unlock();
    reaper_add_work(nullptr);    
}

int get_ucastapp_type(string message){
    stringstream ss(message);
    string dummy,type;
    ss>>dummy>>type;
    if(type == "PING") return 1;
    else if(type == "PONG") return 2;
    else if(type == "TTLZERO") return 3;
    else if(type == "RDTACK") return 4;
    else if(type == "RDTDATA") return 5;
    else{
        cerr << "Unknown message type" << endl;
        return -1;
    }
}

void reader_thread(shared_ptr<Connection> conn_ptr, int* master_socket_fd){
    m.lock();
    m.unlock();
    // 0 - NODEID, 1 - ttl, 2 - flood, 3 - content-length, 4 - entire message
    vector<string> hello_message_params = read_hello_message(conn_ptr);
    if(hello_message_params[0]=="NULL") return;
    log_receive_hello(hello_message_params);

    if(conn_ptr->neighbors_id == "-1"){ // created in main thread
        m.lock();
        for(int i=0;i<connection_list.size();++i){
            if(connection_list[i]!=conn_ptr && connection_list[i]->socket_fd >=0 && connection_list[i]->neighbors_id==hello_message_params[0]){
                conn_ptr->is_duplicate = true;
                break;
            }
        }
        m.unlock();

        if(!conn_ptr->is_duplicate){
            m.lock();
            conn_ptr->neighbors_id = hello_message_params[0];
            m.unlock();
            string hello_message = create_hello_message();
            conn_ptr->add_work(make_shared<string>(hello_message));
        }
    }
    else{ // created in neighbors thread
        m.lock();
        for(int i=0;i<connection_list.size();++i){
            if(connection_list[i]!=conn_ptr && connection_list[i]->socket_fd >=0 && connection_list[i]->neighbors_id==hello_message_params[0]){
                conn_ptr->is_duplicate = true;
                break;
            }
        }
        m.unlock();
    }

    if(!conn_ptr->is_duplicate){
        create_and_flood_lsupdate(conn_ptr);
        int cnt = 0;
        for(;;){
            cnt++;
            string line;
            int message_type = read_header_and_get_type(conn_ptr);
            //cerr << "Message type is: " << message_type << endl;
            if(message_type < 0) break;
            if(message_type == 1){ // lsupdate
                map<string,string> lsupdate_info= read_lsupdate_message_header(conn_ptr);
                if(lsupdate_info.find("NULL")!=lsupdate_info.end()) {
                    break; // reach EOF
                }
                string lsupdate_body = read_lsupdate_message_body(to_int(lsupdate_info["Content-Length"]),conn_ptr->socket_fd);
                log_receive_lsupdate(lsupdate_info,lsupdate_body,conn_ptr);

                m.lock();
                if(*master_socket_fd == -1){
                    m.unlock();
                    break;
                }
                if(received_message_ids.find(lsupdate_info["MessageID"]) == received_message_ids.end()){
                    received_message_ids.insert(lsupdate_info["MessageID"]);
                    if(lsupdate_info["Flood"] == "1"){
                        int ttl = to_int(lsupdate_info["TTL"]);
                        if(ttl!=0)
                        {
                            ttl--; // decrement ttl
                            vector<int> neighbors = count_neighbors();
                            for(int i=0;i<neighbors.size();++i){
                                if(connection_list[neighbors[i]]!=conn_ptr){
                                    string lsupdate_message = create_lsupdate_message(to_string(ttl),lsupdate_info["Flood"],lsupdate_info["MessageID"],lsupdate_info["From"],
                                        lsupdate_info["OriginStartTime"],lsupdate_info["Content-Length"],lsupdate_body);
                                    connection_list[neighbors[i]]->add_work(make_shared<string>(lsupdate_message));
                                }
                            }
                        }
                    }
                }
                m.unlock();
               
                int flood_status_code = update_adj_list(lsupdate_info["From"],lsupdate_body);
                //cerr << "lsupdate flood: " << lsupdate_info["From"] << " " << lsupdate_body << " " << flood_status_code  << endl;
                if(flood_status_code){
                    m.lock();
                    vector<int> neighbors = count_neighbors();
                    m.unlock();
                    for(int i=0;i<neighbors.size();++i){
                        if(cnt==1 && connection_list[neighbors[i]] == conn_ptr) continue;
                        create_and_flood_lsupdate(connection_list[neighbors[i]]);
                    }
                }
            }else if(message_type == 2){ // ucastapp
                map<string,string> ucastapp_header = read_ucastapp_header(conn_ptr);
                if(ucastapp_header.find("NULL") != ucastapp_header.end()){
                    break;// reach EOF
                }
                string ucastapp_body = read_ucastapp_body(to_int(ucastapp_header["Content-Length"]),conn_ptr->socket_fd);
                log_receive_ucastapp(ucastapp_header,ucastapp_body,conn_ptr);
                stringstream ss(ucastapp_body);
                string dummy, type, seq_num;
                ss>>dummy>>type>>seq_num;
                m.lock();
                if(*master_socket_fd == -1){
                    m.unlock();
                    break;
                }
                if(ucastapp_header["To"] == port){
                    int ucastapp_type = get_ucastapp_type(ucastapp_body);
                    
                    if(ucastapp_type == 1) // PING
                    {
                        //cerr << "Received PING, trying to send PONG back" << endl;
                        string message_id;
                        string body = "353UDT/1.0 PONG " + seq_num;
                        udt_send(ucastapp_header["From"],body,false,message_id, to_int(max_ttl)); // sending message out :D
                    }
                    else if(ucastapp_type == 2 || ucastapp_type == 3) // PONG
                    {

                        if(type == "TTLZERO"){
                            timer_list.back()->status = 3;
                        }else{
                            timer_list.back()->status = 2;
                        }
                        timer_list.back()->add_work(ucastapp_header,ucastapp_body);
                    }
                    else if(ucastapp_type == 4){ // RDTACK
                        timer_list.back()->status = 4;
                        timer_list.back()->seq_no = to_int(seq_num);
                        timer_list.back()->add_work(ucastapp_header,ucastapp_body);
                    }
                    else if(ucastapp_type == 5){ // RDTDATA
                        map<string,string> message_content = parse_rdtdata_body(ucastapp_body);
                        int num = find_receiver_for_rdtsend(ucastapp_header["From"], message_content["RDT-App-Number"]);
                        if(num == -1){ // object not created yet
                            Message message(ucastapp_header["From"], message_content["Content"], 1, to_int(message_content["RDT-App-Number"]));
                            received_message.emplace_back(message);
                            string rdtack_message = create_rdtack_body("0","0");
                            string message_id;
                            if(message_content["Content"] == "\n"){
                                num = find_receiver_for_rdtsend(ucastapp_header["From"], message_content["RDT-App-Number"]);
                                if(message_content["RDT-App-Number"] == "0") 
                                    cout << " RDT message '" << received_message[num].content <<  "' received from " <<  ucastapp_header["From"] << endl;
                                received_message.erase(received_message.begin()+num);
                            }
                            udt_send(ucastapp_header["From"],rdtack_message,false,message_id,to_int(max_ttl));
                        }else{
                            if(message_content["Seq-Number"] == to_string(received_message[num].seq_no)){
                                string message_id;
                                string rdtack_message = create_rdtack_body(message_content["Seq-Number"],"0");
                                udt_send(ucastapp_header["From"],rdtack_message,false,message_id,to_int(max_ttl));
                                if(message_content["Content"] == "\n"){ // reach the end of message
                                    if(message_content["RDT-App-Number"] == "0")
                                        cout << " RDT message '" << received_message[num].content <<  "' received from " <<  ucastapp_header["From"] << endl;
                                    received_message.erase(received_message.begin()+num);
                                }else{ // still receiving message
                                    received_message[num].content+=message_content["Content"];
                                    received_message[num].seq_no = !received_message[num].seq_no;
                                }
                            }
                        }
                        // ECHOAPP
                        if(message_content["RDT-App-Number"] != "0"){ // if it is an echoapp message
                            if(message_content["RDT-App-Number"] == "98") // hear from (first time)
                            {
                                int num = find_receiver_for_echoapp(ucastapp_header["From"], "98");
                                if(num == -1){
                                    shared_ptr<EchoJob> echo_ptr = make_shared<EchoJob>(EchoJob(ucastapp_header["From"], "98", message_content["Content"],nullptr));
                                    echo_jobs.emplace_back(echo_ptr);
                                }else{
                                    echo_jobs[num]->message += message_content["Content"];
                                }
                                num = find_receiver_for_echoapp(ucastapp_header["From"], "98");
                                if(message_content["Content"] == "\n"){ // reach the end of echo
                                    thread t = thread(rdt_send,ucastapp_header["From"], echo_jobs[num]->message,"99");
                                    //cerr << "Sending message: " << echo_jobs[num]->message << endl;
                                    t.detach();
                                    echo_jobs.erase(echo_jobs.begin()+num);
                                }
                            }
                            else // received echo == "99"
                            {
                                int num = find_receiver_for_echoapp(ucastapp_header["From"], "99");
                                if(num == -1){ // no object created yet
                                    shared_ptr<EchoJob> echo_ptr = make_shared<EchoJob>(EchoJob(ucastapp_header["From"], "99", message_content["Content"],nullptr));
                                    echo_jobs.emplace_back(echo_ptr);
                                }else{
                                    echo_jobs[num]->message += message_content["Content"];
                                }
                                num = find_receiver_for_echoapp(ucastapp_header["From"], "99");
                                if(message_content["Content"] == "\n"){ // reach the end of echo
                                    echo_jobs[num]->done = true;
                                    usleep(500000);
                                }
                            }
                        }
                    }
                   
                    m.unlock();
                    continue;
                }
                
                int ttl = to_int(ucastapp_header["TTL"]);
                ttl--;
                if(ttl!=0){
                    vector<int> neighbors = count_neighbors();
                    bfs_update();
                    string receiver = forwarding_table[ucastapp_header["To"]];
                    ucastapp_header["TTL"] = to_string(ttl);
                    string ucastapp_message = create_ucastapp_message(ucastapp_header,ucastapp_body);
                    for(int i=0;i<neighbors.size();++i){
                        if(connection_list[neighbors[i]]->neighbors_id == receiver){
                            connection_list[neighbors[i]]->add_work(make_shared<string>(ucastapp_message));
                            break;
                        }
                    }
                }
                else{ // if ttl is 0
                    string message_id;
                    string body = "353UDT/1.0 TTLZERO " + seq_num;
                    udt_send(ucastapp_header["From"],body,false,message_id, to_int(max_ttl)); // sending message out :D
                }
                m.unlock();
            }
        }
    }
    m.lock();
    if(conn_ptr->socket_fd>=0){
        shutdown(conn_ptr->socket_fd,SHUT_RDWR);
        close(conn_ptr->socket_fd);
    }
    conn_ptr->socket_fd = -1;
    
    vector<int> neighbors = count_neighbors();
    m.unlock();
    for(int i=0;i<neighbors.size();++i){
        create_and_flood_lsupdate(connection_list[neighbors[i]]);
    }
    conn_ptr->add_work(nullptr);
    conn_ptr->write_thread_ptr->join();
    reaper_add_work(conn_ptr);
}

void writer_thread(shared_ptr<Connection> conn_ptr, int* master_socket_fd){
    m.lock();
    m.unlock();
    for(;;){
        shared_ptr<string> message = conn_ptr->wait_for_work();
        if(message==nullptr){ 
            break;
        }
        int bytes_written = better_write(conn_ptr->socket_fd,(*message).c_str(),(*message).length());
        if(bytes_written < 0 || *master_socket_fd == -1) {
            break;
        }
        int type = get_message_type(message);
        if(type == 1){
            log_write_hello(*message, conn_ptr);
        }else if(type == 2){ // due to initiation
            log_write_lsupdate(*message,0, conn_ptr);
        }else if(type==3){ // due to flood
            log_write_lsupdate(*message,1, conn_ptr);
        }else if(type == 4){ // due to initiation
            log_write_ucastapp(*message,0,conn_ptr);
        }else if(type == 5){ // due to forwarding
            log_write_ucastapp(*message,1,conn_ptr);
        }else if(type == 6){
            log_write_rdtdata(*message,0,conn_ptr);
        }else if(type == 7){
            log_write_rdtdata(*message,1,conn_ptr);
        }
    }

    m.lock();
    if(conn_ptr->socket_fd>=0){
        shutdown(conn_ptr->socket_fd,SHUT_RDWR);
        close(conn_ptr->socket_fd);
    }
    conn_ptr->socket_fd = -1;
    m.unlock();
}

void add_peer(const string& ini_filename){
    map<string,map<string,string>> config = read_ini_file(ini_filename);

    string logfile = config["startup"]["logfile"];
    port = ":"+config["startup"]["port"];
    host = config["startup"]["host"];
    if(host==""){
        host = LOCALHOST;
    }
    max_ttl = config["params"]["max_ttl"];
    sleeping_time = to_int(config["params"]["neighbor_retry_interval"]);
    string pidfile = config["startup"]["pidfile"];
    message_expiration_time = to_int(config["params"]["msg_lifetime"]);

    output_pid(pidfile);

    myfile.open(logfile,ofstream::out|ofstream::app);
    if(config["logging"]["SAYHELLO"] == "1"){
        mylog_sayhello = &myfile;
    }else{
        mylog_sayhello = &cout;
    }

    if(config["logging"]["LSUPDATE"] == "1"){
        mylog_lsupdate = &myfile;
    }else{
        mylog_lsupdate = &cout;
    }

    if(config["logging"]["UCASTAPP"] == "1"){
        mylog_ucastapp = &myfile;
    }else{
        mylog_ucastapp = &cout;
    }

	int master_socket_fd = create_master_socket(port.substr(1));
    if (master_socket_fd != (-1)) {
        thread console_thread(console, &master_socket_fd);
        thread reaper_thread(reaper,&master_socket_fd);
        thread neighbors_thread(neighbors,&master_socket_fd,ini_filename);
        for (;;) {
            int newsockfd = my_accept(master_socket_fd);

            if (newsockfd == (-1)) break;
            m.lock();
            if(master_socket_fd == -1){
                shutdown(newsockfd, SHUT_RDWR);
                close(newsockfd);
                m.unlock();
                break;
            }
            shared_ptr<Connection> conn_ptr = make_shared<Connection>(Connection(++next_conn_number, newsockfd, NULL,NULL));
            shared_ptr<thread> read_thread_ptr = make_shared<thread>(thread(reader_thread, conn_ptr, &master_socket_fd));
            shared_ptr<thread> write_thread_ptr = make_shared<thread>(thread(writer_thread,conn_ptr,&master_socket_fd));
            conn_ptr->read_thread_ptr = read_thread_ptr;
            conn_ptr->write_thread_ptr = write_thread_ptr;
            conn_ptr->neighbors_id = "-1";
            connection_list.push_back(conn_ptr);
            m.unlock();
        }

        shutdown(master_socket_fd, SHUT_RDWR);
        close(master_socket_fd);
        console_thread.join();
        reaper_thread.join();
        neighbors_thread.join();
    }
}

// #########################################################################################################
// #########################################################################################################
// #########################################################################################################

shared_ptr<Connection> find_connection_by_number(int num){
    for(int i=0;i<connection_list.size();++i){
        if(connection_list[i]->conn_number == num){
            return connection_list[i];
        }
    }
    return nullptr;
}

vector<int> count_neighbors(){
    vector<int> neighbors;
    neighbors.clear();
    for(int i=0;i<connection_list.size();++i){
        if(connection_list[i]->socket_fd>=0){
            neighbors.emplace_back(i);
        }
    }
    return neighbors;
}

void bfs_update(){
    set<string> remain;
    forwarding_table.clear();
    remain.insert(port);
    queue<string> q;
    q.push(port);
    while(!q.empty()){
        string from = q.front();
        q.pop();
        vector<string> temp = adj_list[from];
        for(int i=0;i<temp.size();++i){
            if(remain.find(temp[i])==remain.end()){
                remain.insert(temp[i]);
                q.push(temp[i]);
            }
        }
    }
    map<string,vector<string>> new_adj_list;
    for(auto it: remain){
        new_adj_list[it] = adj_list[it];
    }

    adj_list = new_adj_list;

    while(!q.empty()) q.pop();
    q.push(port);
    remain.clear();
    remain.insert(port);
    while(!q.empty()){
        string from = q.front();
        q.pop();
        vector<string> temp = adj_list[from];
        for(int i=0;i<temp.size();++i){
            if(remain.find(temp[i])==remain.end()){
                remain.insert(temp[i]);
                q.push(temp[i]);
                if(from == port){
                    forwarding_table[temp[i]] = temp[i];
                }else{
                    string curr = from;
                    while(forwarding_table[curr]!=curr){
                        curr = forwarding_table[curr];
                    }
                    forwarding_table[temp[i]] = curr;
                }
            }
        }
    }
}

int update_adj_list(const string& from, string& lsupdate_body){
    int code = 0;
    //cerr << "Update request from: " << from << endl;
    if(adj_list.find(from) == adj_list.end()){
        code = 1;
    }
    vector<string> neighbors_ids;
    stringstream ss(lsupdate_body);
    string id;
    while(getline(ss,id,',')){
        neighbors_ids.emplace_back(id);
    }
    adj_list[from] = neighbors_ids;
    return code;
}

void create_and_flood_lsupdate(shared_ptr<Connection> conn_ptr){
    m.lock();
    string message_id, origin_start_time;
    GetObjID(port, "LSUPDATE", message_id, origin_start_time);
    vector<int> neighbors = count_neighbors();
    vector<string> neighbors_ids;
    for(int i=0;i<(int)neighbors.size();++i){
        neighbors_ids.emplace_back(connection_list[neighbors[i]]->neighbors_id);
    }
    adj_list[port] = neighbors_ids;
    string body = "";
    for(int i=0;i<(int)neighbors.size();++i){
        if(i!=(int)neighbors.size()-1){
            body+=connection_list[neighbors[i]]->neighbors_id;
            body+=",";
        }else{
            body+=connection_list[neighbors[i]]->neighbors_id;
        }
    }
    string lsupdate_message = create_lsupdate_message(max_ttl,"1",message_id,port,origin_start_time,to_string(body.length()),body);
    conn_ptr->add_work(make_shared<string>(lsupdate_message));
    m.unlock();
}

set<string> get_neighbors_ids(const string& ini_filename){
    ifstream ifile(ini_filename);
    string line;
    set<string> ans;
    bool entered = false;
    while(getline(ifile,line)){
        if(entered) break;
        if(line.length()==0) continue;
        if(line=="[topology]"){
            entered = true;
            while(getline(ifile,line)){
                stringstream ss(line);
                string key,value;
                getline(ss,key,'=');
                getline(ss,value);
                if(key==port){
                    stringstream ss1(value);
                    while(getline(ss1,line,',')){
                        ans.insert(line);
                    }
                    break;
                }   
            }
        }
    }
    return ans;
}

void neighbors(int* master_socket_fd, const string& ini_filename){
    set<string> neighbors_ids = get_neighbors_ids(ini_filename);
    for(;;){
        set<string> l = neighbors_ids;
        m.lock();
        if(*master_socket_fd==-1){
            m.unlock();
            break;
        }
        else{
            for(int i=0;i<connection_list.size();++i){
                if(connection_list[i]->neighbors_id!="-1"){
                    l.erase(l.find(connection_list[i]->neighbors_id));
                }
            }
        }
        m.unlock();

        for(auto it: l){
            int client_socket_fd = create_client_socket_and_connect(host, it.substr(1));
            if(client_socket_fd>=0){
                m.lock();
                shared_ptr<Connection> conn_ptr = make_shared<Connection>(Connection(++next_conn_number, client_socket_fd, NULL,NULL));

                shared_ptr<thread> write_thread_ptr = make_shared<thread>(thread(writer_thread, conn_ptr, master_socket_fd));
                shared_ptr<thread> read_thread_ptr = make_shared<thread>(thread(reader_thread, conn_ptr, master_socket_fd));
                
                conn_ptr->read_thread_ptr = read_thread_ptr;
                conn_ptr->write_thread_ptr = write_thread_ptr;
                conn_ptr->neighbors_id = it;
                connection_list.push_back(conn_ptr);
                m.unlock();
                /* say hello first */
                string hello_message = create_hello_message();
                conn_ptr->add_work(make_shared<string>(hello_message));
            }
        }
        sleep(sleeping_time);
    }
}


int main(int argc, char** argv){
    if(argc!=2){
        usage();
    }else{
        add_peer(argv[1]);
    }
    return 0;
}
