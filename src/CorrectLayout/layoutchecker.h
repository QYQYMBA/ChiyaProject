#ifndef LAYOUTCHECKER_H
#define LAYOUTCHECKER_H

#include <QString>

#include <vector>
#include <map>
#include <windows.h>
#include <string>

typedef std::map<int, QChar> VkToChar;
typedef std::map<QString, int> Words;
typedef std::pair<std::pair<VkToChar, VkToChar>, Words> Dictionary;
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

    QString getAlphabet(HKL layout);
    QChar vkToChar(HKL layout, int vk, bool shift);
    int charToVk(HKL layout, QChar c, bool* shift);
    void generateEdits(const QString& word, std::vector<QString>& result, const HKL layout);
    bool findPrefix(QString word, const HKL layout);
    int findWord(const QString& word, const HKL layout);
};

#endif // LAYOUTCHECKER_H
