//
// Created by hongchao1 on 2018/3/13.
//

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "tgp_utils.h"

namespace TGProxy {
    void split(const std::string& s, std::string& delim, std::vector<std::string>& ret) {
        size_t last = 0;
        size_t index = s.find_first_of(delim, last);
        while (index != std::string::npos) {
            ret.push_back(s.substr(last, index - last));
            last = index + 1;
            index = s.find_first_of(delim, last);
        }

        if (index - last > 0) {
            ret.push_back(s.substr(last, index - last));
        }
    }

    /**
     * 获取本机内网IP
     */
    int getLocalIp(std::string& outip) {
        int i = 0;
        int sockfd;
        struct ifconf ifconf;
        char buf[512];
        struct ifreq *ifreq;
        char *ip;

        // 初始化ifconf
        ifconf.ifc_len = 512;
        ifconf.ifc_buf = buf;

        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            return -1;
        }
        ioctl(sockfd, SIOCGIFCONF, &ifconf);
        close(sockfd);

        ifreq = (struct ifreq *)buf;
        for (i = (ifconf.ifc_len / sizeof(struct ifreq)); i > 0; i--) {
            ip = inet_ntoa(((struct sockaddr_in*)&(ifreq[i].ifr_addr))->sin_addr);

            if (strcmp(ip, "127.0.0.1") == 0) {
                ifreq++;
                continue;
            }

            if (isLAN(ip)) {
                outip = ip;
                break;
            }
        }

        return 0;
    }

    /*-----------------------------------------
     * 局域网IP地址范围
     * A类：10.0.0.0-10.255.255.255
     * B类：172.16.0.0-172.31.255.255
     * C类：192.168.0.0-192.168.255.255
    -------------------------------------------*/
    bool isLAN(const std::string& ipstring)
    {
        std::istringstream st(ipstring);
        int ip[2];
        for(int i = 0; i < 2; i++)
        {
            std::string temp;
            getline(st,temp,'.');
            std::istringstream a(temp);
            a >> ip[i];
        }
        if((ip[0]==10) || (ip[0]==172 && ip[1]>=16 && ip[1]<=31) || (ip[0]==192 && ip[1]==168))
            return true;
        else
            return false;
    }
}