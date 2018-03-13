#include <iostream>
#include <string>
#include <getopt.h>

#include "base/tgp_context.h"
#include "base/tgp_log.h"
#include "proxy/tgp_proxy_server.h"
#include "postman/tgp_postman.h"
#include "cluster/tgp_cluster_admin.h"

#include "utils/tgp_utils.h"

/* 入口 */
int main(int argc, char* argv[]) {
    int opt;
    std::string conf_file;
    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c':
                conf_file = optarg;
                break;
            default:
                break;
        }
    }

    if (conf_file.empty()) {
        std::cerr << "Usage: " << argv[0] << " -c config_file" << std::endl;
        return -1;
    }

    TGProxy::TGPContext* context = TGProxy::TGPContext::get();
    if (!context->conf()->read_conf_file(conf_file)) {
        std::cerr << "Wrong Config" << std::endl;
        return -1;
    }

    TGProxy::TGProxyGLog tgpLog(argv[0]);
    tgpLog.setLogDetails(context->conf()->get_logdir(), context->conf()->get_logtostderr());

    LOG(INFO) << "============== TGP ZOOKEEPER START ===============";

    // 打开连接zookeeper线程
    if (!TGProxy::TGPClusterAdmin::getInstance()->startZkProcess()) {
        LOG(ERROR) << "zookeeper connect fail";
        return -1;
    }

    LOG(INFO) << "============== TGP PROXY START ==============";

    std::string ms_conf_file = context->conf()->get_ms_config_file();
    if (ms_conf_file.empty()) {
        std::cerr << "Empty MS Confile" << std::endl;
        LOG(ERROR) << "Empyt MS Config";
        return -1;

    }

    if (!context->msConf()->load_ms_conf_file(ms_conf_file)) {
        std::cerr << "Wrong MS Config" << std::endl;
        LOG(ERROR) << "Wrong MS Config";
        return -1;
    }

    int32_t http_port = 11000;
    if (context->conf()->get_http_port() > 0) {
        http_port = context->conf()->get_http_port();
    }

    TGProxy::TGProxyServer tgpProxyServer(context->conf()->get_proxy_threadnum());
    tgpProxyServer.setHTTPServerIPs(http_port,
                                    proxygen::HTTPServer::Protocol::HTTP);

    LOG(INFO) << "TGProxy Server Start";
    tgpProxyServer.start();

    LOG(INFO) << "TGProxy Server Stop";

    TGProxy::TGPClusterAdmin::getInstance()->stopZkProcess();
    return 0;
}