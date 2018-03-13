if(APPLE)
    include_directories("/usr/local/opt/openssl/include")
endif()

include_directories(/recsys/build2/env/release/include/)
link_directories(/recsys/build2/env/release/lib/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -DTIANGAN_USE_GLOG -DTHREADED")

FILE(GLOB_RECURSE SOURCE_FILES main.cc "proxy/*.cc" "base/*.cc" "tiangan/*.cc" "postman/*.cc" "cluster/*.cc" "utils/*.cc")

add_executable(TGProxyServer ${SOURCE_FILES})

# 使用系统默认的库地址

target_link_libraries(TGProxyServer libproxygenhttpserver.a)
target_link_libraries(TGProxyServer libproxygenlib.a)
target_link_libraries(TGProxyServer libwangle.a)
target_link_libraries(TGProxyServer libfolly.a)
target_link_libraries(TGProxyServer libgflags.a)
target_link_libraries(TGProxyServer libglog.a)
target_link_libraries(TGProxyServer libdouble-conversion.a)
target_link_libraries(TGProxyServer libboost_context.a)


target_link_libraries(TGProxyServer pthread)
target_link_libraries(TGProxyServer libcurl.a)
target_link_libraries(TGProxyServer ssl)
target_link_libraries(TGProxyServer ldap)

target_link_libraries(TGProxyServer libevent.a)
target_link_libraries(TGProxyServer libzookeeper_mt.a)


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)