######################################################################
# configuration ######################################################
######################################################################

cmake_minimum_required(VERSION 3.19)
project(exqudens-test VERSION 1.0.0)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extension.cmake")
    file(
        DOWNLOAD
            #"https://raw.githubusercontent.com/exqudens/exqudens-cmake/8.1.0/src/main/cmake/extension.cmake"
            "https://raw.githubusercontent.com/exqudens/exqudens-cmake/master/src/main/cmake/extension.cmake"
            "${CMAKE_CURRENT_SOURCE_DIR}/extension.cmake"
        EXPECTED_MD5
            "b7bdd8a403ba4a9ccbf2a1793bff07d6"
    )
endif()
include(extension.cmake)

get_filename_component(CXX_COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
set_home_prefix_if_not_defined(HOME_PREFIX)
set_if_not_defined(INSTALL_PREFIX ${HOME_PREFIX}/.cmake/packages)
set_if_not_defined(REPOSITORY_PREFIX ${HOME_PREFIX}/.cmake/repository)

set(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX})
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
    BUILD_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}"
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

######################################################################
# test ###############################################################
######################################################################

add_library(exqudensTestLibraryInterfaceImported INTERFACE IMPORTED)
target_include_directories(exqudensTestLibraryInterfaceImported INTERFACE
    $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_OUTPUT_DIRECTORY>
)
add_dependencies(exqudensTestLibraryInterfaceImported project-build)

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
    COMMAND ${CMAKE_COMMAND} -E echo "building" $<TARGET_NAME_IF_EXISTS:copyTestExecutableSharedLibraries>
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:TestExecutableShared>
    #COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_PROPERTY:TestExecutableShared,RESOURCE_OUTPUT_DIRECTORY>
    #COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src/test/resources $<TARGET_PROPERTY:TestExecutableShared,RESOURCE_OUTPUT_DIRECTORY>
)
foreach(fileName
    "libstdc++-6.dll"
    "libgcc_s_seh-1.dll"
    "libwinpthread-1.dll"
)
    if(EXISTS ${CXX_COMPILER_DIR}/${fileName})
        add_custom_command(
            TARGET copyTestExecutableSharedLibraries
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND}
            ARGS -E copy ${CXX_COMPILER_DIR}/${fileName} $<TARGET_FILE_DIR:TestExecutableShared>
            BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/build/test/${PROJECT_NAME}-${PROJECT_VERSION}/bin/${fileName}
        )
    endif()
endforeach()
add_dependencies(TestExecutableShared testLibraryInterface copyTestExecutableSharedLibraries)

add_custom_target(project-test
    COMMAND           ${CMAKE_COMMAND}
                      -E
                      env
                      PATH=$<TARGET_FILE_DIR:TestExecutableShared>
                      $<TARGET_FILE:TestExecutableShared>
                      -t
                      exqudens.other.OtherTests.test1
                      exqudens.other.OtherTests.test2
    WORKING_DIRECTORY $<TARGET_FILE_DIR:TestExecutableShared>
    COMMENT           "Testing project."
)
add_dependencies(project-test TestExecutableShared)

######################################################################
# install ############################################################
######################################################################

add_custom_command(
    OUTPUT  ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}-${PROJECT_VERSION}
    COMMAND ${CMAKE_COMMAND}
    ARGS    -E
            make_directory
            ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}-${PROJECT_VERSION}
    COMMAND ${CMAKE_COMMAND}
    ARGS    -E
            copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}
            ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}-${PROJECT_VERSION}
    COMMENT "Installing project."
)
add_custom_target(project-install
    DEPENDS ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}-${PROJECT_VERSION}
)
add_dependencies(project-install project-test)

######################################################################
# upload #############################################################
######################################################################

add_custom_target(project-pack
    COMMAND           ${CMAKE_COMMAND}
                      -E
                      tar
                      cfv
                      ${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}.zip
                      --format=zip
                      --
                      ${PROJECT_NAME}-${PROJECT_VERSION}
    BYPRODUCTS        ${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}.zip
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build/main
    COMMENT           "Packaging project."
)
add_dependencies(project-pack project-build)

add_custom_target(project-upload
    COMMAND ${CMAKE_COMMAND}
            -P
            ${CMAKE_CURRENT_SOURCE_DIR}/extension.cmake
            file_upload
            ${CMAKE_CURRENT_BINARY_DIR}/build/main/${PROJECT_NAME}-${PROJECT_VERSION}.zip
            file:///${REPOSITORY_PREFIX}/${PROJECT_NAME}-${PROJECT_VERSION}.zip
    COMMENT "Uploading project."
)
add_dependencies(project-upload project-pack)

######################################################################
# info ###############################################################
######################################################################

add_custom_target(project-info
    COMMAND ${CMAKE_COMMAND} -E echo "1:" $<TARGET_NAME_IF_EXISTS:info>
    COMMAND ${CMAKE_COMMAND} -E echo "2:" $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_SOURCES>
    COMMAND ${CMAKE_COMMAND} -E echo "3:" $<TARGET_PROPERTY:exqudensTestLibraryInterface,SOURCES>
    COMMAND ${CMAKE_COMMAND} -E echo "4:" $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_INCLUDE_DIRECTORIES>
    COMMAND ${CMAKE_COMMAND} -E echo "5:" $<TARGET_PROPERTY:exqudensTestLibraryInterface,INTERFACE_OUTPUT_DIRECTORY>
    COMMAND ${CMAKE_COMMAND} -E echo "6:" $<TARGET_FILE_DIR:TestExecutableShared>
    COMMAND ${CMAKE_COMMAND} -E echo "7:" $<TARGET_NAME_IF_EXISTS:test>
    COMMAND ${CMAKE_COMMAND} -E echo "8:" $<TARGET_PROPERTY:exqudensTestLibraryInterface,BUILD_OUTPUT_DIRECTORY>
    COMMENT "Project info."
)
