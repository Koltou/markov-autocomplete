#pragma once 
#ifndef TEMPERATURE 
#define TEMPERATURE 1.3
#endif 
#ifndef TRIE_H
#define TRIE_H

#include <vector>
#include <unicode/unistr.h>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <random>
struct Suggestion {
    icu::UnicodeString word;
    unsigned int freq=1;
};
struct TrieNode {
    std::shared_ptr<Suggestion> suggestion=nullptr; //if not a leaf undefined
    std::unordered_map<UChar32,std::shared_ptr<TrieNode>> childrens;
};


class  Trie {
    public:
        Trie();
        ~Trie();
        void add_word(const icu::UnicodeString& str);
        std::vector<icu::UnicodeString> get_suggestions(const icu::UnicodeString& prefix);

    private:

        std::pair<std::weak_ptr<TrieNode>,int32_t> existant_advance(const icu::UnicodeString& str);
        std::weak_ptr<TrieNode> advance(const icu::UnicodeString& str);
        void get_leaves(std::weak_ptr<TrieNode> ptr,std::vector<std::weak_ptr<TrieNode>>& res);

        std::shared_ptr<TrieNode> root;
};

#endif 