#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

#ifdef STORE_TEXCOORDS
layout(binding = 0, std430) buffer myBuffer
{
    float nodes [];
};
#endif

out vec2 TexCoords;
uniform mat4 projection = mat4(1.0);
uniform mat4 model = mat4(1.0);
uniform mat4 uv_projection = mat4(1.0);

void main()
{
    gl_Position = projection * model * vec4(aPos.x, aPos.y, 0.5f, 1.0f); 
    //TexCoords = (uv_transform * vec3(aTexCoords, 1.0)).xy;
    TexCoords = (uv_projection * vec4(aTexCoords, 1.0, 1.0)).xy;
	#ifdef STORE_TEXCOORDS
	nodes[gl_VertexID] = TexCoords;
	#endif
} 