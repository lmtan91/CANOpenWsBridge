project(slave)

add_executable(slave slave.cpp)

target_link_libraries(slave PRIVATE lely-coapp)

install(TARGETS slave DESTINATION /arista/protocolbridge)
install(FILES cpp-slave.eds DESTINATION /arista/protocolbridge COMPONENT config)
