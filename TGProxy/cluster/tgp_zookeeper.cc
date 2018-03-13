//
// Created by hongchao1 on 2018/1/19.
//

#include <sys/stat.h>
#include <glog/logging.h>

#include "../utils/tgp_const.h"

#include "tgp_zookeeper.h"
#include "tgp_cluster_admin.h"
#include "tgp_zk_watcher.h"

#define ZK_GET_RETRIES    3

ZK::ZK(const void *ctx) : zzh_(nullptr), zk_addr_(""), ctx_(ctx), zoo_log_fp_(nullptr) {
}

ZK::~ZK() {
    if (zzh_) {
        zookeeper_close(zzh_);
        zzh_ = NULL;
    }

    if (zoo_log_fp_ != NULL) {
        fclose(zoo_log_fp_);
        zoo_log_fp_ = NULL;
    }
}

void ZK::set_zk_address(const std::string& addr) {
    if (addr.empty()) {
        LOG(ERROR) << "set empty zookeeper address";
        return;
    }

    zk_addr_ = addr;
}

/*
 * 打开zookeeper服务
 */
int ZK::start_zk(void *ctx) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    if (zzh_) {
        LOG(ERROR) << "zookeeper is running, can't init";
        return -1;
    }

    zzh_ = zookeeper_init(zk_addr_.c_str(), TGProxy::TGPWatcher::zk_watcher, 10000, 0, ctx, 0);
    if (!zzh_) {
        LOG(ERROR) << "zookeeper connection not established.";
        return -1;
    }

    int count = 0;
    int state = 0;
    while (state != ZOO_CONNECTED_STATE) {
        ::sleep(1);
        state = zoo_state(zzh_);
        LOG(INFO) << "wait zookeeper connect success, state = " << state2String(state);
        count++;
        if (state != ZOO_CONNECTED_STATE && count > 5) {
            LOG(ERROR) << "Wait 5s, zookeeper connection maybe occur an error";
            return -1;
        }
    }

    return 0;
}

void ZK::finalize_zk() {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    if (zzh_) {
        zookeeper_close(zzh_);
        zzh_ = NULL;
    }

    return;
}

void ZK::init_zoo_log(const string &log_dir, const string &zoo_log) {
    if (log_dir.empty() || zoo_log.empty()) {
        LOG(WARNING) << "empty zk_log";
        return;
    }

    string log_path = log_dir + "/" + zoo_log;
    umask(0);
    zoo_log_fp_ = fopen(log_path.c_str(), "a+");
    if (NULL == zoo_log_fp_) {
        LOG(ERROR) << "failed to open zoo log file:" << log_path << ", errno:" << errno;
        return;
    }

    zoo_set_log_stream(zoo_log_fp_);
    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
    return;
}

// zookeeper是否连接正常
bool ZK::is_connected() {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    if (NULL == zzh_) {
        LOG(ERROR) << "zookeeper handle is null";
        return false;
    }

    int state = zoo_state(zzh_);
    if (state != ZOO_CONNECTED_STATE) {
        LOG(ERROR) << "zookeeper handle in wrong state, state = " << state2String(state);
        return false;
    }

    return true;
}

bool ZK::check_path_exist(const std::string& path) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    if (NULL == zzh_) {
        LOG(ERROR) << "zookeeper handle is null";
        return false;
    }

    Stat stat;
    // TODO 同步检查
    int zrc = zoo_exists(zzh_, path.c_str(), 0, &stat);
    if (zrc != ZOK) {
        LOG(ERROR) << "zoo_exists failed, error is " << zerror(zrc);
        return false;
    }

    return true;
}

bool ZK::w_check_path_exists(const std::string& path, watcher_fn watcher) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    int zrc = 0;
    if (NULL == zzh_) {
        LOG(ERROR) << "zookeeper handle is null";
        return false;
    }

    Stat stat;
    zrc = zoo_wexists(zzh_, path.c_str(), watcher, NULL, &stat);
    if (zrc != ZOK) {
        LOG(ERROR) << "zoo_wexists failed, error is " << zerror(zrc);
        return false;
    }

    return true;
}

bool ZK::create_node(const std::string& node_path, bool is_temporary) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    LOG(INFO) << "begin to create a node in zookeeper.";
    LOG(INFO) << "path: " << node_path;

    int zrc = 0;
    int node_flag = 0;
    if (NULL == zzh_) {
        LOG(ERROR) << "zookeeper handle is null";
        return false;
    }

    if (is_temporary) {
        node_flag = ZOO_EPHEMERAL;
    }

    zrc = zoo_create(zzh_, node_path.c_str(), NULL, -1, &ZOO_OPEN_ACL_UNSAFE,
                     node_flag, const_cast<char *>(node_path.c_str()), node_path.length()+1);
    if (zrc != ZOK && zrc != ZNODEEXISTS) {
        LOG(ERROR) << "ERROR happens when create the node: " << node_path;
        LOG(ERROR) << "the error is " << zerror(zrc);
        return false;
    }

    LOG(INFO) << "the node is created successfully.";
    return true;
}

bool ZK::create_node_with_value(const std::string& node_path, const std::string& value, bool is_temporary) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    LOG(INFO) << "begin to create a node with value in zookeeper";
    LOG(INFO) << "path: " << node_path;
    LOG(INFO) << "value: " << value;

    if (NULL == zzh_) {
        LOG(ERROR) << "zookeeper handle is null";
        return false;
    }

    int node_flag = 0;
    if (is_temporary) {
        node_flag = ZOO_EPHEMERAL;
    }

    int zrc = 0;
    zrc = zoo_create(zzh_, node_path.c_str(), value.c_str(),
                     value.length(), &ZOO_OPEN_ACL_UNSAFE, node_flag,
                     const_cast<char *>(node_path.c_str()), node_path.length() + 1);
    if (ZNODEEXISTS == zrc || ZOK == zrc) {
        zrc = set_node_value(node_path, value);
        if (zrc == 0) {
            return true;
        }
    } else {
        LOG(ERROR) << "Error happens when create the node: " << node_path;
        LOG(ERROR) << "the error is " << zerror(zrc);
        return false;
    }

    LOG(INFO) << "the node with node is created successfully.";

    return true;
}

int ZK::set_node_value(const std::string& path, const std::string& value) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    int zrc = 0;
    if (NULL == zzh_) {
        LOG(ERROR) << "zookeeper handle is null";
        return -1;
    }

    zrc = zoo_set(zzh_, path.c_str(), const_cast<char *>(value.c_str()), value.length(), -1);
    if (zrc != ZOK) {
        LOG(ERROR) << "set nodef fail, path:" << path.c_str() << " " << zerror(zrc);
        return -1;
    }

    return 0;
}

int ZK::get_node_value(const std::string& path, std::string *value) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    value->clear();

    int zrc = 0;
    if (NULL == zzh_) {
        LOG(ERROR) << "zookeeper handle is null";
        return -1;
    }

    char buffer[1024] = "\0";
    int buflen = 1024;
    Stat stat;
    zrc = zoo_get(zzh_, path.c_str(), 0, buffer, &buflen, &stat);
    if (zrc != ZOK) {
        LOG(ERROR) << "zoo_get failed, the error is " << zerror(zrc);
        return -1;
    }
    // TODO 节点无数据, 空

    *value = buffer;
    return 0;
}

int ZK::w_get_node_value(const std::string& path, watcher_fn watcher, std::string* value) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    value->clear();

    int zrc = 0;
    if (NULL == zzh_) {
        LOG(ERROR) << "zookeeper handle is null";
        return -1;
    }

    Stat stat;
    char buffer[1024] = "";
    int buflen = 1024;
    zrc = zoo_wget(zzh_, path.c_str(), watcher, NULL, buffer, &buflen, &stat);
    if (zrc != ZOK) {
        LOG(ERROR) << "zoo_wget failed, the error is " << zerror(zrc);
        return -1;
    }

    *value = buffer;
    return 0;
}

int ZK::children_node_cmp(const void *p1, const void *p2) {
    char **s1 = (char **)p1;
    char **s2 = (char **)p2;

    return strcmp(*s1, *s2);
}

/**
 * 从zookeeper获得path路径下的节点
 * @param path: 父节点路径
 * @param setWatcher: 是否设置回调watcher。非零,设置。(全局watcher)
 * @param nodes: 回参,返回path路劲下的节点名。
 */
int ZK::zk_get_chdnodes(const std::string& path, int setWatcher, String_vector &nodes) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    if (path.empty()) {
        LOG(ERROR) << "get chd nodes fail, empty path";
        return TGP_ERR_PARAM;
    }

    int ret;
    for (int i = 0; i < ZK_GET_RETRIES; ++i) {
        ret = zoo_get_children(zzh_, path.c_str(), setWatcher, &nodes);
        switch (ret) {
            case ZOK:
                qsort(nodes.data, nodes.count, sizeof(char *), children_node_cmp);
                return TGP_OK; // 成功
            case ZNONODE:
                return ZK_NODE_NOT_EXIST;
            case ZINVALIDSTATE:
            case ZMARSHALLINGERROR:
                continue;
            default:
                LOG(ERROR) << "Failed to call zoo_get_children. err:" << zerror(ret) << ". path:" << path;
                return TGP_ERR_ZOO_FAILED;
        }
    }

    LOG(ERROR) << "Failed to call zoo_get_children after retry. err:" << zerror(ret) << ". path:" << path;
    return TGP_ERR_ZOO_FAILED;
}

/**
 * 从zookeeper获得path路径下的节点,并对该路径设置特定的watcher。
 * @param path: 父节点路径
 * @param watcher: 回调函数
 * @param watcherCtx: 回调函数参数
 * @param nodes: 回参,返回path路劲下的节点名。
 */
int ZK::zk_wget_chdnodes(const std::string& path, watcher_fn watcher, void *watcherCtx, String_vector &nodes) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    if (path.empty()) {
        LOG(ERROR) << "get chd nodes fail, empty path";
        return TGP_ERR_PARAM;
    }

    int ret;
    for (int i = 0; i < ZK_GET_RETRIES; ++i) {
        ret = zoo_wget_children(zzh_, path.c_str(), watcher, watcherCtx, &nodes);
        switch (ret) {
            case ZOK:
                qsort(nodes.data, nodes.count, sizeof(char *), children_node_cmp);
                return TGP_OK;
            case ZNONODE:
                return ZK_NODE_NOT_EXIST;
            case ZINVALIDSTATE:
            case ZMARSHALLINGERROR:
                continue;
            default:
                LOG(ERROR) << "Failed to call zoo_get_children. err:" << zerror(ret) << ". path:" << path;
                return TGP_ERR_ZOO_FAILED;
        }
    }

    LOG(ERROR) << "Failed to call zoo_get_children after retry. err:" << zerror(ret) << ". path:" << path;
    return TGP_ERR_ZOO_FAILED;
}

/**
 * 获取path路径下的子节点。( TODO 感觉函数不适合放在zookeeper了,不是特别底层,以后修改)
 * @param path: 父节点路径
 * @param results: 回参,子节点节点名
 */
int ZK::get_chdnodes(const std::string& path, watcher_fn watcher, void *watcherCtx, std::list<std::string> &results) {
    int rc;
    Stat stat;
    String_vector children;

    rc = zk_wget_chdnodes(path, watcher, watcherCtx, children);
    if (rc != TGP_OK) {
        LOG(ERROR) << "zookeeper get children fail";
        return rc;
    }

    std::list<std::string> t_result;
    for (int i = 0; i < children.count; ++i) {
        std::string node = children.data[i];

        if (node.empty() || node.find(":") == std::string::npos) {
            // 简单判断
            LOG(ERROR) << "node name is wrong";
            continue;
        }

        t_result.push_back(node);
    }

    deallocate_String_vector(&children);

    if (!t_result.empty()) {
        results.assign(t_result.begin(), t_result.end());
    }

    return TGP_OK;
}

#if 0
int ZK::get_children_nodes(const std::string& path, std::map<std::string, std::string> *results) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    int rc = 0;
    results->clear();
    if (NULL == zzh_) {
        LOG(ERROR) << "zookeeper handle is null";
        return -1;
    }

    Stat stat;
    String_vector children;
    rc = zoo_get_children(zzh_, path.c_str(), 0, &children);
    if (rc != ZOK) {
        LOG(ERROR) << "error happen when zoo_get_children: " << zerror(rc);
        return -1;
    }

    for (int i = 0; i < children.count; ++i) {
        std::string child_path = path + "/" + children.data[i];
        std::string value;

        char buffer[1024] = "";
        int buflen = sizeof(buffer);
        rc = zoo_get(zzh_, child_path.c_str(), 0, buffer, &buflen, &stat);
        if (rc != ZOK) {
            LOG(ERROR) << "error happen when zoo_get: " << zerror(rc);
            continue;
        }

        value = buffer;
        results->insert(make_pair(children.data[i], value));
    }

    rc = children.count;
    deallocate_String_vector(&children);

    return rc;
}

int ZK::w_get_children_nodes(const std::string& path, watcher_fn watcher, void *watcherCtx, std::map<std::string, std::string> *results) {
    std::lock_guard<std::recursive_mutex> lck(mtx_);

    int rc = 0;
    results->clear();

    if (NULL == zzh_) {
        LOG(ERROR) << "zookeeper handle is null";
        return -1;
    }

    Stat stat;
    String_vector children;
    rc = zoo_wget_children(zzh_, path.c_str(), watcher, watcherCtx, &children);
    if (rc != ZOK) {
        LOG(ERROR) << "error happen when zoo_wget_children: " << zerror(rc);
        return -1;
    }

    for (int i = 0; i < children.count; ++i) {
        std::string child_path = path + "/" + children.data[i];
        std::string value;

        char buffer[1024] = "";
        int buflen = sizeof(buffer);
        rc = zoo_get(zzh_, child_path.c_str(), 0, buffer, &buflen, &stat);
        if (rc != ZOK) {
            LOG(ERROR) << "error happen when zoo_get: " << zerror(rc);
            continue;
        }

        value = buffer;
        results->insert(make_pair(children.data[i], value));
    }

    rc = children.count;
    deallocate_String_vector(&children);

    return 0;
}
#endif

const char* ZK::state2String(int state) {
    if (state == 0) {
        return "CLOSED_STATE";
    }
    if (state == ZOO_CONNECTING_STATE) {
        return "CONNECTING_STATE";
    }
    if (state == ZOO_ASSOCIATING_STATE) {
        return "ASSOCIATING_STATE";
    }
    if (state == ZOO_CONNECTED_STATE) {
        return "CONNECTED_STATE";
    }
    if (state == ZOO_EXPIRED_SESSION_STATE) {
        return "EXPIRED_SESSION_STATE";
    }
    if (state == ZOO_AUTH_FAILED_STATE) {
        return "AUTH_FAILED_STATE";
    }

    return "INVALID_STATE";
}

const char* ZK::type2String(int type) {
    if (type == ZOO_CREATED_EVENT) {
        return "CREATED_EVENT";
    }
    if (type == ZOO_DELETED_EVENT) {
        return "DELETED_EVENT";
    }
    if (type == ZOO_CHANGED_EVENT) {
        return "CHANGED_EVENT";
    }
    if (type == ZOO_CHILD_EVENT) {
        return "CHILD_EVENT";
    }
    if (type == ZOO_SESSION_EVENT) {
        return "SESSION_EVENT";
    }
    if (type == ZOO_NOTWATCHING_EVENT) {
        return "NOTWATCHING_EVENT";
    }

    return "UNKNOWN_EVENT_TYPE";
}
