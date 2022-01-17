#include <cstring>
#include <vector>
#include <iomanip>
#include <string>
#include <cmath>

#include "create_read_message.h"
#include "util.h"
#include "my_readwrite.h"

using namespace std;

extern string port;
extern string max_ttl;


/* SAYHELLO */

string create_hello_message(){
    string ans = "353NET/1.0 SAYHELLO NONE/1.0\r\n";
    ans+="TTL: 1\r\n";
    ans+="Flood: 0\r\n";
    ans+="From: "; ans+=port; ans+="\r\n";
    ans+="Content-Length: 0\r\n"; 
    ans+="\r\n";
    return ans;
}

/*
    Process the SAYHELLO Message from neighbors
*/
vector<string> read_hello_message(shared_ptr<Connection> conn_ptr){
    vector<string> ans; ans.clear();
    string entire_message = "";
    string neighbors_id = "";
    string flood_neighbors = "";
    string ttl_neighbors = "";
    string content_length = "";
    int cnt = 0;
    int bytes_received = -1;

    for(;;){
        string line="";
        cnt++;
        bytes_received = read_a_line(conn_ptr->socket_fd,line);
        entire_message+=line;
        if(bytes_received <= 0){
            ans.emplace_back("NULL");
            return ans;
        }
        // seperates the header from body
        if(bytes_received == 2 && line[0]=='\r' && line[1] == '\n'){
            break;
        }
        if(cnt==1) continue;
        stringstream ss(line);
        string key, value;  
        ss>>key>>value;
        key = key.substr(0,key.length()-1);
        if(key=="TTL") ttl_neighbors = value;
        else if(key=="Content-Length"){
            content_length = value;
        }else if(key=="From"){
            neighbors_id = value;
        }
    }

    ans.emplace_back(neighbors_id);
    ans.emplace_back(max_ttl);
    ans.emplace_back("-");
    ans.emplace_back(content_length);
    ans.emplace_back(entire_message);

    return ans;
}

/* LSUPDATE */

int read_header_and_get_type(shared_ptr<Connection> conn_ptr){
    string line;
    int bytes_received = read_a_line(conn_ptr->socket_fd,line);
    if(bytes_received<=0) return -1;
    string temp,type;
    stringstream ss(line);
    ss>>temp>>type;

    if(type == "LSUPDATE") return 1;
    else if(type == "UCASTAPP") return 2;
    else{
        cerr << "Unknown type of message: " << type << endl;
        return 3;
    }

    return -1;
}

string create_lsupdate_message(string ttl,string flood, string MessageID, string From, string OriginStartTime, string content_length, string body){
    string ans = "353NET/1.0 LSUPDATE NONE/1.0\r\n";
    ans+="TTL: "; ans+=ttl; ans+="\r\n";
    ans+="Flood: "; ans+=flood; ans+="\r\n";
    ans+="MessageID: "; ans+=MessageID; ans+="\r\n";
    ans+="From: "; ans+=From; ans+="\r\n";
    ans+="OriginStartTime: "; ans+=OriginStartTime; ans+="\r\n";
    ans+="Content-Length: "; ans+=content_length; ans+="\r\n";
    ans+="\r\n";

    ans+=body;
    return ans;
}

map<string,string> read_lsupdate_message_header(shared_ptr<Connection> conn_ptr){
    map<string,string> ans;
    ans.clear();
    int bytes_received = -1;
    for(;;){
        string line;
        bytes_received = read_a_line(conn_ptr->socket_fd,line);
        if(bytes_received <= 0){
            ans["NULL"] = "NULL";
            return ans;
        }
        // seperates the header from body
        if(bytes_received == 2 && line[0]=='\r' && line[1] == '\n'){
            break;
        }
        stringstream ss(line);
        string key, value;  
        ss>>key>>value;
        key = key.substr(0,key.length()-1);
        ans[key] = value;
    }

    return ans;
}

string read_lsupdate_message_body(int bytes, int fd){
    int bytes_read = 0;
    string ans = "";
    while(bytes_read<bytes){
        char c;
        int bytes_received = read(fd,&c,1);
        if(bytes_received <= 0) break;
        ans+=c;
        bytes_read+=bytes_received;
    }
    return ans;
}

/* UCASTAPP */

string create_ucastapp_message(map<string,string> ucastapp_header, string ucastapp_body){
    string ans = "353NET/1.0 UCASTAPP NONE/1.0\r\n";
    ans+="TTL: "; ans+=ucastapp_header["TTL"]; ans+="\r\n";
    ans+="Flood: 0\r\n"; 
    ans+="MessageID: "; ans+=ucastapp_header["MessageID"]; ans+="\r\n";
    ans+="From: "; ans+=ucastapp_header["From"]; ans+="\r\n";
    ans+="To: "; ans+=ucastapp_header["To"]; ans+="\r\n";

    ans+="Next-Layer: 1\r\n";
    ans+="Content-Length: "; ans+=ucastapp_header["Content-Length"]; ans+="\r\n";
    ans+="\r\n";

    ans+=ucastapp_body;
    return ans;
}

string create_ucastapp_message(const string& to, const string& message, string& message_id, int ttl){
    string ans = "353NET/1.0 UCASTAPP NONE/1.0\r\n";
    ans+="TTL: "; ans+=to_string(ttl); ans+="\r\n";
    ans+="Flood: 0\r\n"; 

    string origin_start_time;
    GetObjID(port, "UCASTAPP", message_id, origin_start_time);
    ans+="MessageID: "; ans+=message_id; ans+="\r\n";

    ans+="From: "; ans+=port; ans+="\r\n";
    ans+="To: "; ans+=to; ans+="\r\n";
    ans+="Next-Layer: 1\r\n";

    int length = message.length();
    ans+="Content-Length: "; ans+=to_string(length); ans+="\r\n";
    ans+="\r\n";
    ans+=message;

    return ans;
}


map<string,string> read_ucastapp_header(shared_ptr<Connection> conn_ptr){
    map<string,string> ans;
    ans.clear();
    int bytes_received = -1;
    for(;;){
        string line;
        bytes_received = read_a_line(conn_ptr->socket_fd,line);
        if(bytes_received <= 0){
            ans["NULL"] = "NULL";
            return ans;
        }
        // seperates the header from body
        if(bytes_received == 2 && line[0]=='\r' && line[1] == '\n'){
            break;
        }
        stringstream ss(line);
        string key, value;  
        ss>>key>>value;
        key = key.substr(0,key.length()-1);
        ans[key] = value;
    }

    return ans;
}

string read_ucastapp_body(int bytes, int fd){
    int bytes_read = 0;
    string ans = "";
    while(bytes_read<bytes){
        char c;
        int bytes_received = read(fd,&c,1);
        if(bytes_received <= 0) break;
        ans+=c;
        bytes_read+=bytes_received;
    }
    return ans;
}

string create_rdtdata_body(char c, string seq_no, string app_no){
    string ans = "353UDT/1.0 RDTDATA NONE/1.0\r\n";
    ans+="Seq-Number: "; ans+=seq_no; ans+="\r\n";
    ans+="RDT-App-Number: "; ans+=app_no; ans+="\r\n";
    ans+="RDT-Content-Length: 1\r\n";
    ans+="\r\n";
    ans+=c; 
    return ans;
}

string create_rdtack_body(string seq_no, string app_no){
    string ans = "353UDT/1.0 RDTACK ";
    ans+=seq_no;
    ans+=" ";
    ans+=app_no;
    return ans;
}

bool isACK(shared_ptr<Timer> t){
    string body = t->body;
    if(body.length()<=10) return false;
    stringstream ss(body);
    string dummy, type;
    ss>>dummy>>type;
    if(type == "RDTACK") return true;
    else return false;
}

map<string,string> parse_rdtdata_body(string s){
    map<string,string> ans;
    ans.clear();
    int cnt = 0;
    stringstream ss(s);
    string line;
    while(getline(ss,line,'\n')){
        cnt++;
        if(cnt == 1) continue;
        if(line.length() == 1 && line[0] == '\r'){
            string c(1,s[s.length()-1]);
            ans["Content"] = c;
            break;
        }
        stringstream ss2(line);
        string key, value;  
        ss2>>key>>value;
        key = key.substr(0,key.length()-1);
        ans[key] = value;
    }

    return ans;
}


