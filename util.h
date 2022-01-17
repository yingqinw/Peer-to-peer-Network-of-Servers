#ifndef _UTIL_H_
#define _UTIL_H_

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <sys/time.h>
#include <memory>
#include "unistd.h"

/**
     * Use this code to return the file size of path.
     *
     * You should be able to use this function as it.
     *
     * @param path - a file system path.
     * @return the file size of path, or (-1) if failure.
*/
int get_file_size(std::string path);

/**
 Change a string to a corresponding integer
**/
int to_int(std::string s);

std::string print_with_tab(const std::string& s);

void usage();

void output_pid(const std::string& pid_file);

void GetObjID(
        std::string node_id,
        const char *obj_category,
        std::string& hexstring_of_unique_obj_id,
        std::string& origin_start_time);

// 1 represents SAYHELLO, 2 represents LSUPDATE i, 3 represents LSUPDATE d, 4 represents UCASTAPP i, 5 represents UCASTAPP f
int get_message_type(std::shared_ptr<std::string> message);


#endif