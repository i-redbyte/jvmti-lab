# Общий каркас агентов: только заголовки.
add_library(agent_common INTERFACE)
target_include_directories(agent_common INTERFACE include)
target_compile_features(agent_common INTERFACE cxx_std_20)
target_link_libraries(agent_common INTERFACE log)
