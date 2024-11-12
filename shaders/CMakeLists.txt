# Custom target will always cause its dependencies to be evaluated and is
# run by default

#add_custom
#
#add_custom_command(
#    OUTPUT
#        ${CMAKE_CURRENT_BINARY_DIR}/shaders/simple_shader.task.spv
#        ${CMAKE_CURRENT_BINARY_DIR}/shaders/simple_shader.mesh.spv
#        ${CMAKE_CURRENT_BINARY_DIR}/shaders/simple_shader.frag.spv
#    POST_BUILD
#    COMMAND echo "Building shaders..."
#    COMMAND glslc ${CMAKE_CURRENT_SOURCE_DIR}/shaders/simple_shader.task -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/simple_shader.task.spv --target-spv=spv1.5
#    COMMAND glslc ${CMAKE_CURRENT_SOURCE_DIR}/shaders/simple_shader.mesh -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/simple_shader.mesh.spv --target-spv=spv1.5
#    COMMAND glslc -fshader-stage=frag ${CMAKE_CURRENT_SOURCE_DIR}/shaders/simple_shader.frag -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/simple_shader.frag.spv --target-spv=spv1.5
#    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders
#    DEPENDS
#        always_rebuild
#)

include(CMakePrintHelpers)


function(add_shaders TARGET_NAME)
    set(SHADER_SOURCE_FILES ${ARGN}) # the rest of arguments to this function will be assigned as shader source files

    # Validate that source files have been passed
    list(LENGTH SHADER_SOURCE_FILES FILE_COUNT)
    if(FILE_COUNT EQUAL 0)
        message(FATAL_ERROR "Cannot create a shaders target without any source files")
    endif()

    set(SHADER_COMMANDS)
    set(SHADER_PRODUCTS)

    foreach(SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
        get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)

        # Build command
        list(APPEND SHADER_COMMAND COMMAND)
        list(APPEND SHADER_COMMAND Vulkan::glslc)
        list(APPEND SHADER_COMMAND "${SHADER_SOURCE}")
        list(APPEND SHADER_COMMAND "--target-spv=spv1.5")
        list(APPEND SHADER_COMMAND "-o")
        list(APPEND SHADER_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}.spv")

        # Add product
        list(APPEND SHADER_PRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}.spv")

        list(APPEND SHADER_COMMANDS ${SHADER_COMMAND})

    endforeach()

    add_custom_target(${TARGET_NAME} ALL
            ${SHADER_COMMANDS}
            COMMENT "Compiling Shaders [${TARGET_NAME}]"
            SOURCES ${SHADER_SOURCE_FILES}
            BYPRODUCTS ${SHADER_PRODUCTS}
    )
endfunction()

add_shaders( shader_programs
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/simple_shader.mesh
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/simple_shader.task
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/simple_shader.frag
)