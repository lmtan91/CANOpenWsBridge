cmake_minimum_required(VERSION 3.10)
project(arista-core)


set(target_name arista-core)

set(sources
        Core.hpp
        Error.cpp
        Error.hpp
        CoreErrors.hpp
        CoreErrors.cpp
        PosixErrors.hpp
        PosixErrors.cpp
        PosixThread.hpp
        PosixThread.cpp
        Thread.hpp
        Thread.cpp
        WorkerThread.hpp
        WorkerThread.cpp
        Observer.hpp
        Observable.hpp
        Subject.hpp
        StringUtils.hpp
        StringUtils.cpp
)

add_library(${target_name} SHARED  ${sources})

target_include_directories(${target_name} PUBLIC
        ../
        ../../
        ../external/   # TODO make suitable for FindPackage
        ../external/gsl/include
        ${LIBGSL_INCLUDE_DIR}
        )

target_include_directories(${target_name} PRIVATE
        .
        )

target_link_libraries(${target_name} fmt)

set(INSTALL_DIR /arista/protocolbridge)

# Install the library
install(TARGETS ${target_name}
        EXPORT ${target_name}Targets
        ARCHIVE DESTINATION ${INSTALL_DIR}
        LIBRARY DESTINATION ${INSTALL_DIR}
        RUNTIME DESTINATION ${INSTALL_DIR})
