#version 450
// Set 0: Global Set
// Set 1: Instance Set
// Set 2: SubMesh Set
// Set 3: Texture Set
// Set 4: Material Set (Not used now)

layout(set = 1, binding = 0) uniform Model {
    mat4 model;
} model;
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * model.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}