#include <fstream>
#include <string>
#include <iostream>

using namespace std;

class protocall{
    public:
        void getInString(string in);
        string getData(string varName);
    private:
        string data;
};

void protocall::getInString(string in){
    for (int i = 0; i < in.length(); ++i){
        if (in[i] != ' ' && in[i] != '\n' && in[i] != '\t')
            data += in[i];
    }
}

string protocall::getData(string varName){
    int index = data.find(varName);
    if (index == -1)
        return "ERROR";
    string temp = data.substr(data.find_first_of('=',index) + 1, data.find_first_of(';',index) - data.find_first_of('=',index) - 1);

    return temp;
};

int main(){
    string name;
    cout << "type file name" << endl;
    getline(cin, name);
    ifstream in(name.c_str()); 
    string file;
    string line;
    while(std::getline(in, line)) {
        file += line;
    }
    
    protocall pro;
    pro.getInString(file);
    
    while (name != "done"){
        cout << "what variale do you want?" << endl;
        getline(cin, name);
    
        cout << "value:" << pro.getData(name) << endl;
    }
}