#version 330 core
out vec4 FragColor;
  
//in vec4 myColor;
in vec2 TexCoord;
flat in int texIndex;

uniform sampler2D[16] myTexture;

void main()
{
	vec4 tex = texture(myTexture[0], TexCoord);// * myColor; 
    if (tex.a < .01f) discard;
	FragColor = tex;
}