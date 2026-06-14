#pragma once 
#ifndef TEMPERATURE 
#define TEMPERATURE 1.3
#endif 
#ifndef NEXTTOKEN_MARKOV_CHAIN
#define NEXTTOKEN_MARKOV_CHAIN
#include <unordered_map> 
#include <vector>
#include <string>
#include <unicode/unistr.h>
#include "header/trie.h"
#include "header/string_utils.h"
class NextTokenMarkovChain {
    public:
        NextTokenMarkovChain();
        ~NextTokenMarkovChain();
        void set_corpus(const icu::UnicodeString& corpus);
        std::vector<icu::UnicodeString> get_suggestions(icu::UnicodeString text, bool new_word);
    private:

        void fill_bigram_vocabulary(const icu::UnicodeString& first_word, const icu::UnicodeString& second_word, const icu::UnicodeString& next_word);
        void fill_onegram_vocabulary(const icu::UnicodeString& start, const icu::UnicodeString& next);


        std::vector<icu::UnicodeString> get_suggestions_for_text(const icu::UnicodeString& previous_previous_word,const icu::UnicodeString& previous_word);
        Trie vocabulary;        
    std::unordered_map<icu::UnicodeString,std::unordered_map<icu::UnicodeString,int,UnicodeStringHash,UnicodeStringEq>,UnicodeStringHash,UnicodeStringEq> bigram_vocabulary;
    std::unordered_map<icu::UnicodeString,std::unordered_map<icu::UnicodeString,int,UnicodeStringHash,UnicodeStringEq>,UnicodeStringHash,UnicodeStringEq> onegram_vocabulary;

};
#endif