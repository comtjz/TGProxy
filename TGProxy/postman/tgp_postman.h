//
// Created by hongchao1 on 2018/1/12.
//

#ifndef TGPROXYSERVER_TGP_POSTMAN_H
#define TGPROXYSERVER_TGP_POSTMAN_H

#include <vector>
#include <map>

#include "../tiangan/tiangan.h"
#include "../tiangan/slicer.h"
#include "../tiangan/event_hub.h"

namespace TGProxy {

    using Slicer = tiangan::RangeSlicer<tiangan::Key>;

    class TGProxyPostman {
    public:
        TGProxyPostman(int32_t range, const std::string& model_name, const std::string& model_ver)
                : range_(range), model_name_(model_name), model_ver_(model_ver), slicer_(range), hub_(new tiangan::EventHub()) {
            weights_.resize(range);
        }

        ~TGProxyPostman() {
            delete hub_;
        }

        bool checkRequest(const std::string& words, std::string& out);

        bool tranferRequest(std::string& out);

        bool answerRequest(std::string& reply, std::string& out);

        static void MSReplyCallback(CURLcode red, tiangan::MultiCURL::CURLHandle *handle);

    private:
        void encodeSlice(const Slicer::Slice& slice, std::string& json_slice);
        void encodeRankingWeights(const std::vector<double>& weights, std::string& json_weights);
        bool decodeMSWeights(const std::string weight_json, std::vector<double>& weights, std::string& out);

        void getRequestUrl();

        void saveMCurlResult(int partition, const std::string& response) {
            assert(partition >= 0 && partition < range_);
            weights_[partition] = response;
        }

        std::string get_ms_url(int i);

    private:
        int32_t     range_;
        std::string model_name_;
        std::string model_ver_;

        Slicer  slicer_;
        // ms返回结果
        std::vector<std::string> weights_;

        tiangan::EventHub *hub_;

        // features的hash值
        std::vector<tiangan::Key> features_;
        std::vector<Slicer::Slice> slices_;
    };
}

#endif //TGPROXYSERVER_TGP_POSTMAN_H
