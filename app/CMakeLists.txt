
# compile main executable
add_executable(master master.cpp)
target_include_directories(
    master
    PRIVATE
    ${PROJECT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/../external
    ${PROJECT_BINARY_DIR}/../core
    ${PROJECT_BINARY_DIR}/../websockets
    )

# link library
target_link_libraries(master PRIVATE arista-core lely-coapp arista-websockets websockets fmt)

# install binary
install(TARGETS master DESTINATION /arista/protocolbridge)
install(FILES master.dcf DESTINATION /arista/protocolbridge COMPONENT config)
