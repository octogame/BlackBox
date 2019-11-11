#pragma once
#include <BlackBox/Render/BaseTexture.hpp>
#include <BlackBox/IGeometry.hpp>
#include <vector>

class Terrain
{
public:
	struct Face
	{
		Vertex v[3];
	};
	Terrain()
	{

	}
	Face getFace(size_t v1, size_t v2, size_t v3);
	Object* load(const char* heightmap);
	void draw();

private:
	float getHeight(int x, int y);

	std::vector<Vertex> vb;
	Image img;
	GLuint VBO, VAO;
	int size;


};