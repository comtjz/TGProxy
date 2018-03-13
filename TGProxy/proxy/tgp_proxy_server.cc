//
// Created by hongchao1 on 2018/1/10.
//

#include <proxygen/httpserver/HTTPServer.h>
#include <folly/SocketAddress.h>

#include "tgp_proxy_server.h"
#include "tgp_proxy_handler_factory.h"

namespace TGProxy {

bool TGProxyServer::start() {
    if (IPs_.empty()) {
        LOG(ERROR) << "TGP proxy, not set addr and protocol";
        return false;
    }

    if (thread_num_ <= 0) {
        thread_num_ = sysconf(_SC_NPROCESSORS_ONLN);
        CHECK(thread_num_ > 0);
    }

    HTTPServerOptions options;
    options.threads     = static_cast<size_t>(thread_num_);
    options.idleTimeout = std::chrono::milliseconds(60000);
    options.shutdownOn  = {SIGINT, SIGTERM};
    options.enableContentCompression = false;
    options.handlerFactories = RequestHandlerChain()
        .addThen<TGProxyHandlerFactory>()
        .build();

    proxygen::HTTPServer server(std::move(options));
    server.bind(IPs_);

    // Start HTTPServer mainloop in a separate thread
    std::thread t([&] () {
        server.start();
    });

    t.join();

    return true;
}

void TGProxyServer::setHTTPServerIPs(int32_t port, HTTPServer::Protocol protocol) {
    // 获取本机内网IP
    std::string ip;
    TGProxy::getLocalIp(ip);

    // Protocol 是枚举类
    HTTPServer::IPConfig ip_conf = {folly::SocketAddress(ip, port, true), protocol};
    IPs_.push_back(ip_conf);
}

}