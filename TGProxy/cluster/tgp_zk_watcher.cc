//
// Created by hongchao1 on 2018/1/23.
//

#include <glog/logging.h>

#include "tgp_zk_watcher.h"
#include "tgp_cluster_admin.h"

namespace TGProxy {
    void TGPWatcher::tgpChildrenWatcher(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx) {
        LOG(INFO) << "ZK Watcher: children watcher";
        LOG(INFO) << "type = " << ZK::type2String(type) << ", state = " << ZK::state2String(state);
        TGPClusterAdmin * admin = (TGPClusterAdmin*)watcherCtx;
        admin->updateModelServerList();

        return;
    }

    void TGPWatcher::zk_watcher(zhandle_t *zh, int type, int state, const char* path, void *watcherCtx) {
        LOG(INFO) << "zk watcher, recv enent: type = " << ZK::type2String(type) << ", state = " << ZK::state2String(state) << ", path = " << path;

        if (NULL == watcherCtx) {
            LOG(ERROR) << "zk watcher, empty watcher ctx";
            return;
        }

        TGPClusterAdmin *admin = (TGPClusterAdmin *)(watcherCtx);
        ZK* zk = admin->getZK();
        std::lock_guard<std::recursive_mutex> lck(zk->mtx_); // 递归锁
        if (zh != zk->zzh_) {
            LOG(ERROR) << "zk watcher, zhandle not matcher";
            return;
        }

        if (type == ZOO_SESSION_EVENT && state == ZOO_CONNECTED_STATE) {
            LOG(INFO) << "build connection ok";

            admin->updateModelServerList();
        } else if (type == ZOO_SESSION_EVENT && state == ZOO_EXPIRED_SESSION_STATE) {
            LOG(INFO) << "connection disconnect";
            // TODO 重连zookeeper
            zk->finalize_zk();
            zk->start_zk(watcherCtx);
            LOG(INFO) << "reconnect";
        }
        LOG(INFO) << "leave zk watcher";
    }
}