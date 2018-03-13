//
// Created by hongchao1 on 2018/1/12.
//

#include <thread>
#include <mutex>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "tgp_postman.h"
#include "../tiangan/mpath.h"
#include "../base/tgp_context.h"
#include "../cluster/tgp_cluster_admin.h"

using namespace rapidjson;

namespace TGProxy {

    bool TGProxyPostman::checkRequest(const std::string& words, std::string& out) {
        LOG(INFO) << "TGP Postman check letter start";
        Document d;
        d.Parse(words.c_str());
        if (d.HasParseError()) {
            LOG(ERROR) << "TGP Postman parse request fail, request = " << words;
            out = "please check, wrong request json";
            return false;
        }

        int64_t count = -1;
        if (d.HasMember("count") && d["count"].IsInt64()) {
            count = d["count"].GetInt64();
        }

        if (!d.HasMember("features")) {
            LOG(ERROR) << "TGP Postman check letter, gone features";
            out = "please check, features is gone";
            return false;
        }

        const Value& features = d["features"];
        if (!features.IsArray()) {
            LOG(ERROR) << "TGP Postman check letter, features is not array";
            out = "please check, features is not array";
            return false;
        }

        // features 有序
        for (SizeType i = 0; i < features.Size(); i++) {
            if (features[i].IsUint64()) {
                uint64_t feature = features[i].GetUint64();
                features_.push_back(feature);
            } else {
                LOG(ERROR) << "TGP Postman check letter, feature is not uint64";
                out = "please check, feature is not uint64";
                return false;
            }
        }

        if (features_.empty()) {
            LOG(ERROR) << "TGP Postman check letter, features is empty";
            out = "please check, features is empty";
            return false;
        }

        if (count != -1 && count != features_.size()) {
            LOG(ERROR) << "TGP Postman check letter, size != count, count = " << count << ", size = " << features_.size();
            out = "please check, features size != count";
            return false;
        }

        LOG(INFO) << "TGP Postman check letter end";
        return true;
    }

    bool TGProxyPostman::tranferRequest(std::string& out) {
        LOG(INFO) << "TGP Postman tranfer letter start";
        assert(!features_.empty());

        // 将features_切分
        slicer_.slice(features_, slices_);

        int64_t count = 0;
        for (int i = 0; i < slices_.size(); ++i) {
            if (slices_[i].empty()) {
                LOG(INFO) << "TGP Postman tranfer letter, partition(" << i << ") features empty";
                continue;
            }

            LOG(INFO) << "TGP Postman tranfer letter, partition(" << i << "):" << slices_[i].size();
            count += slices_[i].size();

            // 将每个分区的features打包成json格式
            std::string json_slice;
            encodeSlice(slices_[i], json_slice);
            if (json_slice.empty()) {
                LOG(ERROR) << "TGP Postman tranfer letter, partition(" << i << ") encode json fail";
                out = "tgp proxy fail, encode slice features fail";
                break;
            }
            // TODO 测试日志
            LOG(INFO) << "TGP Postman tranfer letter, partition(" << i << ") json = " << json_slice;

            // 获取访问ms的URL
            std::string url = get_ms_url(i);
            LOG(INFO) << "TGP Postman tranfer letter, ms url = " << url;

            // 注册访问回调
            hub_->register_event(url, MSReplyCallback, this, 2000, json_slice.c_str(), json_slice.size());
        }

        return true;
    }

    void TGProxyPostman::encodeSlice(const Slicer::Slice& slice, std::string& json_slice) {
        LOG(INFO) << "TGP Postman encode slice";

        Document doc;
        doc.SetObject(); // key-value 相当于map
        Document::AllocatorType &allocator = doc.GetAllocator(); // 获取分配器

        Value key(kStringType);
        key.SetString("features", allocator);

        Value val(kArrayType);
        for (auto it = slice.begin(); it != slice.end(); ++it) {
            val.PushBack(*it, allocator);
        }

        doc.AddMember(key, val, allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);
        json_slice = std::string(buffer.GetString());

        return;
    }

    void TGProxyPostman::encodeRankingWeights(const std::vector<double>& weights, std::string& json_weights) {
        Document doc;
        doc.SetObject();
        Document::AllocatorType &allocator = doc.GetAllocator();

        Value val(kArrayType);
        for (auto it = weights.begin(); it != weights.end(); ++it) {
            val.PushBack(*it, allocator);
        }

        doc.AddMember("weights", val, allocator);

        doc.AddMember("reason", "ok", allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);
        json_weights = std::string(buffer.GetString());

        return;
    }

    bool TGProxyPostman::decodeMSWeights(const std::string weight_json, std::vector<double>& weights, std::string& out) {
        Document doc;
        doc.Parse(weight_json.c_str());
        if (doc.HasParseError()) {
            LOG(ERROR) << "decode ms weights json fail";
            out = "tgp proxy fail, can't decode ms weights";
            return false;
        }

        if (doc.HasMember("reason") && doc.HasMember("weights") && doc["reason"].IsString() && doc["weights"].IsArray()) {
            std::string reason = doc["reason"].GetString();
            if (reason != "ok") {
                LOG(ERROR) << "decode ms weights json, response reason: " << reason;
                out = reason;
                return false;
            }

            Value &value = doc["weights"];
            for (auto& v : value.GetArray()) {
                if (!v.IsDouble()) {
                    LOG(ERROR) << "decode ms weights json, weights is not double";
                    out = "tgp proxy fail, no double weight";
                    return false;
                }
                double weight = v.GetDouble();
                weights.push_back(weight);
            }
        } else {
            LOG(ERROR) << "decode ms weights json, unkown format";
            out = "tgp proxy fail, unkown ms weights format";
            return false;
        }

        return true;
    }

    void TGProxyPostman::MSReplyCallback(CURLcode red, tiangan::MultiCURL::CURLHandle *handle) {
        LOG(INFO) << "multicurl callback";
        long http_code;
        curl_easy_getinfo(handle->easy, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
            LOG(ERROR) << "multicurl callback, response code = " << http_code;
            return;
        }

        TGProxyPostman* postman = (TGProxyPostman *)handle->userp;
        if (postman == nullptr) {
            LOG(ERROR) << "multicurl callback, empty postman pointer";
            return;
        }

        int partition = -1;
        std::string url = handle->url;
        std::size_t pos = url.rfind("/");
        if (pos != std::string::npos) {
            partition = std::stoi(url.substr(pos+1));
        }
        if (partition == -1) {
            LOG(ERROR) << "multicurl callback, wrong partition";
            return;
        }
        LOG(INFO) << "multicurl callback, partition = " << partition;

        std::string &ret = handle->recv_data;
        postman->saveMCurlResult(partition, ret);
    }

    std::string TGProxyPostman::get_ms_url(int partition) {
        std::string hostname = TGPClusterAdmin::getInstance()->getOneMsAddr(partition);
        if (hostname.empty()) {
            LOG(ERROR) << "EMPTY HOSTNAME";

            static int count = 0;
            ++count;
            vector<string> msHost = TGPContext::get()->msConf()->get_ms_list(partition);
            hostname = msHost[count % msHost.size()];
        }

        std::string mpath;
        mpath = model_name_;
        mpath += "/";
        mpath += model_ver_;
        mpath += "/";
        mpath += std::to_string(partition);

        std::string url = hostname + "/1/" + mpath;
        return url;
    }

    bool TGProxyPostman::answerRequest(std::string& reply, std::string& out) {
        LOG(INFO) << "TGP Postman answer Request start";
        hub_->start();

        std::vector<double> weights;
        for (int i = 0; i < slices_.size(); ++i) {
            if (slices_[i].empty()) {
                continue;
            }

            if (weights_[i].empty()) {
                LOG(ERROR) << "TGP Postman answer Request, partition(" << i << ") empty response";
                out = "tgp proxy fail, ms response timout or no response";
                return false;
            }

            if (!decodeMSWeights(weights_[i], weights, out)) {
                LOG(ERROR) << "TGP Postman answer Request, partition(" << i << ") decode ms weight fail";
                return false;
            }
        }

        if (weights.size() == 0) {
            LOG(ERROR) << "TGP Postman answer Request, no weights";
            out = "tgp proxy fail, empty weights";
            return false;
        }

        encodeRankingWeights(weights, reply);

        return true;
    }
}