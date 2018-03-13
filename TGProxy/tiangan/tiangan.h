/*
 * tiangan.h
 *
 * Created on: 2017年11月4日
 * Author: zhujun
 * package tiangan
 * @Weibo @半饱半醉
 */

#ifndef SRC_TIANGAN_H_
#define SRC_TIANGAN_H_

#include <string>
#include <limits>
#include <exception>

#ifdef TIANGAN_USE_GLOG
#include <glog/logging.h>
#else
#include <ostream>
#include <iostream>

#define LOG_INFO tiangan::LogMessage(__FILE__, __LINE__)
#define LOG_ERROR LOG_INFO
#define LOG_WARNING LOG_INFO
#define LOG_FATAL LOG_INFO
#define LOG_QFATAL LOG_FATAL
#define VLOG(x) LOG_INFO.stream()

#define LOG(severity) LOG_##severity.stream()
#define LG LOG_INFO.stream()

namespace tiangan {

    class DateLogger {
    public:
        DateLogger() {
        }

        const char* HumanDate() {
            time_t time_value = time(NULL);
            struct tm now;
            localtime_r(&time_value, &now);
            snprintf(buffer_, sizeof(buffer_), "%02d:%02d:%02d", now.tm_hour,
                     now.tm_min, now.tm_sec);
            return buffer_;
        }

    private:
        char buffer_[9];
    };

    class LogMessage {
    public:
        LogMessage(const char* file, int line) : log_stream_(std::cerr) {
            log_stream_ << "[" << pretty_date_.HumanDate() << "] " << file << ":" << line << ": ";
        }
        ~LogMessage() { log_stream_ << "\n"; }
        std::ostream& stream() { return log_stream_; }

    protected:
        std::ostream& log_stream_;

    private:
        DateLogger pretty_date_;
        LogMessage(const LogMessage&);
        void operator=(const LogMessage&);
    };

} //namespace tiangan

#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

namespace tiangan {

    typedef   int8_t  int8;
    typedef  uint8_t  uint8;
    typedef  int32_t  int32;
    typedef uint32_t  uint32;
    typedef  int64_t  int64;
    typedef uint64_t  uint64;

    using Key = uint64;

    struct FileIOError {
        std::string detail;
    };

    template<class T>
    inline uint8* cast_pointer(T* p) {
        return reinterpret_cast<uint8*>(p);
    }

    template<class T>
    inline const uint8* cast_const_pointer(const T* p) {
        return reinterpret_cast<const uint8*>(p);
    }

    typedef union align_union {
        double d;
        void *v;
        int (*func)(int);
        long l;
    } align_union;

#define MEMORY_ALIGNMENT sizeof(align_union)
#define mem_align(n, a) (n) = ((((size_t)(n) - 1) & ~((a) - 1)) + (a))
#define mem_align_ptr(n, a) (n) = (u_char*)((((size_t)(n) - 1) & ~((size_t)(a) - 1)) + (size_t)(a))

#define NO_INTR(fn)     do {} while((fn) < 0 && errno == EINTR)

#ifndef USE_CXX11
#   define USE_CXX11 (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L || defined(_MSC_VER))
#endif

#ifndef DISALLOW_COPY_AND_ASSIGN_CTOR
#  if USE_CXX11
#    define DISALLOW_COPY_AND_ASSIGN_CTOR(T) \
       T(T const&) = delete; \
       T(T&&) = delete; \
       T& operator=(T const&) = delete; \
       T& operator=(T&&) = delete
#  else
    #    define DISALLOW_COPY_AND_ASSIGN_CTOR(T) \
       T(T const&); \
       T& operator=(T const&)
#  endif
#endif

#define TIANGAN_SUCCESS      0
#define TIANGAN_FAILURE     -1

}

#endif /* SRC_TIANGAN_H_ */
