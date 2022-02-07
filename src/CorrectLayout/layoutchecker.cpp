#include "layoutchecker.h"

#include <QDebug>
#include <QFile>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <regex>

#include "winapiadapter.h"

void LayoutChecker::load(const QString filename, const HKL layout)
{
    Words* words = &_dictionaries[layout].second;
    struct stat buffer;
    QString alphabet = "";

    if (stat(filename.toStdString().c_str(), &buffer) != 0) {
        _dictionaries[layout].second = *words;
        qDebug() << "Can't find" << filename << "!";
        return;
    }

    QFile inputFile(filename);

    if (!inputFile.open(QFile::ReadOnly | QFile::Text)) return;
    QTextStream in(&inputFile);

    alphabet = in.readLine();

    while (!in.atEnd())
    {
        QString line = in.readLine();
        (*words)[line]++;
    }

    _dictionaries[layout].first = alphabet;

    for(QChar c : alphabet)
    {
        KeyPress k = KeyPress::CharToKey(c, layout);
        possibleKeys[k.getVkCode()]++;
    }

    qDebug() << "Log: " << "For this layout alphabet is " << _dictionaries[layout].first;
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
    for(size_t i = 0; i <  _dictionaries[layout].first.size(); i++)
    {
        QChar letter = _dictionaries[layout].first[i];
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

    QString translatedWord = word;

    for (auto it = _dictionaries.begin(); it != _dictionaries.end(); it++) {
        alphabet += it->second.first;
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

            translatedWord = word;

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

        translatedWord = word;

        changeWordLayout(translatedWord, it->first);

        std::string s = translatedWord.toStdString();

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
        QString alphabet = it->second.first;
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
        QString alphabet = it->second.first;
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
  for (std::string::size_type i = 0; i < oldWord.length(); i++) {
      KeyPress k = KeyPress::CharToKey(oldWord[i], currentLayout);
      word += k.toChar(layout);
  }
  word = word.toLower();
}

bool LayoutChecker::isKeyInDictionary(int vkCode)
{
    auto it = possibleKeys.find(vkCode);
    return (it != possibleKeys.end());
}

bool LayoutChecker::findPrefix(const QString& word, const HKL layout) {
    Dictionary* dictionary = &(_dictionaries.find(layout)->second);
    Words *words = &(dictionary->second);

    Words::const_iterator i = (*words).lower_bound(word);
    if (i != (*words).end()) {
        const QString& key = i->first;
        if (key.toStdString().compare(0, word.toStdString().size(), word.toStdString()) == 0)
        {
            return true;
        }
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
