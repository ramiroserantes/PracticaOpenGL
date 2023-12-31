// Copyright (C) 2020 Emilio J. Padrón
// Released as Free Software under the X11 License
// https://spdx.org/licenses/X11.html

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

// GLM library to deal with matrix operations <glm/glm.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include "textfile_ALT.h"

int gl_width = 640;
int gl_height = 480;

void glfw_window_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void render(double);

GLuint shader_program = 0; // shader program to set render pipeline
GLuint vao = 0; // Vertex Array Object to set input data
GLint model_location, view_location, proj_location; // Uniforms for transformation matrices
GLuint new_vao = 0; // Parte 2(Pirámide): Es necesario otro vao para la creación de la imágen.

// Posiciones para la luz y la vista
GLint light_pos_location, view_pos_location; 

// Creación de las variables de ambiente, difusa y especular para la luz
GLint light_ambient_location, light_diffuse_location, light_specular_location; 

// Parte 1(Other_Light): Creación de las variables de ambiente, difusa y especular para la luz (Other), y con su posición.
GLint other_light_ambient_location, other_light_diffuse_location, other_light_specular_location; 
GLint other_light_pos_location;

// Creación de las variables de ambiente, difusa, especular y brillo para el material
GLint material_diffuse_location, material_specular_location, material_ambient_location, material_shininess_location; 

GLint normal_to_world_location;

// Shader names
const char *vertexFileName = "spinningcube_withlight_vs.glsl";
const char *fragmentFileName = "spinningcube_withlight_fs.glsl";

// Camera
glm::vec3 camera_pos(0.0f, 0.0f, 3.0f);

// Lighting
glm::vec3 light_pos(1.2f, 1.0f, 2.0f);
glm::vec3 light_ambient(0.2f, 0.2f, 0.2f);
glm::vec3 light_diffuse(0.5f, 0.5f, 0.5f);
glm::vec3 light_specular(1.0f, 1.0f, 1.0f);

// Parte 1 (Other_Light): Especificación de las coordenadas de la otra luz.
glm::vec3 other_light_pos(-0.2f, 0.5f, 2.0f);
glm::vec3 other_light_ambient(0.2f, 0.2f, 0.2f);
glm::vec3 other_light_diffuse(0.5f, 0.5f, 0.5f);
glm::vec3 other_light_specular(1.0f, 1.0f, 1.0f);

// Material
glm::vec3 material_ambient(1.0f, 0.5f, 0.31f);
glm::vec3 material_diffuse(1.0f, 0.5f, 0.31f);
glm::vec3 material_specular(0.5f, 0.5f, 0.5f);
const GLfloat material_shininess = 32.0f;

int main() {
  // start GL context and O/S window using the GLFW helper library
  if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return 1;
  }

  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  //  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(gl_width, gl_height, "My spinning cube", NULL, NULL);
  if (!window) {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    return 1;
  }
  glfwSetWindowSizeCallback(window, glfw_window_size_callback);
  glfwMakeContextCurrent(window);

  // start GLEW extension handler
  // glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte* vendor = glGetString(GL_VENDOR); // get vendor string
  const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte* glversion = glGetString(GL_VERSION); // version as a string
  const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION); // version as a string
  printf("Vendor: %s\n", vendor);
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", glversion);
  printf("GLSL version supported %s\n", glslversion);
  printf("Starting viewport: (width: %d, height: %d)\n", gl_width, gl_height);

  // Enable Depth test: only draw onto a pixel if fragment closer to viewer
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS); // set a smaller value as "closer"

  // Vertex Shader
  char* vertex_shader = textFileRead(vertexFileName);

  // Fragment Shader
  char* fragment_shader = textFileRead(fragmentFileName);

  // Shaders compilation
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  free(vertex_shader);
  glCompileShader(vs);

  int  success;
  char infoLog[512];
  glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vs, 512, NULL, infoLog);
    printf("ERROR: Vertex Shader compilation failed!\n%s\n", infoLog);

    return(1);
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  free(fragment_shader);
  glCompileShader(fs);

  glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fs, 512, NULL, infoLog);
    printf("ERROR: Fragment Shader compilation failed!\n%s\n", infoLog);

    return(1);
  }

  // Create program, attach shaders to it and link it
  shader_program = glCreateProgram();
  glAttachShader(shader_program, fs);
  glAttachShader(shader_program, vs);
  glLinkProgram(shader_program);

  glValidateProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if(!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
    printf("ERROR: Shader Program linking failed!\n%s\n", infoLog);

    return(1);
  }

  // Release shader objects
  glDeleteShader(vs);
  glDeleteShader(fs);

  // Vertex Array Object
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Cube to be rendered
  //
  //          0        3
  //       7        4 <-- top-right-near
  // bottom
  // left
  // far ---> 1        2
  //       6        5
  //
  const GLfloat vertex_positions[] = {
    -0.25f, -0.25f, -0.25f, 0.00f, 0.00f, -1.00f, // 1
    -0.25f,  0.25f, -0.25f, 0.00f, 0.00f, -1.00f, // 0
     0.25f, -0.25f, -0.25f, 0.00f, 0.00f, -1.00f, // 2

     0.25f,  0.25f, -0.25f, 0.00f, 0.00f, -1.00f, // 3
     0.25f, -0.25f, -0.25f, 0.00f, 0.00f, -1.00f, // 2
    -0.25f,  0.25f, -0.25f, 0.00f, 0.00f, -1.00f,// 0

     0.25f, -0.25f, -0.25f, 1.00f, 0.00f, 0.00f, // 2
     0.25f,  0.25f, -0.25f, 1.00f, 0.00f, 0.00f, // 3
     0.25f, -0.25f,  0.25f, 1.00f, 0.00f, 0.00f, // 5

     0.25f,  0.25f,  0.25f, 1.00f, 0.00f, 0.00f, // 4
     0.25f, -0.25f,  0.25f, 1.00f, 0.00f, 0.00f, // 5
     0.25f,  0.25f, -0.25f, 1.00f, 0.00f, 0.00f, // 3

     0.25f, -0.25f,  0.25f, 0.00f, 0.00f, 1.00f, // 5
     0.25f,  0.25f,  0.25f, 0.00f, 0.00f, 1.00f,  // 4
    -0.25f, -0.25f,  0.25f, 0.00f, 0.00f, 1.00f,  // 6

    -0.25f,  0.25f,  0.25f, 0.00f, 0.00f, 1.00f,  // 7
    -0.25f, -0.25f,  0.25f, 0.00f, 0.00f, 1.00f,  // 6
     0.25f,  0.25f,  0.25f, 0.00f, 0.00f, 1.00f,  // 4

    -0.25f, -0.25f,  0.25f, -1.00f, 0.00f, 0.00f,  // 6
    -0.25f,  0.25f,  0.25f, -1.00f, 0.00f, 0.00f,  // 7
    -0.25f, -0.25f, -0.25f, -1.00f, 0.00f, 0.00f,  // 1

    -0.25f,  0.25f, -0.25f, -1.00f, 0.00f, 0.00f,  // 0
    -0.25f, -0.25f, -0.25f, -1.00f, 0.00f, 0.00f,  // 1
    -0.25f,  0.25f,  0.25f, -1.00f, 0.00f, 0.00f,  // 7

     0.25f, -0.25f, -0.25f, 0.00f, -1.00f, 0.00f,  // 2
     0.25f, -0.25f,  0.25f, 0.00f, -1.00f, 0.00f, // 5
    -0.25f, -0.25f, -0.25f, 0.00f, -1.00f, 0.00f, // 1

    -0.25f, -0.25f,  0.25f, 0.00f, -1.00f, 0.00f, // 6
    -0.25f, -0.25f, -0.25f, 0.00f, -1.00f, 0.00f, // 1
     0.25f, -0.25f,  0.25f, 0.00f, -1.00f, 0.00f, // 5

     0.25f,  0.25f,  0.25f, 0.00f, 1.00f, 0.00f, // 4
     0.25f,  0.25f, -0.25f, 0.00f, 1.00f, 0.00f, // 3
    -0.25f,  0.25f,  0.25f, 0.00f, 1.00f, 0.00f, // 7

    -0.25f,  0.25f, -0.25f,  0.00f, 1.00f, 0.00f, // 0
    -0.25f,  0.25f,  0.25f,  0.00f, 1.00f, 0.00f, // 7
     0.25f,  0.25f, -0.25f,  0.00f, 1.00f, 0.00f, // 3
  };

  // Vertex Buffer Object (for vertex coordinates)
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);

  // Incluír las normales en la representación.
  // Vertex attributes
  // 0: vertex position (x, y, z)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  // Vertex attributes
  // 1: vertex normals (x, y, z)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Unbind vbo (it was conveniently registered by VertexAttribPointer)
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Unbind vao
  glBindVertexArray(0);
  
  // Parte 2 (Pirámide): Declaración del Vertex Array Object, la base y vertices de la pirámide nueva con su normal, y la uniforme.
 
  glGenVertexArrays(1, &new_vao);
  glBindVertexArray(new_vao);
  
  const GLfloat new_vertex_positions[] = {
    // Base
    1.0f,  0.75f, 0.0f,    0.0f, -1.0f, 0.0f,     
    1.0f,  0.25f, 0.25f,   0.0f, -1.0f, 0.0f,     
    0.75f, 0.25f, -0.25f,  0.0f, -1.0f, 0.0f,     

    // Lados
    1.0f,  0.75f, 0.0f,    0.57735f, -0.57735f, -0.57735f, 
    0.75f, 0.25f, -0.25f,  0.57735f, -0.57735f, -0.57735f, 
    1.25f, 0.25f, -0.25f,  0.57735f, -0.57735f, -0.57735f,  

    1.0f,  0.75f, 0.0f,    0.57735f, -0.57735f, 0.57735f, 
    1.25f, 0.25f, -0.25f,  0.57735f, -0.57735f, 0.57735f, 
    1.0f,  0.25f, 0.25f,   0.57735f, -0.57735f, 0.57735f, 

    0.75f, 0.25f, -0.25f,  -0.57735f, -0.57735f, 0.57735f,  
    1.0f,  0.25f, 0.25f,   -0.57735f, -0.57735f, 0.57735f, 
    1.25f, 0.25f, -0.25f,  -0.57735f, -0.57735f, 0.57735f,  
  };

  
  // Coordenadas de los vértices de la pirámide nueva.
  GLuint new_vbo = 0;
  glGenBuffers(1, &new_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, new_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(new_vertex_positions), new_vertex_positions, GL_STATIC_DRAW);

  // Incluír las normales en la representación.
  // Vertex attributes
  // 0: vertex position (x, y, z)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  // Vertex attributes
  // 1: vertex normals (x, y, z)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Unbind vbo (it was conveniently registered by VertexAttribPointer)
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Unbind vao
  glBindVertexArray(0);

  // Uniforms
  // - Model matrix
  // - View matrix
  // - Projection matrix
  // - Normal matrix: normal vectors from local to world coordinates
  // - Camera position
  // - Light data
  // - Material data
  model_location = glGetUniformLocation(shader_program, "model");
  view_location = glGetUniformLocation(shader_program, "view");
  proj_location = glGetUniformLocation(shader_program, "projection");
  normal_to_world_location = glGetUniformLocation(shader_program, "normal_to_world");
  
  view_pos_location = glGetUniformLocation(shader_program, "view_pos");
  
  // Parte 1 (Other_Light): Especificación de las normales.
  other_light_pos_location = glGetUniformLocation(shader_program, "other_light.position");
  other_light_ambient_location = glGetUniformLocation(shader_program, "other_light.ambient");
  other_light_diffuse_location = glGetUniformLocation(shader_program, "other_light.diffuse");
  other_light_specular_location = glGetUniformLocation(shader_program, "other_light.specular");
  
  light_pos_location = glGetUniformLocation(shader_program, "light.position");
  light_ambient_location = glGetUniformLocation(shader_program, "light.ambient");
  light_diffuse_location = glGetUniformLocation(shader_program, "light.diffuse");
  light_specular_location = glGetUniformLocation(shader_program, "light.specular");
  
  material_ambient_location = glGetUniformLocation(shader_program, "material.ambient");
  material_diffuse_location = glGetUniformLocation(shader_program, "material.diffuse");
  material_specular_location = glGetUniformLocation(shader_program, "material.specular");
  material_shininess_location = glGetUniformLocation(shader_program, "material.shininess");

  // Render loop
  while(!glfwWindowShouldClose(window)) {

    processInput(window);

    render(glfwGetTime());

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}

void render(double currentTime) {
  float f = (float)currentTime * 0.3f;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, gl_width, gl_height);

  glUseProgram(shader_program);
  glBindVertexArray(vao);

  glm::mat4 model_matrix, view_matrix, proj_matrix;

  model_matrix = glm::mat4(1.f);
  view_matrix = glm::lookAt(                 camera_pos,  // pos
                            glm::vec3(0.0f, 0.0f, 0.0f),  // target
                            glm::vec3(0.0f, 1.0f, 0.0f)); // up

  // Moving cube
  model_matrix = glm::rotate(model_matrix, f, glm::vec3(0.5f, 1.0f, 0.0f));

  // Projection
  proj_matrix = glm::perspective(glm::radians(50.0f),
  				(float)gl_width / (float)gl_height,
  				 0.1f, 100.0f);

  // Normal matrix: normal vectors to world coordinates
  glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));

  // Valores uniformes
  glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
  glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
  glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));
  
  glUniformMatrix3fv(normal_to_world_location, 1, GL_FALSE, glm::value_ptr(normal_matrix));
  glUniform3fv(view_pos_location, 1, glm::value_ptr(camera_pos));  

  // Parte 1 (Other_Light): Renderizamos las normales.
  glUniform3fv(other_light_pos_location, 1, glm::value_ptr(other_light_pos));
  glUniform3fv(other_light_ambient_location, 1, glm::value_ptr(other_light_ambient));
  glUniform3fv(other_light_diffuse_location, 1, glm::value_ptr(other_light_diffuse)); 
  glUniform3fv(other_light_specular_location, 1, glm::value_ptr(other_light_specular));
  
  glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));
  glUniform3fv(light_ambient_location, 1, glm::value_ptr(light_ambient));
  glUniform3fv(light_diffuse_location, 1, glm::value_ptr(light_diffuse)); 
  glUniform3fv(light_specular_location, 1, glm::value_ptr(light_specular));

  glUniform3fv(material_ambient_location, 1, glm::value_ptr(material_ambient));  
  glUniform3fv(material_diffuse_location, 1, glm::value_ptr(material_diffuse));
  glUniform3fv(material_specular_location, 1, glm::value_ptr(material_specular));
  glUniform1f(material_shininess_location, material_shininess);

  glDrawArrays(GL_TRIANGLES, 0, 36);
  
  // Parte 2(Pirámide): Cargamos en escena la pirámide.
  glBindVertexArray(new_vao);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  
}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
}

// Callback function to track window size and update viewport
void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
  gl_width = width;
  gl_height = height;
  printf("New viewport: (width: %d, height: %d)\n", width, height);
}

