#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;

layout (location = 0) out vec3 outColor;

layout(set = 0, binding = 0) uniform  CameraBuffer{
	mat4 view;
	mat4 proj;
	mat4 view_proj;
    vec3 position;
} cameraData;

layout( push_constant ) uniform constants
{
    mat4 render_matrix;
    vec4 normal_color;
} PushConstants;

void main()
{
    mat4 transformMatrix = (cameraData.view_proj * PushConstants.render_matrix);
    gl_Position = transformMatrix * vec4(vPosition, 1.0f);
    outColor = (PushConstants.normal_color[3] < 1) ? vColor : vec3(PushConstants.normal_color[0], PushConstants.normal_color[1], PushConstants.normal_color[2]);
}
