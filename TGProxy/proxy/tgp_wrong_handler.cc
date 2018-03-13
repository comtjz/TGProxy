//
// Created by hongchao1 on 2018/1/16.
//

#include <glog/logging.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "tgp_wrong_handler.h"

using namespace proxygen;

namespace TGProxy {
    TGPWrongHandler::TGPWrongHandler(const std::string& url) : url_(url) {
    }

    void TGPWrongHandler::onRequest(std::unique_ptr<HTTPMessage> /*headers*/) noexcept {
        LOG(INFO) << "TGPWrongHandler onRequest";
    }

    void TGPWrongHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
        LOG(INFO) << "TGPWrongHandler onBody";
    }

    void TGPWrongHandler::onEOM() noexcept {
        LOG(INFO) << "TGPWrongHandler onEOM";

        ResponseBuilder(downstream_)
                .status(200, "OK")
                .sendWithEOM();
    }

    void TGPWrongHandler::onUpgrade(UpgradeProtocol /*protocol*/) noexcept {
        LOG(INFO) << "TGPWrongHandler onUpgrade";
    }

    void TGPWrongHandler::requestComplete() noexcept {
        LOG(INFO) << "TGPWrongHandler requestComplete";
        delete this;
    }

    void TGPWrongHandler::onError(ProxygenError /*err*/) noexcept {
        LOG(ERROR) << "TGPWrongHandler onError";
        delete this;
    }
}
