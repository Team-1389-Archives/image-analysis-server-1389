#include "configReader.h"
#include <iostream>

bool ConfigReader::loadFromString(string input){
    string in;
    for (unsigned int i = 0; i < input.size() ; ++i){//remove whitespace
        if (input[i] != ' ' && input[i] != '\n' && input[i] != '\t')
            in += input[i];
    }
    values.clear();
    unsigned int charNum = 0;
    string key, value;
    if (in.size() == 0)
        return false;

    while(charNum < in.size()){
        key = "";
        while(in[charNum] != '='){//look for next key
            if (charNum == in.size()){
                return false;
            }
            if (in[charNum] == ';'){
                return false;
            }
            key += in[charNum];
            charNum++;
        }

        value = "";
        charNum++;
        if (charNum == in.size()){
                return false;
        }

        while(in[charNum] != ';'){//look for next value
            if (charNum == in.size()){
                return false;
            }
            if (in[charNum] == '='){
                return false;
            }
            value += in[charNum];
            charNum++;
        }
        if (key == "" || value == "")
            return false;
        values[key] = value;
        charNum++;
    }
    return true;
};

bool ConfigReader::loadFromFile(string fileName){
    string line;
    string allFile = "";
    ifstream file (fileName);
    if (!file.is_open())
        return false;
    while ( getline (file,line) ){
            allFile += line;
    }
    file.close();
    return loadFromString(allFile);
}

string ConfigReader::getString(string name){
    return values[name];
}

