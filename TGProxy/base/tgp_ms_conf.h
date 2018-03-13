//
// Created by hongchao1 on 2018/1/12.
//

#ifndef TGPROXYSERVER_TGP_MS_CONF_H
#define TGPROXYSERVER_TGP_MS_CONF_H

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include <glog/logging.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace std;

namespace TGProxy {

    struct TGProxyMSConf {
        std::string    name_;
        std::string    version_;
        //int32_t        parts_;
        vector<vector<string>> ms_lists_;

        bool parse_ms_conf(const rapidjson::Value& value) {
            if (!value.HasMember("name") || !value["name"].IsString()) {
                LOG(ERROR) << "wrong ms conf, lost name";
                return false;
            }
            name_ = value["name"].GetString();
            LOG(INFO) << "name = " << name_;

            if (!value.HasMember("version") || !value["version"].IsString()) {
                LOG(ERROR) << "wrong ms conf, lost version";
                return false;
            }
            version_ = value["version"].GetString();
            LOG(INFO) << "version = " << version_;

            //if (!value.HasMember("parts") || !value["parts"].IsInt()) {
            //    LOG(ERROR) << "wrong ms conf, lost parts";
            //    return false;
            //}
            //parts_ = value["parts"].GetInt();
            //LOG(INFO) << "parts = " << parts_;

            if (!value.HasMember("ms_list") || !value["ms_list"].IsArray()) {
                LOG(ERROR) << "wrong ms conf, lost ms list";
                return false;
            }

            for (rapidjson::SizeType i = 0; i < value["ms_list"].Size(); i++) {
                if (!value["ms_list"][i].IsArray()) {
                    LOG(ERROR) << "wrong ms conf-ms-list, " << i << "th unknown type";
                    return false;
                }

                for (rapidjson::SizeType j = 0; j < value["ms_list"][i].Size(); j++) {
                    if (!value["ms_list"][i][j].IsString()) {
                        LOG(ERROR) << "wrong ms conf-ms-list-" << i << "-" << j << " unknown type";
                        return false;
                    }
                    string addr = value["ms_list"][i][j].GetString();
                    if (addr.empty()) {
                        LOG(ERROR) << "wrong ms conf-ms-list-" << i << "-" << j << " empty addr";
                        return false;
                    }

                    if (ms_lists_.size() == i) {
                        ms_lists_.push_back(vector<string>());
                    }
                    ms_lists_[i].push_back(addr);
                    LOG(INFO) << "ms list[" << i << "]: " << addr;
                }
            }

            return true;
        }
    };

    struct TGProxyMSConfFile {
        map<std::string, TGProxyMSConf> ms_conf_;
        string key_;

        void set_ms_conf_key(string& key) {
            key_ = key;
        }

        int32_t get_ms_parts(string key = "") {
            if (key.empty()) {
                key = key_;
            }

            return ms_conf_[key].ms_lists_.size();
        }

        vector<string> get_ms_list(int i, string key = "") {
            if (key.empty()) {
                key = key_;
            }

            return ms_conf_[key].ms_lists_[i];
        }

        bool load_ms_conf_file(const std::string& config) {
            ifstream in;
            in.open(config.c_str(), ifstream::in);
            if (!in.is_open()) {
                LOG(ERROR) << "open ms conf fail";
                return false;
            }

            string stringFromStream;
            string line;
            while (getline(in, line)) {
                stringFromStream.append(line + "\n");
            }
            in.close();

            using rapidjson::Document;
            Document doc;
            doc.Parse<0>(stringFromStream.c_str());
            if (doc.HasParseError()) {
                rapidjson::ParseErrorCode code = doc.GetParseError();
                LOG(ERROR) << "parse ms conf fail, errorcode:" << code;
                return false;
            }

            for (auto& m : doc.GetObject()) {
                string key = m.name.GetString();
                if (key.empty()) {
                    LOG(ERROR) << "parse ms conf fail, empty key";
                    return false;
                }

                const rapidjson::Value& value = m.value;
                TGProxyMSConf msconf;
                bool ret = msconf.parse_ms_conf(value);
                if (!ret) {
                    LOG(ERROR) << "parse ms conf fail, wrong ms conf";
                    return false;
                }

                ms_conf_.insert(pair<string, TGProxyMSConf>(key, msconf));

                if (key_.empty()) {
                    key_ = key;
                }
            }

            return true;
        }
    };
}

#endif //TGPROXYSERVER_TGP_MS_CONF_H
