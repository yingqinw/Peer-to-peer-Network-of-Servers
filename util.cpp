#include <string>
#include <iostream>
#include <sys/stat.h>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <map>

using namespace std;

#include "util.h"

extern string port;

/**
     * Use this code to return the file size of path.
     *
     * You should be able to use this function as it.
     *
     * @param path - a file system path.
     * @return the file size of path, or (-1) if failure.
*/
int get_file_size(std::string path)
{
    struct stat stat_buf;
    if (stat(path.c_str(), &stat_buf) != 0) {
        return (-1);
    }
    return (int)(stat_buf.st_size);
}

int to_int(std::string s){
    int n = s.length();
    int ans = 0;
    for(int i=0;i<n;++i){
        ans+=(s[i]-'0')*pow(10,n-i-1);
    }
    return ans;
}

string print_with_tab(const std::string& s){
    std::string ans="\t";
    for(int i=0;i<static_cast<int>(s.length());++i){
        if(s[i]!=static_cast<int>(s.length()-1) && s[i]=='\n'){
            ans+=s[i];
            ans+='\t';
        }else{
            ans+=s[i];
        }
    }
    return ans;
}

void GetObjID(
        string node_id,
        const char *obj_category,
        string& hexstring_of_unique_obj_id,
        string& origin_start_time)
{
    static unsigned long seq_no = 1L;
    static int first_time = 1;
    static struct timeval node_start_time;
    static char origin_start_time_buf[18];
    char hexstringbuf_of_unique_obj_id[80];

    /* IMPORTANT: this code must execute inside a critical section of the main mutex */
    if (first_time) {
        gettimeofday(&node_start_time, NULL);
        snprintf(origin_start_time_buf, sizeof(origin_start_time_buf),
                "%010d.%06d", (int)(node_start_time.tv_sec), (int)(node_start_time.tv_usec));
        first_time = 0;
    }
    seq_no++;
    struct timeval now;
    gettimeofday(&now, NULL);
    snprintf(hexstringbuf_of_unique_obj_id, sizeof(hexstringbuf_of_unique_obj_id),
            "%s_%1ld_%010d.%06d", node_id.c_str(), (long)seq_no, (int)(now.tv_sec), (int)(now.tv_usec));
    hexstring_of_unique_obj_id = hexstringbuf_of_unique_obj_id;
    origin_start_time = origin_start_time_buf;
}

void output_pid(const string& pid_file){
    int pid = static_cast<int>(getpid());
    ofstream ofile(pid_file);
    ofile<<pid<<endl;
}

void usage(){
    cerr << "Usage: ./lab14a CONFIGFILE" << endl;
    exit(-1);
}

int get_message_type(shared_ptr<string> message){
    string line = *message;
    stringstream ss(line);

    string dummy, type;
    ss>>dummy>>type;
    if(type== "SAYHELLO"){
        return 1;
    }else if(type == "LSUPDATE"){
        int cnt = 0;
        line = *message;
        //stringstream ss1(line);
        map<string,string> header;
        while(getline(ss,line,'\n')){
            cnt++;
            if(cnt==1) {
                continue;
            }
            if(line.length()==1 && line[0]=='\r'){ 
                break;
            }
            stringstream ss1(line);
            string key, value;
            ss1>>key>>value;
            key = key.substr(0,key.length()-1);
            header[key] = value;  
        }
        if(header["From"]==port)
            return 2;
        else
            return 3;
    }else if(type == "UCASTAPP"){
        int cnt = 0;
        line = *message;
        //stringstream ss1(line);
        map<string,string> header;
        while(getline(ss,line,'\n')){
            cnt++;
            if(cnt==1) {
                continue;
            }
            if(line.length()==1 && line[0]=='\r'){ 
                break;
            }
            stringstream ss1(line);
            string key, value;
            ss1>>key>>value;
            key = key.substr(0,key.length()-1);
            header[key] = value;  
        }
        string body;
        getline(ss,body,'\0');
        cnt = 0;
        for(int i=0;i<body.length();++i){
            if(body[i] == '\n') cnt++;
        }
        if(cnt<=1)
        {
            if(header["From"] == port){
                return 4;
            }else{
                return 5;
            }
        }
        else{
            if(header["From"] == port){
                return 6;
            }else{
                return 7;
            }
        }
    }
    else{
        cerr << "Unknown meessage type: " << type << endl;
        return -1;
    }

}
