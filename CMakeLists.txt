######################################################################
# configuration ######################################################
######################################################################

cmake_minimum_required(VERSION 3.19)
project(exqudens_cpp_cmake_test VERSION 1.0.0)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extension.cmake")
    file(
        DOWNLOAD
            "https://raw.githubusercontent.com/exqudens/exqudens-cmake/7.0.0/src/main/cmake/extension.cmake"
            "${CMAKE_CURRENT_SOURCE_DIR}/extension.cmake"
        EXPECTED_MD5
            "90f3ffc607fa7ff43f0d702d9d88c977"
    )
endif()
include(extension.cmake)

set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(RESOURCE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/res)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

######################################################################
# copy ###############################################################
######################################################################

get_filename_component(CXX_COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

set_copy_file_command_arguments(CFCA_LIB_STDCPP_6 ${CXX_COMPILER_DIR}
    "libstdc++-6.dll"
)
set_copy_file_command_arguments(CFCA_LIB_GCC_S_SEH_1 ${CXX_COMPILER_DIR}
    "libgcc_s_seh-1.dll"
)
set_copy_file_command_arguments(CFCA_LIB_WINPTHREAD_1 ${CXX_COMPILER_DIR}
    "libwinpthread-1.dll"
)

add_custom_target(copyRuntimeBinaries
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} ${CFCA_LIB_STDCPP_6} ${RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} ${CFCA_LIB_GCC_S_SEH_1} ${RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} ${CFCA_LIB_WINPTHREAD_1} ${RUNTIME_OUTPUT_DIRECTORY}
    #COMMAND ${CMAKE_COMMAND} -E make_directory ${RESOURCE_OUTPUT_DIRECTORY}
    #COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src/test/resources ${RESOURCE_OUTPUT_DIRECTORY}
)

######################################################################
# main ###############################################################
######################################################################

add_library(exqudensTestLibrary INTERFACE
    src/main/cpp/exqudens/test/Application.hpp
)
target_include_directories(exqudensTestLibrary INTERFACE
    src/main/cpp
)

######################################################################
# test ###############################################################
######################################################################

add_library(testLibrary INTERFACE
    src/test/cpp/exqudens/other/OtherTests.hpp
)
target_include_directories(testLibrary INTERFACE
    src/test/cpp
)
target_link_libraries(testLibrary INTERFACE
    exqudensTestLibrary
)

add_executable(TestExecutable src/test/cpp/main.cpp)
target_link_libraries(TestExecutable
    testLibrary
)
add_dependencies(TestExecutable copyRuntimeBinaries)

enable_testing()

add_test(
    NAME main
    WORKING_DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY}
    COMMAND TestExecutable -t
        exqudens.other.OtherTests.test1
        exqudens.other.OtherTests.test2
)
set_property(TEST main PROPERTY
    ENVIRONMENT "PATH=${RUNTIME_OUTPUT_DIRECTORY}"
)
