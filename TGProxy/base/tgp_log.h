//
// Created by hongchao1 on 2018/1/11.
//

#ifndef TGPROXYSERVER_TGP_LOG_H
#define TGPROXYSERVER_TGP_LOG_H

#include <string>

namespace TGProxy {

class TGProxyGLog {
public:
    TGProxyGLog(char *program);

    ~TGProxyGLog();

    void setLogDetails(std::string log_dir, bool logtostderr);
};

}
#endif //TGPROXYSERVER_TGP_LOG_H
