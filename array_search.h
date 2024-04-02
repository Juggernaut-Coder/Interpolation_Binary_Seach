#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef int (*cmp_fn_t)(const void *restrict, const void *restrict);

static inline int compare_int(const void *restrict a, const void *restrict b) { //compare_int is provided as an example
    const int arg1 = *(const int*)a;
    const int arg2 = *(const int*)b;
    return (arg1 > arg2) - (arg1 < arg2);
}

typedef long long (*subtract_fn_t)(const void *restrict, const void *restrict);

static inline long long subtract_int(const void *restrict a, const void *restrict b) { //subtract_int is provided as an example // a > b if positive answer is expected -> a - b
    return (long long)((*(const int *restrict)a - *(const int *restrict)b));
}

static inline size_t interpolate(const void *restrict val, const void *restrict data, const size_t sz, const size_t left, const size_t right, subtract_fn_t subtract) {
    long long fraction = subtract(val, (char *)data + (left * sz)) / subtract((const char *restrict)data + (right * sz), (char *)data + (left * sz));
    return (fraction >= 0) ? left + ((right - left) * (size_t)fraction) : (left - ((right -left) * (size_t)(-fraction)));
}

static inline void update_ib_search_bounds(const void *restrict val, const void *restrict data, const size_t sz, size_t *restrict interpolation, size_t *restrict left, size_t *restrict right, cmp_fn_t cmp) {
    const char *restrict dataBytes = (const char *restrict)data;
    size_t mid;
    if(cmp(val, dataBytes + (*interpolation) * sz) > 0) {
        (*interpolation)++;
        if(cmp(val, dataBytes + (*interpolation) * sz) <= 0) {
            *left = *right = *interpolation;
            return;
        }
        mid = (*interpolation + *right) / 2;
        *left = (cmp(val, dataBytes + (mid * sz)) <= 0) ? ((*right = mid), (*interpolation + 1)) : (mid + 1);        
    } else {
        size_t lookIdx = *interpolation - 1;
        if(*interpolation == *left || cmp(val, dataBytes + (lookIdx) * sz) > 0) {
            *left = *right = *interpolation;
            return;
        }
        mid = (*interpolation + *left) / 2;
        *right = (cmp(val, dataBytes + (mid * sz)) >= 0) ? ((*left = mid), lookIdx) : (mid - 1);
    }
}

static inline bool ibs_valIsInArray(const void *const restrict val, const void *restrict data, const size_t sz, size_t left, const size_t data_len, size_t *restrict idx, subtract_fn_t subtract, cmp_fn_t cmp) {
    if(data_len == 0 || cmp(val, (char *)data + (left * sz)) < 0) {
        *idx = left;
        return false;
    }
    size_t right = data_len - 1;
    if(cmp(val, (char *)data + (right * sz)) > 0) {
        *idx = data_len;
        return false;
    }
    while(left < right) {
        *idx = interpolate(val, data, sz, left, right, subtract);
        update_ib_search_bounds(val, data, sz, idx, &left, &right, cmp);
    }
    return cmp(val, (char *)data + (left * sz)) == 0;
}

static inline bool sq_valIsInArray(const void *restrict val, const void *restrict data, const size_t sz, const size_t left, const size_t data_len, size_t *idx, cmp_fn_t cmp) {
    *idx = left;
    while (*idx < data_len && cmp(val, (char *)data + ((*idx) * sz)) > 0) {
        (*idx)++;
    }
    return (*idx < data_len) && (cmp(val, (char *)data + ((*idx) * sz)) == 0);
}