//
// Created by hongchao1 on 2018/1/10.
//

#ifndef TGPROXY_TGP_CONF_H
#define TGPROXY_TGP_CONF_H

#include <string>

using namespace std;

namespace TGProxy {

class TGProxyConf {
public:
    bool read_conf_file(std::string file);

    int32_t get_http_port() { return http_port_; }
    int32_t get_proxy_threadnum() { return proxy_threadnum_; }

    string get_ms_config_file() { return ms_config_file_; }

    bool get_logtostderr() { return logtostderr_; }
    string get_logdir() { return log_dir_; }

    //string get_partition() {return partition_; }
    //string get_local_addr() { return local_addr_; }
    string get_ms_zk_path() { return ms_zk_path_; }
    string get_zk_address() { return zookeeper_address_; }
    string get_zk_log_dir() { return zk_log_dir_; }
    string get_zk_log_name() { return zk_log_name_; }

private:
    /* 代理服务 */
    int32_t http_port_; // 代理服务 端口
    int32_t proxy_threadnum_; // 代理服务 线程数。如果<=0,使用CPU核心数

    /* zookeeper相关配置 */
    string ms_zk_path_;
    string zookeeper_address_;
    string zk_log_dir_;
    string zk_log_name_;

    /* modelserver */
    string   ms_config_file_;

    /* 日志 */
    bool     logtostderr_;
    string   log_dir_;
};

}



#endif //TGPROXY_TGP_CONF_H
