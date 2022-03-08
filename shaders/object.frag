#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) readonly buffer MaterialID{
    uint matIDs[];
} matIDs;
layout(set = 3, binding = 0) uniform sampler2D diffuses[];

void main() {
    uint matID = matIDs.matIDs[gl_PrimitiveID];
    outColor = texture(diffuses[matID], fragTexCoord);
}