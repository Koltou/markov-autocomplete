#pragma once 
#ifndef LINEAR_ALGEBRA_UTILS
#define LINEAR_ALGEBRA_UTILS
#include <Eigen/Dense>

template<typename M>
M m_pow(M base, long long pow) {
    M result = M::Identity(base.rows(),base.cols());
    while( pow > 0 ) {
        if(pow & 1) result = result * base;
        base = base * base;
        pow >>= 1;
    }
    return result;
}

#endif 