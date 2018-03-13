/*
 * event_hub.h
 *
 * Created on: 2017年9月28日
 * Author: zhujun
 * package tiangan-ps
 * @Weibo @半饱半醉
 */

#ifndef SRC_PS_EVENT_HUB_H_
#define SRC_PS_EVENT_HUB_H_

#include <vector>
#include <string>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <glog/logging.h>
#include "curl_helper.h"

namespace tiangan {

    class EventHub {
    public:
        EventHub() {
            event_base_ = event_base_new();
            if (!event_base_) {
                LOG(ERROR) << "event_base_new error";
                throw std::bad_alloc();
            }

            multi_ = new MultiCURL(event_base_);
            if(!multi_) {
                event_base_free(event_base_);
                LOG(ERROR) << "memory error";
                throw std::bad_alloc();
            }
        }

        ~EventHub() {
            delete multi_;

            event_base_free(event_base_);
        }

        void start() {
            event_base_loop(event_base_, 0);

            //Exit from loop, clean event data
            for(unsigned int i = 0; i < events_.size(); ++i) {
                event_del(events_[i]);
                event_free(events_[i]);
            }
        }

        void stop() {
            event_base_loopbreak(event_base_);
        }

        event_base* get_event_base() {
            return event_base_;
        }

        MultiCURL::CURLHandle* register_event(const std::string& url, const MultiCURL::Callback& cb, void *userp,
                                              long timeout_ms = 0, const char *data = nullptr, long data_size = -1,
                                              const std::vector<std::string>& headers = {}, int verbose = 0) {
            auto handle = multi_->create(url, cb, userp, false, timeout_ms, data, data_size, headers, verbose);
            multi_->add(handle);
            return handle;
        }

        void register_event(MultiCURL::CURLHandle *handle) {
            multi_->add(handle);
        }

        struct event* register_event(int fd, short which, event_callback_fn cb, void *usrp, const struct timeval *timeout = NULL) {
            struct event *ev = event_new(event_base_, fd, which, cb, usrp);
            event_add(ev, timeout);
            events_.push_back(ev);
            return ev;
        }

        struct event* register_event(event_callback_fn cb, void *usrp, const struct timeval *timeout) {
            struct event *ev = evtimer_new(event_base_, cb, usrp);
            evtimer_add(ev, timeout);
            events_.push_back(ev);
            return ev;
        }

        void register_event(struct event *ev, int fd, short which, event_callback_fn cb, void *userp, const struct timeval *timeout = NULL) {
            event_assign(ev, event_base_, fd, which, cb, userp);
            event_add(ev, timeout);
        }

        void register_event(struct event *ev, const struct timeval *timeout = NULL) {
            event_add(ev, timeout);
        }

    private:
        struct event_base *event_base_{nullptr};

        MultiCURL *multi_{nullptr};

        std::vector<struct event*> events_;
    };

}

#endif /* SRC_PS_EVENT_HUB_H_ */
