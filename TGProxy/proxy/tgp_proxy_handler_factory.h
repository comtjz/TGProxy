//
// Created by hongchao1 on 2018/1/10.
//

#ifndef TGPROXY_TGP_PROXY_HANDLER_FACTORY_H
#define TGPROXY_TGP_PROXY_HANDLER_FACTORY_H

#include "tgp_proxy_stats.h"
#include "tgp_proxy_handler.h"
#include "tgp_wrong_handler.h"

#include "../utils/tgp_utils.h"

namespace TGProxy {

class TGProxyHandlerFactory : public RequestHandlerFactory {
public:
    void onServerStart(folly::EventBase* /*evb*/) noexcept override {
        LOG(INFO) << "Server Start";
        stats_.reset(new TGProxyStats);
    }

    void onServerStop() noexcept override {
        LOG(INFO) << "Server Stop";
        stats_.reset();
    }

    // TODO 根据路径分配不同的Handler
    RequestHandler* onRequest(RequestHandler*, HTTPMessage* msg) noexcept override {
        LOG(INFO) << "Handler Factory Request";
        LOG(INFO) << "Handler Factory, httpVersion:" << msg->getVersionString();
        LOG(INFO) << "Handler Factory, url:" << msg->getURL();

        const std::string& url = msg->getURL();
        if (url == "/") {
            // 访问根路径,返回失败
            LOG(INFO) << "Handler Factory, Root Path";
            return new TGPWrongHandler(url);
        }

        std::vector<std::string> url_terms;
        std::string delim = "/";
        TGProxy::split(url, delim, url_terms); // url_terms[0] 为空

        if (url_terms.size() == 3) { // 目前只支持 /model_name/model_ver
            std::string model_name = url_terms[1];
            std::string model_ver  = url_terms[2];

            if (model_name.empty() || model_ver.empty()) {
                LOG(ERROR) << "Handler Factory, Unknown Path";
                return new TGPWrongHandler(url);
            }

            return new TGProxyHandler(stats_.get(), model_name, model_ver);
        } else {
            LOG(INFO) << "Handler Factory, Unknown Path";
            return new TGPWrongHandler(url);
        }
    }

private:
    folly::ThreadLocalPtr<TGProxyStats> stats_;
};

}

#endif //TGPROXY_TGP_PROXY_HANDLER_FACTORY_H
