//
// Created by hongchao1 on 2018/1/10.
//

#ifndef TGPROXY_TGP_CONTEXT_H
#define TGPROXY_TGP_CONTEXT_H

#include "tgp_conf.h"
#include "tgp_ms_conf.h"

namespace TGProxy {

class TGPContext {
public:
    static TGPContext* get() {
        static TGPContext c;
        return &c;
    }

    virtual ~TGPContext() {
        if (conf_) {
            delete(conf_);
            conf_ = NULL;
        }

        if (msConfFile_) {
            delete(msConfFile_);
            msConfFile_ = NULL;
        }
    };

    TGProxyConf* conf() {
        return conf_;
    }

    TGProxyMSConfFile* msConf() {
        return msConfFile_;
    }

private:
    TGProxyConf* conf_;
    TGProxyMSConfFile* msConfFile_;

private:
    TGPContext();

    //DISALLOW_COPY_AND_ASSIGN(TGPContext);
};
}

#endif //TGPROXY_TGP_CONTEXT_H
