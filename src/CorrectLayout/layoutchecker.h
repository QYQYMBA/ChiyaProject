#ifndef LAYOUTCHECKER_H
#define LAYOUTCHECKER_H

#include <vector>
#include <map>
#include <windows.h>
#include <string>

typedef std::map<std::string, int> Words;
typedef std::pair<std::string, Words> Dictionary;
typedef std::map<HKL, Dictionary> Dictionaries;

class LayoutChecker
{
public:
    void load(const std::string filename, const HKL layout);
    HKL checkLayout(const std::string& word, const bool finished);
    HKL identifyLayout(const std::string& word);
    void changeWordLayout(std::string& word, const HKL layout);
private:
    Dictionaries _dictionaries;

    void generateEdits(const std::string& word, std::vector<std::string>& result, const HKL layout);
    bool findPrefix(const std::string& word, const HKL layout);
    int findWord(const std::string& word, const HKL layout);
};

#endif // LAYOUTCHECKER_H
