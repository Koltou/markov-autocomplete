#include "header/trie.h"


Trie::Trie() { 
    this->root = std::make_shared<TrieNode>();
}
Trie::~Trie() {
}

void Trie::add_word(const icu::UnicodeString& str) {
    std::pair<std::weak_ptr<TrieNode>,int32_t> raw_data = this->existant_advance(str);
    std::weak_ptr<TrieNode> end_ptr = raw_data.first;
    int32_t end_i = raw_data.second;
    auto locked = end_ptr.lock();
    if(locked->suggestion != NULL) {
        locked->suggestion->freq++;
        return;
    }
    UChar32 c;
    for(; end_i<str.length();) {
        c =str.char32At(end_i); 
        end_i  +=U16_LENGTH(c);
        locked->childrens[c] = std::make_shared<TrieNode>();
        if(end_i>=str.length()) {
            locked->childrens[c]->suggestion = std::make_shared<Suggestion>();
            locked->childrens[c]->suggestion->word=str;
        }
        end_ptr = locked->childrens[c];
        locked = end_ptr.lock();
    }


}
std::vector<icu::UnicodeString> Trie::get_suggestions(const icu::UnicodeString& prefix) {
    
    std::weak_ptr<TrieNode> ptr = this->advance(prefix);
    if(ptr.expired()) {
        std::string std_string;
        prefix.toUTF8String(std_string);
        throw std::runtime_error("Couldn't find '" + std_string + "' in memory.");
    }
    std::vector<std::weak_ptr<TrieNode>> leaves={};
    this->get_leaves(ptr,leaves);
    //make the pmf 
    std::vector<double> weights ={};
    std::vector<std::weak_ptr<Suggestion>> suggestions = {};
    for(int i = 0; i < leaves.size();i++) {
        auto locked = leaves[i].lock();
        suggestions.push_back(  locked->suggestion);
        double p_x = (double)locked->suggestion->freq; 
        p_x = std::pow(p_x,1.0/TEMPERATURE);
        weights.push_back(p_x);
    }


    std::vector<icu::UnicodeString> res;
    while(res.size() < 5 && !weights.empty()) {
        std::random_device rng;
        std::discrete_distribution<> pmf(weights.begin(),weights.end());
        unsigned int picked_number = pmf(rng);
        weights.erase(weights.begin() + picked_number);
        res.push_back(suggestions[picked_number].lock()->word);
        suggestions.erase(suggestions.begin()+picked_number);
    }
    return res;

}

void Trie::get_leaves(std::weak_ptr<TrieNode> ptr, std::vector<std::weak_ptr<TrieNode>>& res) {
    auto locked = ptr.lock();
    if(locked->suggestion!=nullptr){
        res.push_back(locked);
    }
    for(const auto& [key,value]:locked->childrens) {
        this->get_leaves(value,res);
    }    
}

std::weak_ptr<TrieNode> Trie::advance(const icu::UnicodeString&str) {
    std::weak_ptr<TrieNode> ptr = this->root;
    int32_t i = 0;
    for(; i< str.length();)  {
        UChar32 c = str.char32At(i);
        auto locked = ptr.lock();
        if(locked->childrens.find(c) == locked->childrens.end()) {
            ptr.reset();
            return ptr;
        }
        ptr = locked->childrens.find(c)->second;
        i+= U16_LENGTH(c);
    }
    return ptr;
}

std::pair<std::weak_ptr<TrieNode>,int32_t> Trie::existant_advance(const icu::UnicodeString& str) {
    std::weak_ptr<TrieNode> ptr = this->root;
    int32_t i = 0;
    for(; i< str.length();)  {
        UChar32 c = str.char32At(i);
        auto locked = ptr.lock();

        auto it = locked->childrens.find(c);
        if(it == locked->childrens.end()) {
            break;
        }
        ptr = it->second;

        i+= U16_LENGTH(c);
    }
    return std::make_pair(ptr,i);
}