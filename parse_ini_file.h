#ifndef _PARSE_INI_FILE_H_
#define _PARSE_INI_FILE_H_

#include <map>
#include <string>

using namespace std;

std::map<std::string,std::map<std::string,std::string>> read_ini_file(std::string filename);


#endif