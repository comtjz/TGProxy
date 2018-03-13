//
// Created by hongchao1 on 2018/2/1.
//

#ifndef TGPROXYSERVER_TGP_ZK_WROKER_H
#define TGPROXYSERVER_TGP_ZK_WROKER_H

namespace TGP {
    class TGPClusterAdmin;

    class TGPZKWorker {
    public:
        TGPZKWorker() {}
        ~TGPZKWorker() {}

        // 开始/关闭 zookeeper 连接
        bool startZkProcess();
        void stopZkProcess();

    private:
        ZK  zk_;
    };
}

#endif //TGPROXYSERVER_TGP_ZK_WROKER_H
