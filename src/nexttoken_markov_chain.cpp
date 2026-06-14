#include "header/nexttoken_markov_chain.h"
#include "header/string_utils.h"
NextTokenMarkovChain::NextTokenMarkovChain() {

}

NextTokenMarkovChain::~NextTokenMarkovChain() {

}
void NextTokenMarkovChain::set_corpus(const icu::UnicodeString& corpus) {
    std::vector<icu::UnicodeString> parts = split(corpus,' ');
    for(int i = 0; i<parts.size()-2;i++) {
        this->fill_bigram_vocabulary(parts[i],parts[i+1],parts[i+2]);
        this->fill_onegram_vocabulary(parts[i],parts[i+1]);
        this->vocabulary.add_word(parts[i]);
        if(i==parts.size()-3){
            this->fill_onegram_vocabulary(parts[i+1],parts[i+2]);
            this->vocabulary.add_word(parts[i+1]);
            this->vocabulary.add_word(parts[i+2]);
        }
    }
}

void NextTokenMarkovChain::fill_bigram_vocabulary(const icu::UnicodeString& first_word, const icu::UnicodeString& second_word, const icu::UnicodeString& next_word) {
    icu::UnicodeString starting_pair = first_word+' '+second_word;
    if(this->bigram_vocabulary.find(starting_pair) == this->bigram_vocabulary.end()) {
        this->bigram_vocabulary[starting_pair]={};
    } 
    if(this->bigram_vocabulary[starting_pair].find(next_word) == this->bigram_vocabulary[starting_pair].end()) {
        this->bigram_vocabulary[starting_pair][next_word]=1;
    } else {
        this->bigram_vocabulary[starting_pair][next_word]++;
    }
}

void NextTokenMarkovChain::fill_onegram_vocabulary(const icu::UnicodeString& start, const icu::UnicodeString& next) {
    if(this->onegram_vocabulary.find(start) == this->onegram_vocabulary.end()) {
        this->onegram_vocabulary[start]={};
    }
    if(this->onegram_vocabulary[start].find(next)==this->onegram_vocabulary[start].end()){
        this->onegram_vocabulary[start][next]=1;
    } else {
        this->onegram_vocabulary[start][next]++;
    }
}
std::vector<icu::UnicodeString> NextTokenMarkovChain::get_suggestions(icu::UnicodeString text,bool new_word) {
    if(text.isEmpty()){
        return {};
    }
    std::vector<icu::UnicodeString> words = split(text,' ');    
    if(!new_word) {
        return this->vocabulary.get_suggestions(words[words.size()-1]);
    } else {
        icu::UnicodeString previous_previous_word="", previous_word="";
        previous_word = words[words.size()-1];
        if(words.size()>=2) {
            previous_previous_word=words[words.size()-2];
        }
        return this->get_suggestions_for_text(previous_previous_word,previous_word);
    }
}

std::vector<icu::UnicodeString> NextTokenMarkovChain::get_suggestions_for_text(const icu::UnicodeString& previous_previous_word, const icu::UnicodeString& previous_word) {
    icu::UnicodeString pair = previous_previous_word+' '+previous_word;
    std::vector<icu::UnicodeString> res;
    if(previous_previous_word.isEmpty() || bigram_vocabulary.find(pair) == bigram_vocabulary.end()) {
       
        auto raw_onegram_pdf = this->onegram_vocabulary[previous_word];
        std::vector<icu::UnicodeString> onegram_words; 
        std::vector<double> onegram_weights; 
        for(const auto& [key,value]:raw_onegram_pdf) {
            onegram_words.push_back(key);
            double p_x = (double)value;
            p_x = std::pow(p_x,1.0/TEMPERATURE);
            onegram_weights.push_back(p_x);
        }

        while(res.size() < 5 && onegram_words.size() > 0) {
            std::random_device rng; 
            std::discrete_distribution<> pmf(onegram_weights.begin(),onegram_weights.end());
            unsigned int picked_number = pmf(rng);
            onegram_weights.erase(onegram_weights.begin()+picked_number);
            res.push_back(onegram_words[picked_number]);
            onegram_words.erase(onegram_words.begin()+picked_number); 
        }                



    } else {
        auto raw_bigram_pdf = this->bigram_vocabulary[pair];
        auto raw_onegram_pdf = this->onegram_vocabulary[previous_word];
        
        std::vector<icu::UnicodeString> onegram_words; 
        std::vector<double> onegram_weights; 
        std::vector<icu::UnicodeString> bigram_words; 
        std::vector<double> bigram_weights;
        
        for(const auto& [key,value]:raw_onegram_pdf) {
            onegram_words.push_back(key);
            double p_x = (double)value;
            p_x = std::pow(p_x,1.0/TEMPERATURE);
            onegram_weights.push_back(p_x);
        }

        for(const auto& [key,value]:raw_bigram_pdf) {
            bigram_words.push_back(key);
            double p_x = (double)value;
            p_x = std::pow(p_x,1.0/TEMPERATURE);
            bigram_weights.push_back(p_x);

        }

        while(res.size() < 5 && (onegram_words.size() > 0 || bigram_weights.size() > 0) ) {
            //let's random choose btw

            std::random_device rng; 
            std::uniform_int_distribution<int> pmf(1,2);
            int picked_number =  pmf(rng);
            if( (picked_number ==1 && !onegram_words.empty() ) || bigram_words.empty()) {
                std::random_device rng; 
                std::discrete_distribution<> pmf(onegram_weights.begin(),onegram_weights.end());
                unsigned int picked_number = pmf(rng);
                onegram_weights.erase(onegram_weights.begin()+picked_number);
                res.push_back(onegram_words[picked_number]);
                onegram_words.erase(onegram_words.begin()+picked_number); 
            } else if( (picked_number == 2 && !bigram_words.empty()) || onegram_words.empty() ) {
                std::random_device rng; 
                std::discrete_distribution<> pmf(bigram_weights.begin(),bigram_weights.end());
                unsigned int picked_number = pmf(rng);
                bigram_weights.erase(bigram_weights.begin()+picked_number);
                res.push_back(bigram_words[picked_number]);
                bigram_words.erase(bigram_words.begin()+picked_number); 
            }
        }                
    }
    return res; 
}