#pragma once 
#ifndef STRING_UTILS
#define STRING_UTILS
#include <unicode/unistr.h>
#include <vector>

inline std::vector<icu::UnicodeString> split(const icu::UnicodeString& str,const char separator) {
    std::vector<icu::UnicodeString> res;
    int32_t start=0;

    while(start <= str.length()) {
        int32_t pos  = str.indexOf((UChar)separator,start);
        if(pos < 0) {
            icu::UnicodeString part = str.tempSubString(start);
            if(!part.isEmpty()) res.push_back(part);
            break;
        }

        icu::UnicodeString part = str.tempSubStringBetween(start,pos);
        if(!part.isEmpty()) res.push_back(part);
        start = pos + 1;
    }
    return res; 
}



struct UnicodeStringHash {
    std::size_t operator()(const icu::UnicodeString& str) const noexcept {
        return static_cast<std::size_t>(str.hashCode());
    }
};

struct UnicodeStringEq {
    bool operator()(const icu::UnicodeString& a, const icu::UnicodeString& b) const noexcept {
        return a==b;
    }
};

#endif 