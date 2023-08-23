/*                                                                   includes
----------------------------------------------------------------------------- */
#include "../include/glapp.h"

#include <glslshader.h>								//OpenGL libraries and addons
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glhelper.h>


#include <iostream>
#include <sstream>
#include <iomanip>
#include <array>
#include <vector>
#include <iomanip>
#include <string>
#include <random>
#include <list>

#define MAX_OBJECTS 32768

// Definition for Keys
#include <keyDefinition.h>

/*                                                   objects with file scope
----------------------------------------------------------------------------- */
GLint GLApp::width;
GLint GLApp::height;
GLdouble GLApp::fps;
GLdouble GLApp::delta_time;
std::string GLApp::title;
GLFWwindow* GLApp::ptr_window;
#define WORLD_WIDTH 10000.0f
#define WORLD_HEIGHT 10000.0f

//creating random seed and generator
std::random_device rd;// get random seed
std::default_random_engine random(rd());// Standard mersenne_twister_engine seeded with rd()

using VPSS = std::vector<std::pair<std::string, std::string>>;

void init_models_cont(); 
void init_shdrpgms_cont(VPSS const& vpss);
bool _isCapacityMax = false;


struct GLModel {
	GLenum primitive_type;
	GLuint primitive_cnt;
	GLuint vaoid;
	GLuint draw_cnt;
	GLuint model_cnt;
	GLModel() : primitive_type(0), primitive_cnt(0), vaoid(0), draw_cnt(0), model_cnt(0) {}

};

GLModel Box_Model();

struct GLObject {
	glm::vec2 scaling;				// scaling
	GLfloat angle_speed, angle_disp;	// orientation
	glm::vec2 position;				// translation
	glm::mat3 mdl_to_ndc_xform;
	GLuint mdl_ref, shd_ref;

	// set up initial state
	void init();
	void draw() const;
	void update(GLdouble delta_time);
};

std::vector<GLModel> models{}; // singleton
std::list<GLObject> objects{};			// Declaration of List GLApp::GLObject
std::vector<GLSLShader> shdrpgms{};			// Declaration of Vector Container of GLSL Shader




float rand_uniform_float(float min, float max)
{
	std::uniform_real_distribution<float> rand_float_value(min, std::nextafter(max, std::numeric_limits<float>::max()));
	return rand_float_value(random);
}

float rand_float(float min = 0.0f, float max = 1.0f)
{
	return min + ((max - min) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)));
}


int rand_int(int min, int max)
{
	std::uniform_int_distribution<int> rand_int_value(min, static_cast<int>(std::nextafter(max, std::numeric_limits<float>::max())));
	return rand_int_value(random);
}

/*  _________________________________________________________________________ */
/*! init



@param GLint width
@param GLint height
Dimensions of window requested by program

@param std::string title_str
String printed to window's title bar

@return bool
true if OpenGL context and GLEW were successfully initialized.
false otherwise.

Uses GLFW to create OpenGL context. GLFW's initialization follows from here:
http://www.glfw.org/docs/latest/quick.html
a window of size width x height pixels
and its associated OpenGL context that matches a core profile that is
compatible with OpenGL 4.5 and doesn't support "old" OpenGL, has 32-bit RGBA,
double-buffered color buffer, 24-bit depth buffer and 8-bit stencil buffer
with each buffer of size width x height pixels
*/
bool GLApp::init(GLint w, GLint h, std::string t) {
  GLApp::width = w;
  GLApp::height = h;
  GLApp::title = t;

  if (!glfwInit()) {
    std::cout << "GLFW init has failed - abort program!!!" << std::endl;
    return false;
  }
  // In case a GLFW function fails, an error is reported to callback function
  //glfwSetErrorCallback(GLApp::error_cb);

  // Before asking GLFW to create an OpenGL context, we specify the minimum constraints
  // in that context:

  // specify OpenGL version 4.5
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  // specify modern OpenGL only - no compatibility with "old" OpenGL
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // applications will be double-buffered ...
  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
  // default behavior: colorbuffer is 32-bit RGBA, depthbuffer is 24-bits
  // don't change size of window
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // size of viewport: width x height
  GLApp::ptr_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
  if (!GLApp::ptr_window) {
    std::cerr << "GLFW unable to create OpenGL context - abort program\n";
    glfwTerminate();
    return false;
  }

  // make the previously created OpenGL context current ...
  glfwMakeContextCurrent(GLApp::ptr_window);

  // set callback for events associated with window size changes; keyboard;
 // mouse buttons, cursor position, and scroller
 //glfwSetFramebufferSizeCallback(GLApp::ptr_window, GLApp::fbsize_cb);
 //glfwSetKeyCallback(GLApp::ptr_window, GLApp::key_cb);
 //glfwSetMouseButtonCallback(GLApp::ptr_window, GLApp::mousebutton_cb);
 //glfwSetCursorPosCallback(GLApp::ptr_window, GLApp::mousepos_cb);
 //glfwSetScrollCallback(GLApp::ptr_window, GLApp::mousescroll_cb);

  // this is the default setting ...'
  glfwSetInputMode(GLApp::ptr_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  // initialize OpenGL (and extension) function loading library
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cerr << "Unable to initialize GLEW - error: "
      << glewGetErrorString(err) << " abort program" << std::endl;
    return false;
  }
  if (GLEW_VERSION_4_5) {
    std::cout << "Using glew version: " << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "Driver supports OpenGL 4.5\n" << std::endl;
  } else {
    std::cerr << "Driver doesn't support OpenGL 4.5 - abort program" << std::endl;
    return false;
  }

  // we'll use the entire window as viewport ...
  glm::ivec2 vp_width_height(0, 0);
  glfwGetFramebufferSize(ptr_window, &vp_width_height.x, &vp_width_height.y);

  init_models_cont();


  VPSS shdr_file_names{ // vertex & fragment shader files
	  std::make_pair<std::string, std::string>
	  ("../shaders/my-tutorial-3.vert", "../shaders/my-tutorial-3.frag")
  };

  init_shdrpgms_cont(shdr_file_names);



  return true;
}


void GLApp::update() {


    //Testing Shit




	if (GLHelper::LMouseState == GLFW_PRESS)
	{

		std::cout << "print\n";
		// Set LMouseState (GLHelper) to GL_FALSE
		GLHelper::LMouseState = false;

		// Check 1: Checks if object size is lesser than or equals to MAX_OBJECTS as set by the user (32768)
		// Check 2: Checks if _isCapacityMax is TRUE or FALSE
		if (objects.size() <= MAX_OBJECTS && _isCapacityMax == false)
		{

			// Spawn new object(s)
			// Multiply the number of objects by 2
			size_t currentObjectCount = objects.size();
			size_t newObjectCount = currentObjectCount * 2;

			// Spawn new objects
			for (size_t i = currentObjectCount; i < newObjectCount; i++)
			{
				GLObject newObject{};


				newObject.init();
				models[newObject.mdl_ref].model_cnt++;
				// ...
				objects.emplace_back(newObject);
			}


			// Initial Number of Objects to spawn on first click.
			// Have to be below the previous if statement as C++ is a top to bottom language.

#ifdef _DEBUG
	// Checks for the object size and see if the object is properly multiplied by its previous number.
			std::cout << GLApp::objects.size() << '\n';
#endif
			if (objects.size() == 0)
			{
				size_t numNewObjects = 1;
				for (size_t i = 0; i < numNewObjects; i++)
				{

					GLObject newObject{};

					newObject.init();

					models[newObject.mdl_ref].model_cnt++;
					// ...
					objects.emplace_back(newObject);
				}
			}

			// Check: Checks if object size is equals to the MAX_OBJECTS
			if (objects.size() >= MAX_OBJECTS)
			{
				_isCapacityMax = true;
			}
		}
		else if (objects.size() != 0 && _isCapacityMax)
		{

			// Kill oldest objects
			size_t numObjectsToKill = objects.size() / 2;  // Number of objects to kill

			for (size_t i = 0; i < numObjectsToKill; i++)
			{
				// if the container is not empty
				if (!objects.empty())
				{
					// model count decrement
					models[objects.front().mdl_ref].model_cnt--;
					objects.pop_front();  // Remove the oldest object from the front of the list
				}
			}

			// Flag to check if the size is 1
			if (objects.size() == 1)
			{
				_isCapacityMax = false;
			}
		}
	}
	else
	{
		// Mouse button is not pressed
		// Empty for now
	}








  // write window title with current fps ...
  std::stringstream sstr;
  sstr << std::fixed << std::setprecision(2) << GLApp::title << " | " << GLApp::fps;
  glfwSetWindowTitle(GLApp::ptr_window, sstr.str().c_str());
}


void GLApp::draw() {
  // clear colorbuffer with RGBA value in glClearColor ...
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(1.f, 1.f, 1.f, 1.f);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  for (auto const& x : objects)
  {
	  x.draw();
  }

}

void init_models_cont()
{
	models.emplace_back(Box_Model());
}

void GLObject::init()
{
	GLObject::mdl_ref = rand_int(0, 1);
	GLObject::shd_ref = 0;

	GLObject::position = glm::vec2{ rand_uniform_float(-1.f,1.f) * static_cast<float>(WORLD_WIDTH / 2), // x axis
									rand_uniform_float(-1.f,1.f) * static_cast<float>(WORLD_HEIGHT / 2) // y axis
	};

	//std::cout << "pos x : " << position.x << ", " << "pos y : " << position.y << '\n';

	// initialize initial angular displacement and angular speed of object
	GLfloat const max_rotation_speed = 30.f;

	//current rotation
	GLObject::angle_disp = rand_float() * 360.f; //in degree

	//rotation speed
	GLObject::angle_speed = rand_float() * max_rotation_speed; //in degree

	GLfloat const min_scale = 450.f;
	GLfloat const max_scale = 450.f;

	GLObject::scaling = glm::vec2{ rand_float(min_scale,max_scale), rand_uniform_float(min_scale,max_scale) };

	glm::mat3 Translate = glm::mat3
	{
		1, 0, position.x,
		0, 1, position.y,
		0, 0, 1
	};

	glm::mat3 Rotation = glm::mat3
	{
		cosf(glm::radians(angle_disp)), -sinf(glm::radians(angle_disp)), 0,
		sinf(glm::radians(angle_disp)), cosf(glm::radians(angle_disp)), 0,
		0, 0, 1
	};

	glm::mat3 Scale = glm::mat3
	{
		scaling.x, 0, 0,
		0, scaling.y, 0,
		0, 0, 1
	};

	glm::mat3 ScaleToWorldToNDC = glm::mat3
	{
		1 / (WORLD_WIDTH / 2), 0, 0,
		0, 1 / (WORLD_WIDTH / 2), 0,
		0, 0, 1
	};

	//scaling not required to transpose. They be the same after transpose
	mdl_to_ndc_xform = ScaleToWorldToNDC * glm::transpose(Translate) * glm::transpose(Rotation) * Scale;
}



void GLObject::draw() const
{

	shdrpgms[shd_ref].Use();
	// Part 2: Bind object's VAO handle
	glBindVertexArray(models[mdl_ref].vaoid); // Bind object's VAO handle

	// Part 3: Copy object's 3x3 model-to-NDC matrix to vertex shader
	GLint uniform_var_loc1 = glGetUniformLocation(shdrpgms[shd_ref].GetHandle(), "uModel_to_NDC");
	if (uniform_var_loc1 >= 0) {
		glUniformMatrix3fv(uniform_var_loc1, 1, GL_FALSE, glm::value_ptr(GLObject::mdl_to_ndc_xform));
	}
	else {
		std::cout << "Uniform variable doesn't exist!!!" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// Part 4: Render using glDrawElements or glDrawArrays
	glDrawElements(
		models[mdl_ref].primitive_type,
		models[mdl_ref].draw_cnt,
		GL_UNSIGNED_SHORT, NULL);

	// Part 5: Clean up
	glBindVertexArray(0); // Unbind the VAO
	shdrpgms[shd_ref].UnUse(); // Uninstall the shader program
}


void init_shdrpgms_cont(VPSS const& vpss) {

	for (auto const& x : vpss) {
		std::vector<std::pair<GLenum, std::string>> shdr_files;
		shdr_files.emplace_back(std::make_pair(GL_VERTEX_SHADER, x.first));
		shdr_files.emplace_back(std::make_pair(GL_FRAGMENT_SHADER, x.second));
		GLSLShader shdr_pgm;
		shdr_pgm.CompileLinkValidate(shdr_files);
		// insert shader program into container
		shdrpgms.emplace_back(shdr_pgm);
	}

}

void GLApp::cleanup() {
  glfwTerminate();
}






void GLApp::update_time(double fps_calc_interval) {
  // get elapsed time (in seconds) between previous and current frames
  static double prev_time = glfwGetTime();
  double curr_time = glfwGetTime();
  GLApp::delta_time = curr_time - prev_time;
  prev_time = curr_time;

  // fps calculations
  static double count = 0.0; // number of game loop iterations
  static double start_time = glfwGetTime();
  // get elapsed time since very beginning (in seconds) ...
  double elapsed_time = curr_time - start_time;

  ++count;

  // update fps at least every 10 seconds ...
  fps_calc_interval = (fps_calc_interval < 0.0) ? 0.0 : fps_calc_interval;
  fps_calc_interval = (fps_calc_interval > 10.0) ? 10.0 : fps_calc_interval;
  if (elapsed_time > fps_calc_interval) {
    GLApp::fps = count / elapsed_time;
    start_time = curr_time;
    count = 0.0;
  }
}



GLModel Box_Model()
{
	
	std::vector<glm::vec2> pos_vtx
	{
		glm::vec2(0.5f, -0.5f), glm::vec2(0.5f, 0.5f),
			glm::vec2(-0.5f, 0.5f), glm::vec2(-0.5f, -0.5f)
	};

	std::default_random_engine dre(std::random_device{}());
	std::uniform_real_distribution<float> urdf(0.f, std::nextafter(1.f, std::numeric_limits<float>::max()));
	std::vector<glm::vec3> clr_vtx;

	for (size_t i{}; i < pos_vtx.size(); i++)
	{
		float red = urdf(dre);
		float blue = urdf(dre);
		float green = urdf(dre);

		glm::vec3 color_to_push = { red, green, blue };
		clr_vtx.push_back(color_to_push);
	}

	GLModel mdl;
	// Allocating buffer objects
	// transfer vertex position and colowr attributes to VBO
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);

	//Allocating and filling data store
	glNamedBufferStorage(vbo_hdl,
		sizeof(glm::vec2) * pos_vtx.size() + sizeof(glm::vec3) * clr_vtx.size(),
		nullptr, GL_DYNAMIC_STORAGE_BIT);
	//transfer vertex position data
	glNamedBufferSubData(vbo_hdl, 0,
		sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data());
	//transfer vertex color data
	glNamedBufferSubData(vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(),
		sizeof(glm::vec3) * clr_vtx.size(), clr_vtx.data());

	GLuint vaoid;
	// encapsulate information about contents of VBO and VBO handlee
	// to another object called VAO
	glCreateVertexArrays(1, &vaoid); // vaoid is data member of GLApp::GLModel

	// for vertex position array, we use vertex attribute index 8
	// and vertex buffer binding point 3
	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayVertexBuffer(vaoid, 3, vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 0, 3);

	// Color Attributes
	// for vertex color array, we use vertex attribute index 9
	// and vertex buffer binding point 4
	glEnableVertexArrayAttrib(vaoid, 1);
	glVertexArrayVertexBuffer(vaoid, 4, vbo_hdl,
		sizeof(glm::vec2) * pos_vtx.size(), sizeof(glm::vec3));
	glVertexArrayAttribFormat(vaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 1, 4);

	// represents indices of vertices that will define 2 triangles with
	// counterclockwise winding
	std::array<GLushort, 6> idx_vtx{
		0, 1, 2,  // 1st triangle with counterclockwise winding is specified by
		// vertices in VBOs with indices 0,1,2
		2, 3, 0	  // 2nd trinagle with counter clockwise winding 
	};

	GLuint idx_elem_cnt;
	//get the number of indices in the idx_vtx array
	idx_elem_cnt = idx_vtx.size();

	//allocate an element buffer object and fill it with data from idx_vtx
	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl,
		sizeof(GLushort) * idx_elem_cnt,
		reinterpret_cast<GLvoid*>(idx_vtx.data()),
		GL_DYNAMIC_STORAGE_BIT);

	//attached the element buffer object to the vertex array object
	//and unbind the vertex array object
	glVertexArrayElementBuffer(vaoid, ebo_hdl);
	glBindVertexArray(0);

	mdl.vaoid = vaoid;
	mdl.primitive_type = GL_TRIANGLES;
	mdl.draw_cnt = idx_vtx.size();
	mdl.primitive_cnt = pos_vtx.size();
	return mdl;

}