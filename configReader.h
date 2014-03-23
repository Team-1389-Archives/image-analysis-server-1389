#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <cstdlib>


using namespace std;

class ConfigReader;

class ConfigReader{
    public:
    	ConfigReader(){}
    	ConfigReader(const ConfigReader &other)
    		: values(other.values)
    	{}
    	virtual ~ConfigReader(){}
        bool loadFromString(string);
        bool loadFromFile(string);
        string getString(string);
    private:
        map <string, string> values;
};
