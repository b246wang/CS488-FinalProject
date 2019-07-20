#version 330

in vec3 position;
in vec2 vertexUV;
out vec2 UV;
out vec4 lightSpace;

uniform mat4 PVM;
uniform mat4 lightMatrix;

void main() {
	gl_Position = PVM * vec4(position, 1.0);
    UV = vertexUV;
    lightSpace = lightMatrix  * vec4(position, 1.0);
}
