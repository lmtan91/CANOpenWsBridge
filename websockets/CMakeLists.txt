cmake_minimum_required(VERSION 3.10)
project(arista-websockets)

set(target_name arista-websockets)

set(sources
        HttpRequest.hpp
        WsServerLws.cpp
        WsServerLws.hpp
        WsServerClient.hpp
        WsServerClientLws.hpp
        WsServerClientLws.cpp
        WsServerClientLwsImpl.hpp
        WsServerClientLwsImpl.cpp
        WsServerLwsPriv.hpp
        WsServerLwsPriv.cpp
        IncomingHttpRequest.hpp
        IncomingHttpRequest.cpp
)

add_library(${target_name} SHARED ${sources})

target_include_directories(${target_name} PUBLIC 
        ../../
        ../external/   # TODO make suitable for FindPackage
        ../external/cxxopt/
        ../core/
        ${LIBGSL_INCLUDE_DIR}
        )

target_include_directories(${target_name} PRIVATE
        .
        )

target_link_libraries(${target_name} arista-core fmt)

set(INSTALL_DIR /arista/protocolbridge)

# Install the library
install(TARGETS ${target_name}
        EXPORT ${target_name}Targets
        ARCHIVE DESTINATION ${INSTALL_DIR}
        LIBRARY DESTINATION ${INSTALL_DIR}
        RUNTIME DESTINATION ${INSTALL_DIR})
