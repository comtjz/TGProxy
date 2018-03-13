//
// Created by hongchao1 on 2018/1/10.
//

#include "tgp_context.h"

namespace TGProxy {

TGPContext::TGPContext() {
    conf_ = new(TGProxyConf);

    msConfFile_ = new(TGProxyMSConfFile);
}

}