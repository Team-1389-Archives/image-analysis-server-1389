#include <vector>
#include <string>
#include <map>
#include <fstream>

using namespace std;

class ConfigReader;

class ConfigReader{
    public:
        bool loadFromString(string);
        bool loadFromFile(string);
        string getString(string);
        int getInt(string);
        float getFloat(string);
    private:
        map <string, string> values;
};
