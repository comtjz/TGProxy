//
// Created by hongchao1 on 2018/1/10.
//

#include <folly/io/IOBuf.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "tgp_proxy_stats.h"
#include "tgp_proxy_handler.h"
#include "../postman/tgp_postman.h"
#include "../base/tgp_context.h"

using namespace proxygen;

namespace TGProxy {

    TGProxyHandler::TGProxyHandler(TGProxyStats *stats, const std::string &model_version, const std::string &model_ver)
            : stats_(stats), model_name_(model_version), model_ver_(model_ver) {
        start_ = std::chrono::system_clock::now();
    }

    TGProxyHandler::~TGProxyHandler() {
        end_ = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_);
        LOG(INFO) << "cost time: " << duration.count();
    }

    // 收到请求头
    void TGProxyHandler::onRequest(std::unique_ptr<HTTPMessage> header) noexcept {
        LOG(INFO) << "TGProxyHandler onRequest";
        LOG(INFO) << "TGProxyHandler client address: " << header->getClientIP() << ":" << header->getClientPort();
        LOG(INFO) << "TGProxyHandler Http Method: " << header->getMethodString();

        stats_->recoredRequest();
    }

    // 收到实体
    void TGProxyHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
        LOG(INFO) << "TGProxyHandler onBody";
        if (body_) {
            body_->prependChain(std::move(body));
        } else {
            body_ = std::move(body);
        }
    }

    // 请求接收完毕
    void TGProxyHandler::onEOM() noexcept {
        LOG(INFO) << "TGProxyHandler onEOM";

        std::string reply;
        std::string out;
        auto start = std::chrono::system_clock::now();
        bool ret = tgpExpress(std::move(body_), reply, out);
        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        LOG(INFO) << "TGProxyHandler express, cost time: " << duration.count();
        if (!ret) {
            reply = "{\"weights\":[], \"reason\":\"" + out + "\"}";
        }

        std::unique_ptr<folly::IOBuf> response_body = folly::IOBuf::copyBuffer(reply);
        ResponseBuilder(downstream_)
            .status(200, "OK")
            .body(std::move(response_body))
            .sendWithEOM();
    }

    void TGProxyHandler::onUpgrade(UpgradeProtocol /*protocol*/) noexcept {
        // handler doesn't support upgrades
        LOG(INFO) << "TGProxyHandler onUpgrade";
    }

    void TGProxyHandler::requestComplete() noexcept {
        LOG(INFO) << "TGProxyHandler requestComplete";
        delete this;
    }

    void TGProxyHandler::onError(ProxygenError /*err*/) noexcept {
        LOG(ERROR) << "TGProxyHandler onError";
        delete this;
    }

    bool TGProxyHandler::tgpExpress(std::unique_ptr<folly::IOBuf> body, std::string& reply, std::string& out) {
        LOG(INFO) << "TGProxyHandler start express";

        if (body == nullptr || body->empty()) {
            LOG(ERROR) << "TGP Express empty request body";
            out = "please check, request body is empty";
            return false;
        }

        std::string request = body->moveToFbString().toStdString();
        if (request.empty()) {
            LOG(ERROR) << "TGP Express empty request body";
            out = "please check, request body is empty";
            return false;
        }

        // TODO ranges 需要充zookeeper的解析来吗?
        int ranges = TGProxy::TGPContext::get()->msConf()->get_ms_parts();
        TGProxy::TGProxyPostman tgpPostman(ranges, model_name_, model_ver_);

        auto start = std::chrono::system_clock::now();
        if (!tgpPostman.checkRequest(request, out)) {
            LOG(ERROR) << "TGP Express, check request fail, out:" << out;
            return false;
        }
        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        LOG(INFO) << "TGP Postman checkRequest, cost time: " << duration.count();

        start = std::chrono::system_clock::now();
        if (!tgpPostman.tranferRequest(out)) {
            LOG(ERROR) << "TGP Express, tranfer request fail, out:" << out;
            return false;
        }
        end = std::chrono::system_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        LOG(INFO) << "TGP Postman transferRequest, cost time: " << duration.count();

        start = std::chrono::system_clock::now();
        if (!tgpPostman.answerRequest(reply, out)) {
            LOG(ERROR) << "TGP Express, answer request fail, out:" << out;
            return false;
        }
        end = std::chrono::system_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        LOG(INFO) << "TGP Postman answerRequest, cost time: " << duration.count();

        return true;
    }

}

