#ifndef INITIALSTATEREADER_H
#define INITIALSTATEREADER_H

#include <string>

class InitialStateReader {
    public:
        InitialStateReader(const std::string &filename);
        std::vector<std::string> getLines();
        std::vector<std::string> splitStringByRegex(const std::string& input, const char& pattern);
        int getRegexCount(const std::string &line, const char &regex);
    private:
        std::string filename;
        std::vector<std::string> lines;
        std::vector<std::string> readFile(const std::string &filename);
};

#endif // INITIALSTATEREADER_H