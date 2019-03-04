#pragma once

#include "IWindow.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// #if __cplusplus >= 2001103L
//
#include "common.h"
#include <map>

// #else

// #error "not corrected c++ standart ( c++11 )"

// #endif

typedef std::function<void(GLFWwindow *window, int key, int scancode, int action, int mode)> kcbfunc;

class CWindow : public IWindow {
private:
  static constexpr int DEFAULT_WIDTH = 1024;
  static constexpr int DEFAULT_HEIGHT = 768;
  static constexpr char *DEFAULT_TITLE = "GLFW Window";

  GLFWwindow* m_Window; 
  bool m_bClose;
  int m_Width;
  int m_Height;
  char *m_Title;
  GLfloat m_BackColor[4] = { 0.5, 0.3, 0.8, 1.0 };

public:
  CWindow(char*, int, int);
  ~CWindow();
  bool create();
  bool init();
  void update();
  void clear();
  bool closed();
  void swap();
  void setTitle(char*);
  void show();

private:
	std::map<int, kcbfunc> kcbfuncs;
	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
};
