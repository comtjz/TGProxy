/*
 * mpath.h
 *
 * Created on: 2017年11月16日
 * Author: zhujun
 * package tiangan-ps-v1
 * @Weibo @半饱半醉
 */

#ifndef INCLUDE_TIANGAN_MPATH_H_
#define INCLUDE_TIANGAN_MPATH_H_

#include <string>
#include <tuple>
#include "tiangan.h"
#include "murmur_hash3.h"

namespace tiangan {

    inline uint32 hash_u32(const void * key, int len, uint32 seed) {
        uint32 hash = 0;
        MurmurHash3_x86_32(key, len, seed, reinterpret_cast<void*>(&hash));
        return hash;
    }

    inline uint64 hash_u64(const void * key, int len, uint32 seed) {
        uint64 hash[2];
        MurmurHash3_x64_128(key, len, seed, reinterpret_cast<void*>(hash));
        return hash[0];
    }

    inline tiangan::Key hash_feature(const void * key, int len) {
        static const unsigned int hash_seed = 141652681;
        return hash_u64(key, len, hash_seed);
    }

    const static char default_mpath_delim = '-';

    inline std::string get_mpath(const std::string& model_name,
                                 const std::string& model_version,
                                 int partition,
                                 int timestamp = 0,
                                 const char* delim = nullptr) {
        std::string mpath_delim;
        if(delim)
            mpath_delim = delim;
        else
            mpath_delim = default_mpath_delim;

        std::string mpath;
        mpath = model_name;
        mpath += mpath_delim;
        mpath += model_version;
        mpath += mpath_delim;
        mpath += std::to_string(partition);
        if(timestamp > 0) {
            mpath += mpath_delim;
            mpath += std::to_string(timestamp);
        }
        return mpath;
    }

    inline std::tuple<std::string, std::string, int, int> parse_mpath(const std::string& mpath, const char* delim = nullptr) {
        auto sections = std::tuple<std::string, std::string, int, int>("", "", 0, 0);

        return sections;
    }

}

#endif /* INCLUDE_TIANGAN_MPATH_H_ */
