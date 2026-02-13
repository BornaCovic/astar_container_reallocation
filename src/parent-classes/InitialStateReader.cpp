#include <fstream>
#include <string>
#include <vector>

#include "InitialStateReader.h"

InitialStateReader::InitialStateReader(const std::string &filename) : filename(filename) {
    lines = readFile(filename);
}

std::vector<std::string> InitialStateReader::getLines(){
    return lines;
}

std::vector<std::string> InitialStateReader::readFile(const std::string &filename){
    std::ifstream file(filename);
    if(!file.is_open()) throw std::runtime_error("Could not open file: " + filename);

    std::string line;
    std::vector<std::string> lines;
    while(std::getline(file, line)){
        lines.push_back(line);
    }
    
    return lines;
}


std::vector<std::string> InitialStateReader::splitStringByRegex(const std::string& input, const char& pattern) {
    std::string line = input;
    std::vector<std::string> result;
    int count = getRegexCount(line, pattern);

    int i = 0;
    while (i < count)
    {
        int index = line.find(pattern);
        std::string cutout = line.substr(0, index);
        line = line.substr(index + 1);
        result.push_back(cutout);
        i++;
    }

    result.push_back(line);
    return result;
}

int InitialStateReader::getRegexCount(const std::string &line, const char &regex){
    int count = 0;
    for(char c : line)
        if(c == regex)
            count++;
    return count;
}