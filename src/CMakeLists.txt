
# compile main executable
add_executable(master master.cpp WsServer.cpp PosixThread.cpp Thread.cpp WorkerThread.cpp Error.cpp CoreErrors.cpp PosixErrors.cpp)
target_include_directories(
    master
    PRIVATE
    ${PROJECT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/../external
    )

# link library
target_link_libraries(master PRIVATE lely-coapp websockets fmt)

# install binary
install(TARGETS master DESTINATION /arista/protocolbridge)
install(FILES master.dcf DESTINATION /arista/protocolbridge COMPONENT config)
