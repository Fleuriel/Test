/* !
@file    glapp.h
@author  pghali@digipen.edu
@date    10/11/2016

This file contains the declaration of class GLApp that encapsulates the
functionality required to create an OpenGL context using GLFW; use GLEW
to load OpenGL extensions; initialize OpenGL state; and finally initialize
the OpenGL application by calling initalization functions associated with
objects participating in the application.

*//*__________________________________________________________________________*/

/*                                                                      guard
----------------------------------------------------------------------------- */
#ifndef GLAPP_H
#define GLAPP_H

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <GL/glew.h> // for access to OpenGL API declarations 
#include <GLFW/glfw3.h>
#include <string>

/*  _________________________________________________________________________ */
struct GLApp
  /*! GLApp structure to encapsulate initialization stuff ...
  */
{
  static bool init(GLint w, GLint h, std::string t);
  static void update();
  static void draw();
  static void cleanup();


  static void update_time(double fpsCalcInt = 1.0);

  static GLint width, height;
  static GLdouble fps;
  static GLdouble delta_time; // time taken to complete most recent game loop
  static std::string title;
  static GLFWwindow *ptr_window;
};

#endif /* GLAPP_H */
