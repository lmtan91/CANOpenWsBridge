
# compile main executable
add_executable(master master.cpp)
target_include_directories(
    master
    PRIVATE
        ${PROJECT_BINARY_DIR}
    )

# link library
target_link_libraries(master PRIVATE lely-coapp)

# install binary
install(TARGETS master DESTINATION /arista/protocolbridge)
install(FILES master.dcf DESTINATION /arista/protocolbridge COMPONENT config)
