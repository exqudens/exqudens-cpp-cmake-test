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
