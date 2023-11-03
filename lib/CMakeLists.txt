
add_subdirectory(btree)
add_subdirectory(tp_poll)

add_library(SocketUtils STATIC message_utils.c sock_utils.c remote_routine.h remote_routine.c meta_info.c meta_info.h)
target_include_directories(SocketUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SocketUtils PUBLIC btree)
