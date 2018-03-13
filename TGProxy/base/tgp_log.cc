//
// Created by hongchao1 on 2018/1/11.
//

#include <glog/logging.h>

#include "tgp_log.h"

namespace TGProxy {

TGProxyGLog::TGProxyGLog(char *program) {
    google::InitGoogleLogging(program);
}

TGProxyGLog::~TGProxyGLog() {
    google::ShutdownGoogleLogging();
}

void TGProxyGLog::setLogDetails(std::string log_dir, bool logtostderr) {
    if (logtostderr) {
        google::SetStderrLogging(google::INFO);
        FLAGS_colorlogtostderr = true;
    }

    std::string log_info_dest = log_dir + "/INFO_";
    google::SetLogDestination(google::INFO, log_info_dest.c_str());

    std::string log_warn_dest = log_dir + "/WARNING_";
    google::SetLogDestination(google::WARNING, log_warn_dest.c_str());

    std::string log_error_dest = log_dir + "/ERROR_";
    google::SetLogDestination(google::ERROR, log_error_dest.c_str());

    FLAGS_logbufsecs = 0; // 缓冲日志输出,此处改为立即输出
    FLAGS_max_log_size = 2000; // 最大日志大小为 2GB
    FLAGS_stop_logging_if_full_disk = true; // 当磁盘被写满时,停止日志输出
}

}
