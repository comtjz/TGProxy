/*
 * c_api.h
 *
 * Created on: 2017年11月17日
 * Author: zhujun
 * package tiangan-ps-v1
 * @Weibo @半饱半醉
 */

#ifndef INCLUDE_TIANGAN_C_API_H_
#define INCLUDE_TIANGAN_C_API_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef void* RangeSlicer;

uint64_t tiangan_hash_feature(const void * key, int len);

void tiangan_get_mpath(char buffer[], int buffer_len,
                       const char *model_name, const char *model_version,
                       int partition, int timestamp, const char* delim);

//RangeSlicer api
RangeSlicer tiangan_range_slicer_new(int range);

void tiangan_range_slicer_slice(RangeSlicer slicer, uint64_t *arr, int len, int *pos);

void tiangan_range_slicer_destroy(RangeSlicer *slicer);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif /* INCLUDE_TIANGAN_C_API_H_ */
