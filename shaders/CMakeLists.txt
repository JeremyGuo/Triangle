macro(target_shader target shader_name suffix)
    add_custom_command(
        OUTPUT ${shader_name}.${suffix}.spv.command
        COMMAND glslc ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shader_name}.${suffix} -o ${shader_name}.${suffix}.spv
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shader_name}.${suffix}
        COMMENT "Compile shader ${SHADER_PATH}/${shader_name}.${suffix}"
    )
    add_custom_target(${shader_name}.${suffix}.spv
        DEPENDS ${shader_name}.${suffix}.spv.command)
    add_dependencies(${target} ${shader_name}.${suffix}.spv)
endmacro()
