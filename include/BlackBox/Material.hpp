#pragma once
#include <memory>
#include <BlackBox/Render/Texture.hpp>
#include <BlackBox/Render/CShader.hpp>
#include <glm/glm.hpp>
#include <vector>

class CShader;
class CBaseShaderProgram;
class Object;
class CCamera;

struct Material
{
  BaseTexture
  /**diffuse,*/
  *specular,
  *bump,
  *normal,
  *mask;
  float shininess;
  glm::vec3 diffuseColor;
	
	std::vector<BaseTexture*> diffuse;

  std::shared_ptr<CShaderProgram> program;
	std::string program_name;
  std::shared_ptr<std::string> name;
  bool hasTexture = false;
  bool enabledNormal = true;
	int current_diffuse = 0;

  Material() : 
  //diffuse(nullptr),
  specular(nullptr),
  bump(nullptr),
  normal(nullptr),
  mask(nullptr),
  diffuseColor({0,0,0}),
  shininess(64.0f),
  program(nullptr)
  {

  }
  void apply(Object *object, CCamera *camera);
  void setTexture(Texture *texture, const char *type);
	void nextDiffuse();
	void prevDiffuse();
private:
  void activeTexture(uint32_t block, const char *uniform, BaseTexture *texture);
};
