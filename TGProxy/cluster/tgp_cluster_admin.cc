//
// Created by hongchao1 on 2018/1/19.
//

#include <glog/logging.h>
#include <zookeeper/zookeeper.h>

#include "../utils/tgp_const.h"

#include "tgp_cluster_admin.h"
#include "tgp_zk_watcher.h"
#include "../base/tgp_context.h"

namespace TGProxy {

    void tg_parent_watcher(zhandle_t* zh, int type, int state, const char *path, void *watcherCtx) {
        LOG(INFO) << "event happend ======>";
        LOG(INFO) << "event: node [" << path << "]";
        LOG(INFO) << "event: state [" << state << "]";
        LOG(INFO) << "event: type [" << type << "]";
        LOG(INFO) << "event done <======";

        int ret = 0;
        struct Stat stat;

        // TODO 设置失败
        ret = zoo_wexists(zh, path, tg_parent_watcher, watcherCtx, &stat);
        if (ret) {
            LOG(ERROR) << "zoo wexists fail, the error is " << zerror(ret);
        }
    }

    bool TGPClusterAdmin::startZkProcess() {
        LOG(INFO) << "INIT ZOOKEEPER LOG";
        const string zk_log_dir  = TGPContext::get()->conf()->get_zk_log_dir();
        const string zk_log_name = TGPContext::get()->conf()->get_zk_log_name();
        zk_.init_zoo_log(zk_log_dir, zk_log_name);

        LOG(INFO) << "Cluster Admin, start zookeeper";

        string zk_addr = TGPContext::get()->conf()->get_zk_address();
        if (zk_addr.empty()) {
            LOG(ERROR) << "empty zk address";
            return false;
        }

        zk_.set_zk_address(zk_addr);
        if (zk_.start_zk(this) != 0) {
            LOG(ERROR) << "start zookeeper fail";
            return false;
        }

        return true;
    }

    void TGPClusterAdmin::stopZkProcess() {
        LOG(INFO) << "Cluster Admin, stop zookeeper";
        zk_.finalize_zk();
    }

    void TGPClusterAdmin::loopUpdateModelServerList() {
        while (true) {
            LOG(INFO) << "start update model server list in loop";
            std::this_thread::sleep_for(std::chrono::seconds(600)); // 十分钟
            if (zk_.is_connected()) {
                LOG(INFO) << "zookeeper connect state is normal";
                if (updateModelServerList()) {
                    LOG(INFO) << "tgp cluster admin: update model server list success";
                } else {
                    LOG(ERROR) << "tgp cluster admin: update model server list fail";
                }
            } else {
                LOG(ERROR) << "zookeeper connect state in problem";
                stopZkProcess();
                startZkProcess();
            }
        }

        return;
    }

    bool TGPClusterAdmin::updateModelServerList() {
        LOG(INFO) << "start update model server list";

        string ms_path = TGPContext::get()->conf()->get_ms_zk_path();

        // 加载ms_path下的所有子节点数据
        list<string> childrens;
        if (zk_.get_chdnodes(ms_path, TGPWatcher::tgpChildrenWatcher, this, childrens) != TGP_OK) {
            LOG(ERROR) << "update model server fail";
            return false;
        }

        if (childrens.empty()) {
            LOG(ERROR) << "update model server fail, empty nodes";
            return false;
        }

        LOG(INFO) << "update model server:";
        for (list<string>::iterator it = childrens.begin(); it != childrens.end(); ++it) {
            LOG(INFO) << "\t" << *it;
        }

        // TODO 这里没有判断内存中的集群信息和本次更新是否一致

        // 准备修改
        std::lock_guard<std::mutex> lck(mtx_);
        ms_cluster_.assign(childrens.begin(), childrens.end());

        return true;
    }

    string TGPClusterAdmin::getOneMsAddr(int partition) {
        std::lock_guard<std::mutex> lck(mtx_);
        if (ms_cluster_.empty()) {
            return "";
        }

        size_t pos = ++count_ % ms_cluster_.size();

        return ms_cluster_[pos];
    }
}