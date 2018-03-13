/*
 * c_api.cc
 *
 * Created on: 2017年11月17日
 * Author: zhujun
 * package tiangan-ps-v1
 * @Weibo @半饱半醉
 */

#include <cstring>
#include "tiangan.h"
#include "c_api.h"
#include "mpath.h"
#include "slicer.h"

using TGRangeSlicer = tiangan::RangeSlicer<tiangan::Key>;

uint64_t tiangan_hash_feature(const void * key, int len) {
    return tiangan::hash_feature(key, len);
}

void tiangan_get_mpath(char buffer[], int buffer_len,
                       const char *model_name, const char *model_version,
                       int partition, int timestamp, const char* delim) {
    std::string mpath;
    memset(buffer, 0, buffer_len);
    mpath = tiangan::get_mpath(model_name, model_version, partition, timestamp, delim);
    strncpy(buffer, mpath.c_str(), buffer_len);
}

RangeSlicer tiangan_range_slicer_new(int range) {
    return new TGRangeSlicer(range);
}

void tiangan_range_slicer_slice(RangeSlicer slicer, uint64_t *arr, int len, int *pos) {
    TGRangeSlicer *s = reinterpret_cast<TGRangeSlicer*>(slicer);
    s->slice(arr, arr + len, pos);
}

void tiangan_range_slicer_destroy(RangeSlicer *slicer) {
    TGRangeSlicer *s = reinterpret_cast<TGRangeSlicer*>(*slicer);
    delete s;
    *slicer = nullptr;
}
