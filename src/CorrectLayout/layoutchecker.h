#ifndef LAYOUTCHECKER_H
#define LAYOUTCHECKER_H

#include <QString>

#include <vector>
#include <map>
#include <windows.h>
#include <string>

#include "keypress.h"

typedef std::map<QString, int> Words;
typedef std::pair<QString, Words> Dictionary;
typedef std::map<HKL, Dictionary> Dictionaries;

class LayoutChecker
{
public:
    void load(const QString filename, const HKL layout);
    HKL checkLayout(const QString& word, const bool finished);
    HKL identifyLayout(const QString& word);
    void changeWordLayout(QString& word, const HKL layout);
    bool isKeyInDictionary(int vkCode);
private:
    Dictionaries _dictionaries;
    std::map<int, int> possibleKeys;

    void generateEdits(const QString& word, std::vector<QString>& result, const HKL layout);
    bool findPrefix(const QString& word, const HKL layout);
    int findWord(const QString& word, const HKL layout);
};

#endif // LAYOUTCHECKER_H
