//
// Created by hongchao1 on 2018/1/23.
//

#ifndef TGPROXYSERVER_TGP_ZK_WATCHER_H
#define TGPROXYSERVER_TGP_ZK_WATCHER_H

#include <string>
#include <zookeeper/zookeeper.h>


namespace TGProxy {

    struct TGPWatcher {
        static void tgpChildrenWatcher(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx);

        static void zk_watcher(zhandle_t *zh, int type, int state, const char* path, void *watcherCtx);
    };
}


#endif //TGPROXYSERVER_TGP_ZK_WATCHER_H
