#version 450

#extension GL_EXT_nonuniform_qualifier : require

// layout(set = TEXUTRE_SET, binding = 0) uniform sampler2D diffuses[];     // For mesh // set = 1
// layout(set = GLOBAL_SET, binding = 0) uniform struct MV;                 // For camera // set = 0
// layout(set = MATID_SET, binding = 0) struct { uint matIDs[]; };          // For submesh // set = 2

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(set = 2, binding = 0) uniform MaterialID{
    uint matIDs[];
} matIDs;
layout(set = 0, binding = 1) uniform sampler2D diffuses[];

layout(location = 0) out vec4 outColor;

void main() {
    //    outColor = vec4(fragTexCoord, 0.0, 1.0);
//    outColor = texture(diffuse, fragTexCoord);
    uint matID = matIDs.matIDs[gl_PrimitiveID];
    outColor = texture(diffuses[matID], fragTexCoord);
}