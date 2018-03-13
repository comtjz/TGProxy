//
// Created by hongchao1 on 2018/1/16.
//

#ifndef TGPROXYSERVER_TGP_WRONG_HANDLER_H
#define TGPROXYSERVER_TGP_WRONG_HANDLER_H

#include <proxygen/httpserver/RequestHandler.h>

namespace proxygen {
    class ResponseHandler;
}

namespace TGProxy {

    class TGPWrongHandler : public proxygen::RequestHandler {
    public:
        explicit TGPWrongHandler(const std::string& url);

        void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

        void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

        void onEOM() noexcept override;

        void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

        void requestComplete() noexcept override;

        void onError(proxygen::ProxygenError err) noexcept override;

    private:
        std::unique_ptr<folly::IOBuf> body_;

        std::string url_;
    };

}

#endif //TGPROXYSERVER_TGP_WRONG_HANDLER_H
