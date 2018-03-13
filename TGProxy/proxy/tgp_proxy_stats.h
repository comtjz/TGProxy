//
// Created by hongchao1 on 2018/1/10.
//

#ifndef TGPROXY_TG_PROXY_STATS_H
#define TGPROXY_TG_PROXY_STATS_H

#include <cstdint>

namespace TGProxy {

/**
 * 代理请求的统计计数
 */
class TGProxyStats {
public:
    virtual ~TGProxyStats() {
    }

    void recoredRequest() {
        ++reqCount_;
    }

    uint64_t getRequestCount() {
        return reqCount_;
    }

private:
    uint64_t reqCount_{0};
};
}

#endif //TGPROXY_TG_PROXY_STATS_H
