

add_executable(Server main.c)
target_link_libraries(Server PUBLIC SocketUtils)
target_link_libraries(Server PUBLIC tp_pool)
