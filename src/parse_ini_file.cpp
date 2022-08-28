#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <unordered_map>
#include <set>
#include <string>

using namespace std;


map<string,map<string,string>> read_ini_file(string filename){
	ifstream ifile(filename);
	string line;
	map<string,map<string,string>> ans; ans.clear();
	map<string,string> curr; curr.clear();
	string section="";

	while(getline(ifile,line)){
		if(line.length()==0) continue;
		if(line[0] == '[' && line[line.length()-1]==']'){
			if(section.length()!=0 && section!=" "){
				ans[section] = curr;
				curr.clear();
			}
			section = line.substr(1,line.length()-2);
		}else{
			stringstream ss(line);
			string key,value;
			getline(ss,key,'=');
			getline(ss,value);
			curr[key] = value;
		}
	}
	ans[section] = curr;

	return ans;
}

void output_all_sections(const map<string,map<string,string>>& m){
	for(auto it: m){
		cout << it.first << endl;
	}
}

void output_all_keys(map<string,map<string,string>>& m, const string& section, const string& filename){
	if(m.find(section)==m.end()){
		cerr << "Cannot find the [" << section << "] section in " << filename << endl;
	}else{
		map<string,string> curr = m[section];
		for(auto it: curr){
			cout << it.first << endl;
		}
	}
}

void output_value(map<string,map<string,string>>& m, const string& section, const string& key, const string& filename){
	if(m.find(section)==m.end()){
		cerr << "Cannot find the [" << section << "] section in " << filename << endl;
	}else{
		map<string,string> curr = m[section];
		if(curr.find(key)==curr.end()){
			cerr << "Cannot find the \"" << key << "\" key in the [" << section << "] section in " << filename << endl;
			return; 
		}
		string value = curr[key];
		if(value.length()==0){
			cerr << "There is no value for \"" << key << "\" key in the [" << section << "] section in " << filename << endl;
			return;
		}
		cout << value << endl;
	}
}

