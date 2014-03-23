#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>


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
    private:
    	bool loadFromFile(string);
        map <string, string> values;
};
