#ifndef LAYOUTCHECKER_H
#define LAYOUTCHECKER_H

#include <QString>

#include <vector>
#include <map>
#include <windows.h>
#include <string>
#include "layoutcontroller.h"

typedef std::map<int, QChar> VkToChar;
typedef std::map<QString, int> Words;
typedef std::pair<std::pair<VkToChar, VkToChar>, Words> Dictionary;
typedef std::map<HKL, Dictionary> Dictionaries;
typedef std::map<HKL, bool> Activations;

class LayoutChecker
{
public:
    LayoutChecker(LayoutController* lc);

    bool load(const QString filename, const HKL layout, const bool activated = true);
    HKL checkLayout(const QString& word, const bool finished, HKL currentLayout = 0);
    HKL identifyLayout(const QString& word);
    void changeWordLayout(QString& word, const HKL layout);
    bool isKeyInDictionary(int vkCode);
    QChar vkToChar(HKL layout, int vk, bool shift);
    int charToVk(HKL layout, QChar c, bool* shift);
private:
    Dictionaries _dictionaries;
    Activations _activations;
    std::map<int, int> _possibleKeys;
    LayoutController* _layoutController;
    std::vector<HKL> _layoutsList;

    QString getAlphabet(HKL layout);
    void generateEdits(const QString& word, std::vector<QString>& result, const HKL layout);
    bool findPrefix(QString word, const HKL layout);
    int findWord(const QString& word, const HKL layout);
};

#endif // LAYOUTCHECKER_H
