//
// Created by hongchao1 on 2018/1/17.
//

#ifndef TGPROXYSERVER_TGP_UTILS_H
#define TGPROXYSERVER_TGP_UTILS_H

#include <string>

namespace TGProxy {
    void split(const std::string& s, std::string& delim, std::vector<std::string>& ret);
    /**
     * 获取本机内网IP
     */
    int getLocalIp(std::string& outip);

    bool isLAN(const std::string& ipstring);
}

#endif //TGPROXYSERVER_TGP_UTILS_H
