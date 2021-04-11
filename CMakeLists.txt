######################################################################
# configuration ######################################################
######################################################################

cmake_minimum_required(VERSION 3.20)
project(exqudens-test VERSION 1.0.0)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extension.cmake")
    file(
        DOWNLOAD
            "https://raw.githubusercontent.com/exqudens/exqudens-cmake/8.1.0/src/main/cmake/extension.cmake"
            "${CMAKE_CURRENT_SOURCE_DIR}/extension.cmake"
        EXPECTED_MD5
            "d454147f9aebec3e35e924221c4042e9"
    )
endif()
include(extension.cmake)

get_filename_component(CXX_COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
set_home_prefix_if_not_defined(HOME_PREFIX)
set_if_not_defined(REPOSITORY_PREFIX ${HOME_PREFIX}/.cmake/repository)
set_if_not_defined(FILES_PREFIX ${HOME_PREFIX}/.cmake/files)

set(CMAKE_INSTALL_PREFIX ${FILES_PREFIX}/${PROJECT_NAME}/${PROJECT_VERSION})
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

######################################################################
# main ###############################################################
######################################################################

add_library(exqudensTestLibraryInterface INTERFACE
    src/main/cpp/include/exqudens/test/Application.hpp
)
target_include_directories(exqudensTestLibraryInterface INTERFACE
    src/main/cpp/include
)
set_target_properties(exqudensTestLibraryInterface PROPERTIES
    BUILD_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}"
    BUILD_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}.zip"
    INTERFACE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}/include"
)
add_custom_target(project-build
    COMMAND    ${CMAKE_COMMAND}
               -E
               make_directory
               $<TARGET_PROPERTY:exqudensTestLibraryInterface,BUILD_OUTPUT_DIRECTORY>
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}
    COMMAND    ${CMAKE_COMMAND}
               -E
               make_directory
               $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_OUTPUT_DIRECTORY>
    COMMAND    ${CMAKE_COMMAND}
               -E
               copy_directory
               $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_INCLUDE_DIRECTORIES>
               $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_OUTPUT_DIRECTORY>
    COMMENT    "Building project."
)
add_dependencies(project-build exqudensTestLibraryInterface)

add_custom_targets_zip_directory(
    project-pack
    true
    project-build
    ${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}
    ${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}.zip
    ${CMAKE_COMMAND}
)

add_executable(main src/main/cpp/main.cpp)
target_link_libraries(main exqudensTestLibraryInterface)

######################################################################
# test ###############################################################
######################################################################

add_custom_targets_dependency(
    submodule-exqudens-test
    true
    "project-pack-clean;project-pack"
    file:///${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}.zip
    include
    ""
    ""
    ${CMAKE_CURRENT_BINARY_DIR}/build/test/dependencies/${PROJECT_NAME}/${PROJECT_VERSION}
    ${CMAKE_COMMAND}
    ${CMAKE_CURRENT_SOURCE_DIR}/extension.cmake
)

#[[add_library(exqudensTestLibraryInterfaceImported INTERFACE IMPORTED)
target_include_directories(exqudensTestLibraryInterfaceImported INTERFACE
    $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_OUTPUT_DIRECTORY>
)
add_dependencies(exqudensTestLibraryInterfaceImported project-build)]]

add_library(testLibraryInterface INTERFACE
    src/test/cpp/exqudens/other/OtherTests.hpp
)
target_include_directories(testLibraryInterface INTERFACE
    $<TARGET_PROPERTY:submodule-exqudens-test,INCLUDE_DIRECTORIES>
    src/test/cpp
)
#[[target_link_libraries(testLibraryInterface INTERFACE
    exqudensTestLibraryInterfaceImported
)]]
add_dependencies(testLibraryInterface submodule-exqudens-test)

add_executable(TestExecutableShared src/test/cpp/main.cpp)
target_link_libraries(TestExecutableShared
    testLibraryInterface
)
set_target_properties(TestExecutableShared PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build/test/${PROJECT_NAME}-${PROJECT_VERSION}/bin"
)
foreach(fileName
    "libstdc++-6.dll"
    "libgcc_s_seh-1.dll"
    "libwinpthread-1.dll"
)
    if(EXISTS ${CXX_COMPILER_DIR}/${fileName})
        add_custom_command(
            TARGET TestExecutableShared
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND}
            ARGS -E copy ${CXX_COMPILER_DIR}/${fileName} $<TARGET_FILE_DIR:TestExecutableShared>
            BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/build/test/${PROJECT_NAME}-${PROJECT_VERSION}/bin/${fileName}
            COMMENT "Copying '${fileName}' shared library."
        )
    endif()
endforeach()
add_dependencies(TestExecutableShared testLibraryInterface)

add_custom_target(project-test
    COMMAND           ${CMAKE_COMMAND}
                      -E
                      env
                      PATH=$<TARGET_FILE_DIR:TestExecutableShared>
                      $<TARGET_FILE:TestExecutableShared>
                      -ff
                      -t
                      all
                      #exqudens.other.testFunction1
                      #exqudens.other.testFunction2
                      #exqudens.other.OtherTests.testMethod1
                      #exqudens.other.OtherTests.testMethod2
    WORKING_DIRECTORY $<TARGET_FILE_DIR:TestExecutableShared>
    COMMENT           "Testing project."
)
add_dependencies(project-test submodule-exqudens-test-clean TestExecutableShared)

######################################################################
# install ############################################################
######################################################################

add_custom_targets_install(
    project-install
    ""
    project-test
    ${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}
    ${CMAKE_INSTALL_PREFIX}
    ${CMAKE_COMMAND}
)

######################################################################
# upload #############################################################
######################################################################

add_custom_targets_upload_file(
    project-upload
    ""
    project-test
    ${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}.zip
    file:///${REPOSITORY_PREFIX}/${PROJECT_NAME}-${PROJECT_VERSION}.zip
    ${CMAKE_COMMAND}
    ${CMAKE_CURRENT_SOURCE_DIR}/extension.cmake
)

######################################################################
# info ###############################################################
######################################################################

add_custom_target(project-info
    COMMAND ${CMAKE_COMMAND} -E echo "1:" $<TARGET_NAME_IF_EXISTS:project-info>
    COMMAND ${CMAKE_COMMAND} -E echo "2:" $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_SOURCES>
    COMMAND ${CMAKE_COMMAND} -E echo "3:" $<TARGET_PROPERTY:exqudensTestLibraryInterface,SOURCES>
    COMMAND ${CMAKE_COMMAND} -E echo "4:" $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_INCLUDE_DIRECTORIES>
    COMMAND ${CMAKE_COMMAND} -E echo "5:" $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_OUTPUT_DIRECTORY>
    COMMAND ${CMAKE_COMMAND} -E echo "6:" $<TARGET_FILE_DIR:TestExecutableShared>
    COMMAND ${CMAKE_COMMAND} -E echo "7:" $<TARGET_NAME_IF_EXISTS:test>
    COMMAND ${CMAKE_COMMAND} -E echo "8:" $<TARGET_PROPERTY:exqudensTestLibraryInterface,BUILD_OUTPUT_DIRECTORY>
    COMMAND ${CMAKE_COMMAND} -E echo "11:" $<TARGET_PROPERTY:submodule-exqudens-test,INCLUDE_DIRECTORIES>
    COMMAND ${CMAKE_COMMAND} -E echo "12:" $<TARGET_PROPERTY:submodule-exqudens-test,LINK_DIRECTORIES>
    COMMAND ${CMAKE_COMMAND} -E echo "13:" $<TARGET_PROPERTY:submodule-exqudens-test,LIBRARIES>
    COMMENT "Project info."
)
