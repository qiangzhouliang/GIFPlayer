
cmake_minimum_required(VERSION 3.22.1)

project("gifplayer")


add_library(
        gifplayer

        SHARED

        native-lib.cpp
        dgif_lib.c
        gifalloc.c
        openbsd-reallocarray.c
)

find_library(
        jnigraphics-lib
        jnigraphics  # 操作gif需要导入此系统库
)

find_library(
        log-lib
        log)


target_link_libraries(
        gifplayer

        ${log-lib}
        ${jnigraphics-lib} # 链接系统库
)