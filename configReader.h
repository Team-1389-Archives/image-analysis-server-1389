#ifndef CONFIG_READER_H
#define CONFIG_READER_H
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <cstdlib>


using namespace std;

class ConfigReader;

class ConfigReader{
    public:
    	ConfigReader(string name){
    		if(!loadFromFile(name)){
    			cerr<<"Unable to open config file "<<name<<endl;
    		}
    	}
    	ConfigReader(const ConfigReader &other)
    		: values(other.values)
    	{}
    	virtual ~ConfigReader(){}
        bool loadFromString(string);
        string getString(string);
        int getIntDefault(string key, int def){
            if(values.count(key)){
                string str=values[key];
                return atoi(str.c_str());
            }else{
                cerr<<"Config value for "<<key<<" not found"
                    <<"defaulting to "<<def<<endl;
                return def;
            }
        }
    private:
    	bool loadFromFile(string);
        map <string, string> values;
};

#endif