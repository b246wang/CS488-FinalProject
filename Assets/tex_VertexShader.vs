#version 330

in vec3 position;
in vec2 vertexUV;
out vec2 UV;

uniform mat4 PVM;

void main() {
	gl_Position = PVM * vec4(position, 1.0);
    UV = vertexUV;
}
