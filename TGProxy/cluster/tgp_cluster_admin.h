//
// Created by hongchao1 on 2018/1/19.
//

#ifndef TGPROXYSERVER_TGP_CLUSTER_ADMIN_H
#define TGPROXYSERVER_TGP_CLUSTER_ADMIN_H

#include <string>
#include <vector>
#include <thread>
#include <functional>

#include <pthread.h>

#include "tgp_zookeeper.h"

using std::string;
using std::vector;
using std::thread;

namespace TGProxy {
    class TGPClusterAdmin {
    public:
        ~TGPClusterAdmin() {
        }

        static TGPClusterAdmin* getInstance() {
            static TGPClusterAdmin clusterAdmin;
            return &clusterAdmin;
        }

        // 开始/关闭 zookeeper 连接
        bool startZkProcess();
        void stopZkProcess();

        void loopUpdateModelServerList();
        bool updateModelServerList();

        string getOneMsAddr(int partition);

        ZK* getZK() { return &zk_; }

    private:
        TGPClusterAdmin() : zk_(this), t_(std::bind(&TGPClusterAdmin::loopUpdateModelServerList, this)), count_(0) {
            t_.detach();
        }

    private:
        ZK  zk_;

        string local_addr_;

        std::thread t_; // 间歇地注册对应节点的watcher,防止zookeeper的session异常后,watcher消失

        /* 保存model server的地址 */
        vector<string> ms_cluster_;
        uint64_t count_;
        std::mutex mtx_;
    };
}

#endif //TGPROXYSERVER_TGP_CLUSTER_ADMIN_H
