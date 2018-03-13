//
// Created by hongchao1 on 2018/1/10.
//

#ifndef TGPROXY_TGP_PROXY_SERVER_H
#define TGPROXY_TGP_PROXY_SERVER_H

#include <string>

#include <proxygen/httpserver/HTTPServer.h>

using namespace proxygen;

namespace TGProxy {

class TGProxyServer {
public:
    TGProxyServer(int thread_num = 8) : thread_num_(thread_num)  {}
    virtual ~TGProxyServer() {}

    /**
     * 设置代理服务
     * @param port     端口
     * @param protocol 代理协议
     *
     * 自动选择本机内网IP
     */
    void setHTTPServerIPs(int32_t port, HTTPServer::Protocol protocol);

    bool start();

private:
    int thread_num_;
    std::vector<HTTPServer::IPConfig> IPs_;
};

}

#endif //TGPROXY_TGP_PROXY_SERVER_H
