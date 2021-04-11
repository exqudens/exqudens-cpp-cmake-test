# declare script commands
if("file_upload" STREQUAL "${CMAKE_ARGV3}")
    # file = ${CMAKE_ARGV4}
    # url = ${CMAKE_ARGV5}
    file(UPLOAD ${CMAKE_ARGV4} ${CMAKE_ARGV5})
elseif("file_download" STREQUAL "${CMAKE_ARGV3}")
    # url = ${CMAKE_ARGV4}
    # file = ${CMAKE_ARGV5}
    file(DOWNLOAD ${CMAKE_ARGV4} ${CMAKE_ARGV5})
endif()

# declare macro 'add_custom_targets_upload_file'
macro(add_custom_targets_upload_file
    name
    clean
    depends
    file
    url
    command
    script
)
    add_custom_target(${name}
        COMMAND ${command} -P ${script} file_upload ${file} ${url}
        COMMENT "Upload file '${file}'"
    )
    if(NOT "" STREQUAL "${depends}")
        foreach(d ${depends})
            add_dependencies(${name} ${d})
        endforeach()
    endif()
    if(NOT "" STREQUAL "${clean}")
        add_custom_target(${name}-clean
            COMMAND ${command} -E echo "Nothing to clean"
            COMMENT "Clean upload file '${file}'"
        )
    endif()
endmacro()

# declare macro 'add_custom_targets_download_file'
macro(add_custom_targets_download_file
    name
    clean
    depends
    url
    file
    command
    script
)
    add_custom_target(${name}
        COMMAND ${command} -P ${script} file_download ${url} ${file}
        COMMENT "Download file '${url}'"
    )
    if(NOT "" STREQUAL "${depends}")
        foreach(d ${depends})
            add_dependencies(${name} ${d})
        endforeach()
    endif()
    if(NOT "" STREQUAL "${clean}")
        add_custom_target(${name}-clean
            COMMAND ${command} -E rm -f ${file}
            COMMENT "Clean download file '${url}'"
        )
    endif()
endmacro()

# declare macro 'add_custom_targets_zip_directory'
macro(add_custom_targets_zip_directory
    name
    clean
    depends
    input
    output
    command
)
    add_custom_target(${name}
        COMMAND ${command} -E tar cfv ${output} --format=zip -- .
        BYPRODUCTS ${output}
        WORKING_DIRECTORY ${input}
        COMMENT "Zip directory '${input}'"
    )
    if(NOT "" STREQUAL "${depends}")
        foreach(d ${depends})
            add_dependencies(${name} ${d})
        endforeach()
    endif()
    if(NOT "" STREQUAL "${clean}")
        add_custom_target(${name}-clean
            COMMAND ${command} -E rm -f ${output}
            COMMENT "Clean zip directory '${input}'"
        )
    endif()
endmacro()

# declare macro 'add_custom_targets_unzip_directory'
macro(add_custom_targets_unzip_directory
    name
    clean
    depends
    input
    output
    command
)
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${command}
        ARGS -E make_directory ${output}
        BYPRODUCTS ${output}
    )
    add_custom_target(${name}
        COMMAND ${command} -E tar xzf ${input}
        DEPENDS ${output}
        WORKING_DIRECTORY ${output}
        COMMENT "Unzip directory '${input}'"
    )
    if(NOT "" STREQUAL "${depends}")
        foreach(d ${depends})
            add_dependencies(${name} ${d})
        endforeach()
    endif()
    if(NOT "" STREQUAL "${clean}")
        add_custom_target(${name}-clean
            COMMAND ${command} -E rm -Rf ${output}
            COMMENT "Clean unzip directory '${input}'"
        )
    endif()
endmacro()

# declare macro 'add_custom_targets_install'
macro(add_custom_targets_install
    name
    clean
    depends
    input
    output
    command
)
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${command}
        ARGS -E make_directory ${output}
        COMMAND ${command}
        ARGS -E copy_directory ${input} ${output}
        COMMENT "Install '${input}'"
    )
    add_custom_target(${name}
        DEPENDS ${output}
    )
    if(NOT "" STREQUAL "${depends}")
        foreach(d ${depends})
            add_dependencies(${name} ${d})
        endforeach()
    endif()
    if(NOT "" STREQUAL "${clean}")
        add_custom_target(${name}-clean
            COMMAND ${command} -E rm -Rf ${output}
            COMMENT "Clean install '${input}'"
        )
    endif()
endmacro()

# declare macro 'add_custom_targets_dependency'
macro(add_custom_targets_dependency
    name
    clean
    depends
    url
    include
    link
    libraries
    output
    command
    script
)
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${command}
        ARGS -E make_directory ${output}
        BYPRODUCTS ${output}
    )
    add_custom_command(
        OUTPUT ${output}/archive.zip
        COMMAND ${command}
        ARGS -P ${script} file_download ${url} ${output}/archive.zip
        DEPENDS ${output}
        BYPRODUCTS ${output}/archive.zip
    )
    add_custom_target(${name}
        COMMAND ${command} -E tar xzf ${output}/archive.zip
        DEPENDS ${output}/archive.zip
        WORKING_DIRECTORY ${output}
        COMMENT "Dependency '${name}'"
    )

    if(NOT "" STREQUAL "${include}")
        set(add_custom_target_dependency_include "")
        foreach(i ${include})
            list(APPEND add_custom_target_dependency_include ${output}/${i})
        endforeach()
        set_property(TARGET ${name} PROPERTY
            INCLUDE_DIRECTORIES ${add_custom_target_dependency_include}
        )
        unset(add_custom_target_dependency_include)
    endif()

    if(NOT "" STREQUAL "${link}")
        set(add_custom_target_dependency_link "")
        foreach(l ${link})
            list(APPEND add_custom_target_dependency_link ${output}/${l})
        endforeach()
        set_property(TARGET ${name} PROPERTY
            LINK_DIRECTORIES ${add_custom_target_dependency_link}
        )
        unset(add_custom_target_dependency_link)
    endif()

    if(NOT "" STREQUAL "${libraries}")
        set(add_custom_target_dependency_libraries "")
        foreach(l ${libraries})
            list(APPEND add_custom_target_dependency_libraries ${l})
        endforeach()
        set_property(TARGET ${name} PROPERTY
            LIBRARIES ${add_custom_target_dependency_libraries}
        )
        unset(add_custom_target_dependency_libraries)
    endif()

    if(NOT "" STREQUAL "${depends}")
        foreach(d ${depends})
            add_dependencies(${name} ${d})
        endforeach()
    endif()
    if(NOT "" STREQUAL "${clean}")
        add_custom_target(${name}-clean
            COMMAND ${command} -E rm -Rf ${output}
            COMMENT "Clean dependency '${name}'"
        )
    endif()
endmacro()

# declare function 'set_if_not_defined'
function(set_if_not_defined variableName)
    if(NOT DEFINED ${variableName})
        if(${ARGC} EQUAL 2)
            set(${variableName} ${ARGV1} PARENT_SCOPE)
        elseif(${ARGC} GREATER 2)
            set(values)
            foreach(argv ${ARGV})
                if(NOT "${argv}" STREQUAL "${variableName}")
                    list(APPEND values "${argv}")
                endif()
            endforeach()
            set(${variableName} ${values} PARENT_SCOPE)
        else()
            set(${variableName} PARENT_SCOPE)
        endif()
    endif()
endfunction()

# declare function 'set_home_prefix'
function(set_home_prefix variableName)
    if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
        file(TO_CMAKE_PATH "$ENV{USERPROFILE}" homePrefix)
    elseif("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
        file(TO_CMAKE_PATH "$ENV{HOME}" homePrefix)
    elseif("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Darwin")
        file(TO_CMAKE_PATH "$ENV{HOME}" homePrefix)
    else()
        message(
            FATAL_ERROR
            "Unable to get 'homePrefix' for CMAKE_HOST_SYSTEM_NAME: '${CMAKE_HOST_SYSTEM_NAME}'"
        )
    endif()
    set("${variableName}" "${homePrefix}" PARENT_SCOPE)
endfunction()

# declare function 'set_home_prefix_if_not_defined'
function(set_home_prefix_if_not_defined variableName)
    if(NOT DEFINED ${variableName})
        set_home_prefix(homePrefix)
        set(${variableName} ${homePrefix} PARENT_SCOPE)
    endif()
endfunction()
