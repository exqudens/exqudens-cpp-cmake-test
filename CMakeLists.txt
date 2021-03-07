######################################################################
# configuration ######################################################
######################################################################

cmake_minimum_required(VERSION 3.19)
project(exqudens-test VERSION 1.0.0)

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

get_filename_component(CXX_COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

set_copy_file_command_arguments(CFCA_LIB_STDCPP_6     ${CXX_COMPILER_DIR} "libstdc++-6.dll")
set_copy_file_command_arguments(CFCA_LIB_GCC_S_SEH_1  ${CXX_COMPILER_DIR} "libgcc_s_seh-1.dll")
set_copy_file_command_arguments(CFCA_LIB_WINPTHREAD_1 ${CXX_COMPILER_DIR} "libwinpthread-1.dll")

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

######################################################################
# main ###############################################################
######################################################################

add_library(exqudensTestLibraryInterface INTERFACE
    src/main/cpp/exqudens/test/Application.hpp
)
target_include_directories(exqudensTestLibraryInterface INTERFACE
    src/main/cpp
)
set_target_properties(exqudensTestLibraryInterface PROPERTIES
    INTERFACE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}/include"
)
add_custom_target(exqudensTestLibraryInterfaceBuild
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_OUTPUT_DIRECTORY>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_OUTPUT_DIRECTORY>
)
add_dependencies(exqudensTestLibraryInterfaceBuild exqudensTestLibraryInterface)

######################################################################
# test ###############################################################
######################################################################

add_library(exqudensTestLibraryInterfaceImported INTERFACE IMPORTED)
target_include_directories(exqudensTestLibraryInterfaceImported INTERFACE
    $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_OUTPUT_DIRECTORY>
)
add_dependencies(exqudensTestLibraryInterfaceImported exqudensTestLibraryInterfaceBuild)

add_library(testLibraryInterface INTERFACE
    src/test/cpp/exqudens/other/OtherTests.hpp
)
target_include_directories(testLibraryInterface INTERFACE
    src/test/cpp
)
target_link_libraries(testLibraryInterface INTERFACE
    exqudensTestLibraryInterfaceImported
)
add_dependencies(testLibraryInterface exqudensTestLibraryInterfaceImported)

add_executable(TestExecutableShared src/test/cpp/main.cpp)
target_link_libraries(TestExecutableShared
    testLibraryInterface
)
set_target_properties(TestExecutableShared PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build/test/${PROJECT_NAME}-${PROJECT_VERSION}/bin"
)
add_custom_target(copyTestExecutableSharedLibraries
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_PROPERTY:TestExecutableShared,RUNTIME_OUTPUT_DIRECTORY>
    COMMAND ${CMAKE_COMMAND} ${CFCA_LIB_STDCPP_6} $<TARGET_PROPERTY:TestExecutableShared,RUNTIME_OUTPUT_DIRECTORY>
    COMMAND ${CMAKE_COMMAND} ${CFCA_LIB_GCC_S_SEH_1} $<TARGET_PROPERTY:TestExecutableShared,RUNTIME_OUTPUT_DIRECTORY>
    COMMAND ${CMAKE_COMMAND} ${CFCA_LIB_WINPTHREAD_1} $<TARGET_PROPERTY:TestExecutableShared,RUNTIME_OUTPUT_DIRECTORY>
    #COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_PROPERTY:TestExecutableShared,RESOURCE_OUTPUT_DIRECTORY>
    #COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src/test/resources $<TARGET_PROPERTY:TestExecutableShared,RESOURCE_OUTPUT_DIRECTORY>
)
add_dependencies(TestExecutableShared testLibraryInterface copyTestExecutableSharedLibraries)

enable_testing()

add_test(
    NAME main
    WORKING_DIRECTORY $<TARGET_PROPERTY:TestExecutableShared,RUNTIME_OUTPUT_DIRECTORY>
    COMMAND TestExecutableShared -t
        exqudens.other.OtherTests.test1
        exqudens.other.OtherTests.test2
)
set_tests_properties(main PROPERTIES
    ENVIRONMENT "PATH=$<TARGET_PROPERTY:TestExecutableShared,RUNTIME_OUTPUT_DIRECTORY>"
)

######################################################################
# info ###############################################################
######################################################################

add_custom_target(info
    COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_SOURCES>
    COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_PROPERTY:exqudensTestLibraryInterface,SOURCES>
    COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_INCLUDE_DIRECTORIES>
    COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_OUTPUT_DIRECTORY>
    COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_PROPERTY:TestExecutableShared,RUNTIME_OUTPUT_DIRECTORY>
)
