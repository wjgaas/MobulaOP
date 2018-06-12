#ifndef _MOBULA_FUNC_
#define _MOBULA_FUNC_

#include "defines.h"
#include "context/context.h"

namespace mobula {


template <typename T, typename UNARY_FUNC>
MOBULA_KERNEL unary_kernel(const int n, const T *a, T *out, UNARY_FUNC func) {
    parfor(n, [&](int i) {
        out[i] = func(a[i]);
    });
}

template <typename T, typename BINARY_FUNC>
MOBULA_KERNEL binary_kernel(const int n, const T *a, const T *b, T *out, BINARY_FUNC func) {
    parfor(n, [&](int i) {
        out[i] = func(a[i], b[i]);
    });
}

/*
 * out[i,j,k,m] = sum(a[i,j,:] * b[k, :, m])
 * for u = 0...n - 1
 *    out[i, k, m] += a[i, u] * b[k, u, m]
 */
template <typename T>
MOBULA_KERNEL dot_kernel(const int n, const T *a, const T *b, const int U, const int K, const int M, T *out) {
    parfor(n, [&](int index) {
        const int i = index / (K * M);
        const int k = (index / M) % K;
        const int m = index % M;
        out[index] = 0;
        for (int u = 0; u < U; ++u) {
            out[index] += a[i * U + u] * b[(k * U + u) * M + m];
        }
    });
}

template <typename T>
MOBULA_KERNEL assign_carray_kernel(const int n, const T *a, T *out) {
    parfor(n, [&](int index) {
        out[index] = a[index];
    });
}

template <typename T>
MOBULA_KERNEL assign_val_kernel(const int n, const T val, T *out) {
    parfor(n, [&](int i) {
        out[i] = val;
    });
}


}

extern "C" {
using namespace mobula;

#define REGISTER_UNARY_FUNC(func_name, func) \
    using T = DType;\
    void func_name(const int _n, const T *_a, T *_out) {\
        auto _func = func;\
        KERNEL_RUN((unary_kernel<T, decltype(_func)>), _n)(_n, _a, _out, _func);\
    } \

#define REGISTER_BINARY_FUNC(func_name, func) \
    void func_name(const int _n, const DType *_a, const DType *_b, DType *_out) {\
        auto _func = func;\
        KERNEL_RUN((binary_kernel<DType, decltype(_func)>), _n)(_n, _a, _b, _out, _func);\
    } \

REGISTER_UNARY_FUNC(abs_, [](const DType &a){return abs(a);})

REGISTER_BINARY_FUNC(add, [](const DType &a, const DType &b){return a + b;})
REGISTER_BINARY_FUNC(sub, [](const DType &a, const DType &b){return a - b;})
REGISTER_BINARY_FUNC(mul, [](const DType &a, const DType &b){return a * b;})
REGISTER_BINARY_FUNC(div_, [](const DType &a, const DType &b){return a / b;})

void dot(const DType *a, const DType *b, const int I, const int U, const int K, const int M, DType *out) {
    const int N = I * K * M;
    KERNEL_RUN(dot_kernel<DType>, N)(N, a, b, U, K, M, out);
}

void print_carray(CArray<DType> ca) {
    bool first = true;
    for (int i = 0; i < ca.size; ++i) {
        if (!first) std::cout << ", ";
        first = false;
        std::cout << ca.data[i];
    }
    std::cout << std::endl;
}

void assign_carray(CArray<DType> a, DType *out) {
    const int N = a.size;
    auto sp = ctx_pointer<DType>(N, a.data);
    sp.set_ctx(CTX::DEVICE);
    const DType *pa = sp.pointer();
    KERNEL_RUN(assign_carray_kernel<DType>, N)(N, pa, out);
}

void assign_val(const int n, const int val, DType *out) {
    KERNEL_RUN(assign_val_kernel<DType>, n)(n, val, out);
}

void sum(const int n, CArray<DType*> a, DType *out) {
    const int num_vars = a.size;
    assign_val(n, 0, out);
    for (int i = 0; i < num_vars; ++i) {
        DType *e = a.data[i];
        add(n, e, out, out);
    }
}

}

#endif