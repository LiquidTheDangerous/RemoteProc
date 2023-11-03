

set(tp_pool_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/tp_node.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tp_task.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tp_task_node.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tp_task_queue.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tp_thread_pool.c)

add_library(tp_pool STATIC ${tp_pool_sources})
set(tp_pool_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(tp_pool PUBLIC ${tp_pool_include_dir})
