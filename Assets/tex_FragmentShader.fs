#version 330

in vec2 UV;
out vec4 fragColour;

uniform sampler2D tex;

void main() {
	fragColour = texture(tex, UV);
}
