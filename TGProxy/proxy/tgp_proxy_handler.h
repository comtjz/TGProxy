//
// Created by hongchao1 on 2018/1/10.
//

#ifndef TGPROXY_PROXY_HANDLER_H
#define TGPROXY_PROXY_HANDLER_H

#include <chrono>
#include <proxygen/httpserver/RequestHandler.h>

namespace proxygen {
class ResponseHandler;
}

namespace TGProxy {

class TGProxyStats;

class TGProxyHandler : public proxygen::RequestHandler {
public:
    explicit TGProxyHandler(TGProxyStats* stats, const std::string& model_name, const std::string& model_ver);
    virtual ~TGProxyHandler();

    void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

    void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

    void onEOM() noexcept override;

    void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

    void requestComplete() noexcept override;

    void onError(proxygen::ProxygenError err) noexcept override;

private:
    bool tgpExpress(std::unique_ptr<folly::IOBuf> body, std::string &reply, std::string &out);

private:
    TGProxyStats* const stats_{nullptr};

    std::unique_ptr<folly::IOBuf> body_;

    std::chrono::time_point<std::chrono::system_clock> start_;
    std::chrono::time_point<std::chrono::system_clock> end_;

    std::string model_name_;
    std::string model_ver_;
};
}

#endif //TGPROXY_PROXY_HANDLER_H
