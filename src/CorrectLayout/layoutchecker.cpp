#include "layoutchecker.h"

#include <QDebug>
#include <QFile>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <regex>

#include "winapiadapter.h"

void LayoutChecker::load(const QString filename, const HKL layout)
{
    int vkCodes[] = {VK_SPACE,192,49,50,51,52,53,54,55,56,57,48,189,187,81,87,69,82,84,89,85,73,79,80,219,221,220,65,83,68,70,71,72,74,75,76,186,222,90,88,67,86,66,78,77,188,190,191};

    Words* words = &_dictionaries[layout].second;
    struct stat buffer;

    if (stat(filename.toStdString().c_str(), &buffer) != 0) {
        _dictionaries[layout].second = *words;
        qDebug() << "Can't find" << filename << "!";
        return;
    }

    QFile inputFile(filename);

    if (!inputFile.open(QFile::ReadOnly | QFile::Text)) return;
    QTextStream in(&inputFile);

    QString noShift = in.readLine();
    for (int i = 0; i < sizeof(vkCodes)/sizeof(int); i++){
        _dictionaries[layout].first.first[vkCodes[i]] = noShift[i];
        possibleKeys[vkCodes[i]]++;
    }

    QString shift = in.readLine();
    for (int i = 0; i < sizeof(vkCodes)/sizeof(int); i++){
        _dictionaries[layout].first.second[vkCodes[i]] = shift[i];
    }

    while (!in.atEnd())
    {
        QString line = in.readLine();
        (*words)[line]++;
    }

    qDebug() << "Log: " << "For this layout alphabet is " << getAlphabet(layout);
}

void LayoutChecker::generateEdits(const QString& word, std::vector<QString>& result, const HKL layout)
{
    for (std::string::size_type i = 0; i < word.size(); i++)
    {
        result.push_back(word.mid(0, i) + word.mid(i + 1)); //deletions
    }
    for (std::string::size_type i = 0; i < word.size() - 1; i++)
    {
        result.push_back(word.mid(0, i) + word[i + 1] + word[i] + word.mid(i + 2)); //transposition
    }
    QString alphabet = getAlphabet(layout);
    for(size_t i = 0; i < sizeof(alphabet); i++)
    {
        QChar letter = alphabet[i];
        for (std::string::size_type i = 0; i < word.size(); i++)
        {
            result.push_back(word.mid(0, i) + letter + word.mid(i + 1)); //alterations
        }
        for (std::string::size_type i = 0; i < word.size() + 1; i++)
        {
            result.push_back(word.mid(0, i) + letter + word.mid(i)); //insertion

        }
    }
}

HKL LayoutChecker::checkLayout(const QString& word, const bool finished) {
    QString alphabet = "";

    std::vector<QString> edits;
    edits.clear();

    QString translatedWord = word.toLower();

    for (auto it = _dictionaries.begin(); it != _dictionaries.end(); it++) {
        alphabet += getAlphabet(it->first);
    }

    for (int i = 0; i < translatedWord.size(); i++)
    {
        if (alphabet.indexOf(translatedWord[i]) == -1)
        {
            translatedWord[i] = '-';
        }
    }

    const HKL layout = identifyLayout(translatedWord);

    if (layout == nullptr) {
        return nullptr;
    }

    if (finished) {
        changeWordLayout(translatedWord, layout);

        if (findWord(translatedWord, layout)) {
            return layout;
        }

        for (auto it = _dictionaries.begin(); it != _dictionaries.end(); it++) {
            if (it->first == layout) {
                continue;
            }

            translatedWord = word.toLower();

            changeWordLayout(translatedWord, it->first);

            if (findWord(translatedWord, it->first)) {
                return it->first;
            }
        }
    }

    changeWordLayout(translatedWord, layout);

    if (findPrefix(translatedWord, layout)) {
        return layout;
    }

    if (finished) {
        generateEdits(translatedWord, edits, layout);

        for (std::size_t i = 0; i < edits.size(); i++) {
            if (findPrefix(edits[i], layout)) {
                return layout;
            }
        }
    }

    for (auto it = _dictionaries.begin(); it != _dictionaries.end(); it++) {
        if (it->first == layout) {
            continue;
        }

        translatedWord = word.toLower();

        changeWordLayout(translatedWord, it->first);

        if (findPrefix(translatedWord, it->first)) {
            return it->first;
        }

        if (finished) {
            generateEdits(translatedWord, edits, it->first);

            for (std::size_t i = 0; i < edits.size(); i++) {
                if (findPrefix(edits[i], it->first)) {
                    return it->first;
                }
            }
        }
    }

    return layout;
}

HKL LayoutChecker::identifyLayout(const QString& word) {
    for (auto it = _dictionaries.begin(); it != _dictionaries.end(); it++)
    {
        QString alphabet = getAlphabet(it->first);
        bool currentAlphabet = true;
        for (std::size_t i = 0; i < word.length(); i++) {
            if (alphabet.indexOf(word[i]) == std::string::npos)
            {
                currentAlphabet = false;
                break;
            }
        }
        if(currentAlphabet)
            return it->first;
    }
    for (auto it = _dictionaries.begin(); it != _dictionaries.end(); it++)
    {
        QString alphabet = getAlphabet(it->first);
        for (std::size_t i = 0; i < word.length(); i++) {
            if (alphabet.indexOf(word[i]) != std::string::npos)
            {
                return it->first;
            }
        }
    }
    return nullptr;
}

void LayoutChecker::changeWordLayout(QString& word, const HKL layout)
{
  const QString oldWord = word;
  HKL currentLayout = identifyLayout(oldWord);
  word = "";
  bool shift = false;
  for (std::string::size_type i = 0; i < oldWord.length(); i++) {
      int c = charToVk(currentLayout, oldWord[i], &shift);
      word += vkToChar(layout, c, shift);
  }
}

bool LayoutChecker::isKeyInDictionary(int vkCode)
{
    auto it = possibleKeys.find(vkCode);
    return (it != possibleKeys.end());
}

QString LayoutChecker::getAlphabet(HKL layout)
{
    QString alphabet = "";
    VkToChar vkToChar = _dictionaries[layout].first.first;
    for (auto it = vkToChar.begin(); it != vkToChar.end(); it++)
    {
        alphabet += it->second;
    }
    return alphabet;
}

QChar LayoutChecker::vkToChar(HKL layout, int vk, bool shift)
{
    if (!shift)
        return _dictionaries[layout].first.first[vk];
    else
        return _dictionaries[layout].first.second[vk];
}

int LayoutChecker::charToVk(HKL layout, QChar c, bool* shift)
{
    *shift = false;
    VkToChar vkToChar = _dictionaries[layout].first.first;
    for (auto it = vkToChar.begin(); it != vkToChar.end(); it++)
    {
        if (it->second == c)
            return it->first;
    }
    vkToChar = _dictionaries[layout].first.second;
    for (auto it = vkToChar.begin(); it != vkToChar.end(); it++)
    {
        if (it->second == c)
        {
            *shift = true;
            return it->first;
        }
    }
    return -1;
}

bool LayoutChecker::findPrefix(QString word, const HKL layout) {
    Dictionary* dictionary = &(_dictionaries.find(layout)->second);
    Words *words = &(dictionary->second);

    Words::const_iterator i = (*words).lower_bound(word);
    if (i != (*words).end()) {
        QString key = i->first;
        key = key.mid(0, word.length());
        if (key == word)
        {
            return true;
        }
        i++;
    }

    return false;
}

int LayoutChecker::findWord(const QString& word, const HKL layout) {
    Dictionary* dictionary = &(_dictionaries.find(layout)->second);
    Words* words = &(dictionary->second);

    Words::const_iterator i = (*words).find(word);
    if (i != (*words).end()) {
        return i->second;
    }

    return 0;
}
