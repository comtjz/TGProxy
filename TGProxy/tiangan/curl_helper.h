/*
 * curl_helper.h
 *
 * Created on: 2017年9月2日
 * Author: zhujun
 * package tiangan-ps
 * @Weibo @半饱半醉
 */

#ifndef SRC_CURL_HELPER_H_
#define SRC_CURL_HELPER_H_

#include <glog/logging.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <curl/curl.h>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <random>
#include <functional>
#include <string>
#include <iostream>
#include "str.h"

namespace tiangan {

    static uint write_cb(char *in, uint size, uint nmemb, void *out) {
        std::string *ret = (std::string*)out;
        auto len = size * nmemb;
        ret->append(in, len);
        return len;
    }

    inline void replace_url_param_value(std::string& url, const std::string& param, const std::string& new_value) {
        std::string query = param;
        query += "=";

        auto pos = url.find(query);
        if(pos == std::string::npos) {
            if(url.find("?") == std::string::npos) {
                url += "?";
                url += query;
                url += new_value;
            }
            else {
                url += "&";
                url += query;
                url += new_value;
            }
        }
        else {
            pos += query.size();
            auto endp = url.find("&", pos);
            if(endp == std::string::npos) {
                endp = url.size();
            }
            url.replace(pos, endp - pos, new_value);
        }
    }

    class EasyCURL {
    public:
        EasyCURL() : result_(""), http_code_(0), curl_(curl_easy_init()) {
            if(!curl_) {
                error_ = "curl_easy_init() failed";
            }
        }

        ~EasyCURL() {
            curl_easy_cleanup(curl_);
        }

        bool request(const std::string& url, const std::vector<std::string>& headers,
                     long conn_timeout_ms, long timeout_ms,
                     int verbose) {
            bool ret;
            CURLcode res;
            struct curl_slist *http_headers = NULL;

            error_.clear();
            result_.clear();
            http_code_ = 0;

            curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

            if(verbose == 1) {
                curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L);
            }
            else {
                curl_easy_setopt(curl_, CURLOPT_VERBOSE, 0L);
            }

            if(headers.size() > 0) {
                for(auto& header : headers) {
                    http_headers = curl_slist_append(http_headers, header.c_str());
                }
                curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, http_headers);
            }
            else {
                curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, NULL);
            }

            if(conn_timeout_ms > 0) {
                curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT_MS, conn_timeout_ms);
            }
            else {
                curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT_MS, 0L);	//default
            }

            if(timeout_ms > 0) {
                curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeout_ms);
            }
            else {
                curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, 0L); //default
            }

            curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_cb);
            curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &result_);

            res = curl_easy_perform(curl_);
            if(res != CURLE_OK) {
                error_ = curl_easy_strerror(res);
                ret = false;
                goto clean;
            }
            curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code_);
            ret = true;
            clean:
            if(headers.size() > 0) {
                curl_slist_free_all(http_headers);
            }
            return ret;
        }

        bool get(const std::string& url, const std::vector<std::string>& headers = {},
                 long conn_timeout_ms = 0, long timeout_ms = 0, int verbose = 0) {
            curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
            return request(url, headers, conn_timeout_ms, timeout_ms, verbose);
        }

        bool post(const std::string& url, const std::string& data,
                  const std::vector<std::string>& headers = {},
                  long conn_timeout_ms = 0, long timeout_ms = 0, int verbose = 0) {
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, data.size());
            return request(url, headers, conn_timeout_ms, timeout_ms, verbose);
        }

        bool put(const std::string& url, const std::string& data,
                 const std::vector<std::string>& headers = {},
                 long conn_timeout_ms = 0, long timeout_ms = 0, int verbose = 0) {
            curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, data.size());
            return request(url, headers, conn_timeout_ms, timeout_ms, verbose);
        }

        std::string escape(const char *str, int length) {
            char* new_str = curl_easy_escape(curl_, str, length);
            std::string ret = new_str;
            curl_free(new_str);
            return ret;
        }

    public:
        std::string result_;
        std::string error_;
        int http_code_;

    private:
        CURL *curl_;
    };

    class MultiCURL {
    public:
        class CURLHandle;

        using Callback = std::function<void(CURLcode res, CURLHandle* handle)>;

        class CURLHandle {
        public:
            CURLHandle() {
                easy = curl_easy_init();
                error[0] = '\0';
            }

            ~CURLHandle() {
                curl_easy_cleanup(easy);
                if(sheaders) {
                    curl_slist_free_all(sheaders);
                }
            }

            void clear() {
                memset(error, 0, CURL_ERROR_SIZE);
                recv_data.clear();
                rheaders.clear();
            }

        public:
            CURL *easy{nullptr};
            std::string url;
            struct curl_slist *sheaders{nullptr};           //send headers
            char error[CURL_ERROR_SIZE];
            long timeout_ms{0};
            std::vector<char> send_data;
            std::string recv_data;
            std::map<std::string, std::string> rheaders;    //recv headers
            bool recycled{false};
            Callback cb{nullptr};
            void *userp{nullptr};
        };

        struct CURLSocket {
            curl_socket_t fd;
            CURL *easy;
            int action;
            long timeout;
            struct event ev;
        };

        MultiCURL(event_base *event_base) : event_base_(event_base), curl_multi_(curl_multi_init()) {
            curl_multi_setopt(curl_multi_, CURLMOPT_SOCKETFUNCTION, MultiCURL::curl_sock_handler);
            curl_multi_setopt(curl_multi_, CURLMOPT_SOCKETDATA, this);
            curl_multi_setopt(curl_multi_, CURLMOPT_TIMERFUNCTION, MultiCURL::curl_timer_handler);
            curl_multi_setopt(curl_multi_, CURLMOPT_TIMERDATA, this);
            ev_timer_ = evtimer_new(event_base_, MultiCURL::ev_timer_handler, this);
        }

        ~MultiCURL() {
            curl_multi_cleanup(curl_multi_);
            evtimer_del(ev_timer_);
            event_free(ev_timer_);
        }

        /*
         * recycled:
         *          false, 请求完成后删除handle
         *          true,  请求完成后不删除handle，可继续复用
         * */
        CURLHandle* create(const std::string& url, const Callback& cb, void *userp, bool recycled = false,
                           long timeout_ms = 0, const char *data = nullptr, long data_size = -1,
                           const std::vector<std::string>& headers = {}, int verbose = 0) {
            auto* handle = new CURLHandle();
            handle->cb = cb;
            handle->userp = userp;
            handle->timeout_ms = timeout_ms;
            handle->recycled = recycled;
            handle->url = url;

            if(headers.size() > 0) {
                for(auto& header : headers) {
                    handle->sheaders = curl_slist_append(handle->sheaders, header.c_str());
                }
                curl_easy_setopt(handle->easy, CURLOPT_HTTPHEADER, handle->sheaders);
            }
            curl_easy_setopt(handle->easy, CURLOPT_URL, url.c_str());
            if(data) {
                //POST method
                handle->send_data.resize(data_size);
                std::copy(data, data + data_size, handle->send_data.data());
                curl_easy_setopt(handle->easy, CURLOPT_POSTFIELDS, handle->send_data.data());
                curl_easy_setopt(handle->easy, CURLOPT_POSTFIELDSIZE, handle->send_data.size());
            }
            if(timeout_ms > 0) {
                curl_easy_setopt(handle->easy, CURLOPT_TIMEOUT_MS, timeout_ms);
            }
            else {
                curl_easy_setopt(handle->easy, CURLOPT_TIMEOUT_MS, 0L); //default
            }
            curl_easy_setopt(handle->easy, CURLOPT_WRITEFUNCTION, MultiCURL::curl_write_cb);
            curl_easy_setopt(handle->easy, CURLOPT_WRITEDATA, handle);
            curl_easy_setopt(handle->easy, CURLOPT_HEADERFUNCTION, MultiCURL::curl_header_cb);
            curl_easy_setopt(handle->easy, CURLOPT_HEADERDATA, handle);
            if(verbose) curl_easy_setopt(handle->easy, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(handle->easy, CURLOPT_ERRORBUFFER, handle->error);
            curl_easy_setopt(handle->easy, CURLOPT_PRIVATE, handle);
            return handle;
        }

        void add(CURLHandle *handle) {
            CURLMcode rc;

            VLOG(2) << "ADD url: " << handle->url;
            handle->clear();
            rc = curl_multi_add_handle(curl_multi_, handle->easy);
            if(rc != CURLM_OK) {
                LOG(ERROR) << "curl_multi_add_handle failed: " << rc;
            }
        }

    private:
        static inline timeval get_timeval(long timeout_ms) {
            struct timeval tv;
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            return tv;
        }

        void check_multi_info() {
            char *eff_url;
            CURLMsg *msg;
            int msgs_left;
            CURLHandle *handle;
            CURL *easy;
            CURLcode res;
            long http_code;

            while((msg = curl_multi_info_read(this->curl_multi_, &msgs_left))) {
                if(msg->msg == CURLMSG_DONE) {
                    res = msg->data.result;
                    easy = msg->easy_handle;
                    curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);
                    curl_easy_getinfo(easy, CURLINFO_PRIVATE, &handle);
                    curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &eff_url);
                    if(res != CURLE_OK) {
                        LOG(ERROR) << "ERR: " << eff_url << " => " << res << ", code: " << http_code << ", error: " << handle->error << ", size: " << handle->send_data.size();
                    }
                    else {
                        VLOG(2) << "DONE: " << eff_url << " => " << res << ", code: " << http_code << ", error: " << handle->error;
                    }
                    VLOG(2) << "REMAINING: " << still_running_;
                    curl_multi_remove_handle(this->curl_multi_, easy);
                    if(handle->cb) handle->cb(res, handle);
                    if(!handle->recycled) {
                        delete handle;
                    }
                }
            }
        }

    private:
        static int curl_timer_handler(CURLM *multi, long timeout_ms, void *userp) {
            MultiCURL *curl = (MultiCURL*)userp;

            if(timeout_ms == 0) {
                CURLMcode rc;
                rc = curl_multi_socket_action(curl->curl_multi_, CURL_SOCKET_TIMEOUT, 0, &curl->still_running_);
                if(rc != CURLM_OK) {
                    LOG(ERROR) << "curl_timer_handler failed: " << rc;
                    return 0;
                }
                curl->check_multi_info();
            }
            else if(timeout_ms == -1) {
                evtimer_del(curl->ev_timer_);
            }
            else {
                struct timeval timeout = MultiCURL::get_timeval(timeout_ms);
                evtimer_add(curl->ev_timer_, &timeout);
            }
            return 0;
        }

        static int curl_sock_handler(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp) {
            MultiCURL *curl = (MultiCURL*)cbp;
            CURLSocket* sk = (CURLSocket*)sockp;
            CURLHandle *handle;

            curl_easy_getinfo(e, CURLINFO_PRIVATE, &handle);
            if(what == CURL_POLL_REMOVE) {
                event_del(&sk->ev);
                delete sk;
            }
            else {
                if(!sk) {
                    sk = new CURLSocket();
                    curl_multi_assign(curl->curl_multi_, s, sk);
                }
                int kind = (what & CURL_POLL_IN  ? EV_READ  : 0) |
                           (what & CURL_POLL_OUT ? EV_WRITE : 0) |
                           EV_PERSIST;
                sk->fd = s;
                sk->easy = e;
                sk->action = what;
                if(event_initialized(&sk->ev)) {
                    if(event_pending(&sk->ev, EV_READ|EV_WRITE|EV_SIGNAL|EV_TIMEOUT, NULL)) {
                        event_del(&sk->ev);
                    }
                }
                event_assign(&sk->ev, curl->event_base_, s, kind, MultiCURL::ev_event_handler, curl);
                if(handle->timeout_ms > 0) {
                    struct timeval tv = MultiCURL::get_timeval(handle->timeout_ms);
                    event_add(&sk->ev, &tv);
                }
                else {
                    event_add(&sk->ev, NULL);
                }
            }
            return 0;
        }

    private:
        static size_t curl_write_cb(void *ptr, size_t size, size_t nmemb, void *data) {
            CURLHandle *handle = (CURLHandle*)data;
            handle->recv_data.append((char*)ptr, size * nmemb);
            return size * nmemb;
        }

        static size_t curl_header_cb(void *ptr, size_t size, size_t nmemb, void *data) {
            CURLHandle *handle = (CURLHandle*)data;
            size_t len = size * nmemb;
            char *start = (char*)ptr;
            char *end = start + len;

            auto pos = std::find(start, end, ':');
            if(pos == end) {
                return len;
            }

            std::string key;
            std::string val;

            key.append(start, pos - start);
            val.append(pos + 1, end - pos - 1);
            trim(key);
            trim(val);
            if(key.size() && val.size())
                handle->rheaders[key] = val;
            else
                LOG(WARNING) << "Invalid header: Key: " << key << ", val: " << val;
            return len;
        }

    private:
        static void ev_timer_handler(int fd, short kind, void *userp) {
            MultiCURL *curl = (MultiCURL*)userp;
            CURLMcode rc;

            rc = curl_multi_socket_action(curl->curl_multi_, CURL_SOCKET_TIMEOUT, 0, &curl->still_running_);
            if(rc != CURLM_OK) {
                LOG(ERROR) << "ev_timer_handler failed: " << rc;
                return;
            }
            curl->check_multi_info();
        }

        static void ev_event_handler(int fd, short kind, void *userp) {
            MultiCURL *curl = (MultiCURL*)userp;
            CURLMcode rc;

            int action = (kind & EV_READ ? CURL_CSELECT_IN : 0) | (kind & EV_WRITE ? CURL_CSELECT_OUT : 0);
            rc = curl_multi_socket_action(curl->curl_multi_, fd, action, &curl->still_running_);
            if(rc != CURLM_OK) {
                LOG(ERROR) << "ev_event_handler failed: " << rc;
                return;
            }

            curl->check_multi_info();
            if(curl->still_running_ <= 0) {
                VLOG(2) << "All task finished";
            }
        }

    private:
        event_base *event_base_{nullptr};
        CURLM *curl_multi_;
        struct event *ev_timer_;
        int still_running_{0};
    };

}

#endif /* SRC_CURL_HELPER_H_ */
