/*
 * str.h
 *
 * Created on: 2017年11月16日
 * Author: zhujun
 * package tiangan-ps-v1
 * @Weibo @半饱半醉
 */

#ifndef INCLUDE_TIANGAN_STR_H_
#define INCLUDE_TIANGAN_STR_H_

#include <string>
#include <algorithm>
#include <cctype>

namespace tiangan {

    inline void trim(std::string& line) {
        line.erase(line.begin(), find_if_not(line.begin(), line.end(), [](const char c){return std::isspace(c);}));
        line.erase(find_if_not(line.rbegin(), line.rend(), [](const char c){return std::isspace(c);}).base(), line.end());
    }

}

#endif /* INCLUDE_TIANGAN_STR_H_ */
