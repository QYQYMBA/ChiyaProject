#include "layoutchecker.h"

#include <QDebug>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <regex>


#include "winapiadapter.h"
#include "key.h"

void LayoutChecker::load(const std::string filename, const HKL layout)
{
    Words* words = &_dictionaries[layout].second;
    struct stat buffer;
    std::string alphabet = "";

    if (stat(filename.c_str(), &buffer) != 0) {
        _dictionaries[layout].second = *words;
        qDebug() << "Can't find" << QString::fromStdString(filename) << "!";
        return;
    }

    std::ifstream file(filename.c_str(), std::ios_base::binary | std::ios_base::in);

    file.seekg(0, std::ios_base::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios_base::beg);

    std::string data(static_cast<std::size_t>(length), '\0');

    file.read(&data[0], length);

    alphabet = data.substr(0, data.find_first_of('\r\n'));

    //transform(data.begin(), data.end(), data.begin(), filter);
    for(unsigned int i = 0; i < data.size(); i++)
    {
        if (alphabet.find(data[i]) == -1)
        {
            data[i] = '-';
        }
    }

    for (std::string::size_type i = 0; i != std::string::npos;)
    {
        const std::string::size_type firstNonFiltered = data.find_first_not_of('-', i + 1);
        if (firstNonFiltered == std::string::npos) {
            break;
        }

        const std::string::size_type end = data.find('-', firstNonFiltered);
        const std::string word = data.substr(firstNonFiltered, end - firstNonFiltered);
        (*words)[word]++;

        i = end;
    }

    _dictionaries[layout].first = alphabet;
    qDebug() <<  "For this layout alphabet is " << QString::fromStdString(_dictionaries[layout].first);
}

void LayoutChecker::generateEdits(const std::string& word, std::vector<std::string>& result, const HKL layout)
{
    for (std::string::size_type i = 0; i < word.size(); i++)
    {
        result.push_back(word.substr(0, i) + word.substr(i + 1)); //deletions
    }
    for (std::string::size_type i = 0; i < word.size() - 1; i++)
    {
        result.push_back(word.substr(0, i) + word[i + 1] + word[i] + word.substr(i + 2)); //transposition
    }
    for(size_t i = 0; i <  _dictionaries[layout].first.size(); i++)
    {
        char letter = _dictionaries[layout].first[i];
        for (std::string::size_type i = 0; i < word.size(); i++)
        {
            result.push_back(word.substr(0, i) + letter + word.substr(i + 1)); //alterations
        }
        for (std::string::size_type i = 0; i < word.size() + 1; i++)
        {
            result.push_back(word.substr(0, i) + letter + word.substr(i)); //insertion

        }
    }
}

HKL LayoutChecker::checkLayout(const std::string& word, const bool finished) {

    std::string pattern = "(https?:\/\/)?(www\.)?[a-zA-Z0-9@:%._\+~#=]{2,256}\\.[a-z]{2,6}([-a-zA-Z0-9@:%_\+.~#?&//=]*)?";
    std::regex url_regex(pattern);

    std::string alphabet = "";

    std::vector<std::string> edits;
    edits.clear();

    std::string translatedWord = word;

    //transform(translatedWord.begin(), translatedWord.end(), translatedWord.begin(), filter);

    for (auto it = _dictionaries.begin(); it != _dictionaries.end(); it++) {
        alphabet += it->second.first;
    }

    for (int i = 0; i < translatedWord.size(); i++)
    {
        if (alphabet.find(translatedWord[i]) == -1)
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

    if (std::regex_match(translatedWord, url_regex) == true) {
        return layout;
    }

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

        if (std::regex_match(translatedWord, url_regex) == true) {
            return it->first;
        }

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

HKL LayoutChecker::identifyLayout(const std::string& word) {
    for (auto it = _dictionaries.begin(); it != _dictionaries.end(); it++)
    {
        std::string alphabet = it->second.first;
        bool currentAlphabet = true;
        for (std::size_t i = 0; i < word.length(); i++) {
            if (alphabet.find(word[i]) == std::string::npos)
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
        std::string alphabet = it->second.first;
        for (std::size_t i = 0; i < word.length(); i++) {
            if (alphabet.find(word[i]) != std::string::npos)
            {
                return it->first;
            }
        }
    }
    return nullptr;
}

void LayoutChecker::changeWordLayout(std::string& word, const HKL layout)
{
    const std::string oldWord = word;
    HKL currentLayout = identifyLayout(oldWord);
    word = "";
    for (std::string::size_type i = 0; i < oldWord.length(); i++) {
        Key k = Key::CharToKey(oldWord[i], currentLayout);
        word += k.toChar(layout);
    }
    transform(word.begin(), word.end(), word.begin(), ::tolower);
}

bool LayoutChecker::findPrefix(const std::string& word, const HKL layout) {
    Dictionary* dictionary = &(_dictionaries.find(layout)->second);
    Words *words = &(dictionary->second);

    Words::const_iterator i = (*words).lower_bound(word);
    if (i != (*words).end()) {
        const std::string& key = i->first;
        if (key.compare(0, word.size(), word) == 0)
        {
            return true;
        }
    }

    return false;
}

int LayoutChecker::findWord(const std::string& word, const HKL layout) {
    Dictionary* dictionary = &(_dictionaries.find(layout)->second);
    Words* words = &(dictionary->second);

    Words::const_iterator i = (*words).find(word);
    if (i != (*words).end()) {
        return i->second;
    }

    return 0;
}
