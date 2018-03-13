//
// Created by hongchao1 on 2018/1/19.
//

#ifndef TGPROXYSERVER_TGP_ZOOKEEPR_H
#define TGPROXYSERVER_TGP_ZOOKEEPR_H

#include <map>
#include <list>
#include <mutex>  // std::mutex
#include <string>
#include <libgen.h>

#include <zookeeper/zookeeper.h>

class ZK {
public:
    ZK(const void *ctx = NULL);
    virtual ~ZK();

    // 设置zookeeper集群地址
    void set_zk_address(const std::string& addr);
    void set_outter(void *outter) { outter_ = outter;}
    void *get_outter() { return outter_; }

    int start_zk(void *);
    void finalize_zk();

    void init_zoo_log(const std::string &log_dir, const std::string &zoo_log);

    bool is_connected();

    bool check_path_exist(const std::string& path);
    bool w_check_path_exists(const std::string& path, watcher_fn watcher);

    bool create_node(const std::string& node_path, bool is_temporary);
    bool create_node_with_value(const std::string& node_path, const std::string& value, bool is_temporary);

    int set_node_value(const std::string& path, const std::string& value);
    int get_node_value(const std::string& path, std::string *value);
    int w_get_node_value(const std::string& path, watcher_fn watcher, std::string* value);

    int get_chdnodes(const std::string& path, watcher_fn watcher, void *watcherCtx, std::list<std::string> &results);

    static const char *state2String(int state);
    static const char *type2String(int type);

private:
    static int children_node_cmp(const void *p1, const void *p2);

    int zk_get_chdnodes(const std::string& path, int setWatcher, String_vector& nodes);
    int zk_wget_chdnodes(const std::string& path, watcher_fn watcher, void *watcherCtx, String_vector& nodes);

public:
    //std::mutex mtx_;
    std::recursive_mutex mtx_;

    std::string zk_addr_;
    zhandle_t * zzh_;

    const void *ctx_; // 外部设置

    void *outter_;

    FILE *zoo_log_fp_;
};

#endif //TGPROXYSERVER_TGP_ZOOKEEPR_H
