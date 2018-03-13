//
// Created by hongchao1 on 2018/1/10.
//

#include <iostream>

#include "tgp_conf.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/ini_parser.hpp"

namespace TGProxy {

bool TGProxyConf::read_conf_file(std::string file) {
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(file, pt);

    try {
        /* 代理 */
        http_port_       = pt.get<int32_t>("PROXYSERVER.http_port");
        proxy_threadnum_ = pt.get<int32_t>("PROXYSERVER.proxy_threadnum");

        /* model server */
        ms_config_file_ = pt.get<string>("MODELSERVER.ms_config");

        /* zookeeper 地址 */
        ms_zk_path_        = pt.get<string>("ZOOKEEPER.ms_zk_path");
        zookeeper_address_ = pt.get<string>("ZOOKEEPER.zk_addr");
        zk_log_dir_        = pt.get<string>("ZOOKEEPER.zk_log_dir");
        zk_log_name_       = pt.get<string>("ZOOKEEPER.zk_log_name");

        /* 日志 */
        logtostderr_ = pt.get<bool>("LOG.logtostderr");
        log_dir_     = pt.get<std::string>("LOG.log_dir");
    } catch (std::exception e) {
        std::cerr << "read config: exception[" << e.what() << "]" << std::endl;
        return false;
    }

    return true;
}

}
