#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>

// Helper to load shader string from file
std::string loadShader(const char* path) {
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void checkShaderErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" 
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
}

void checkLinkingErrors(unsigned int program) {
    int success;
    char infoLog[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM_LINKING_ERROR\n" 
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
}


int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow* window = glfwCreateWindow(400, 400, "Mac OpenGL Path Tracer", NULL, NULL);
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  // 1. Create Ping-Pong Textures & FBOs
  unsigned int fbos[2], texs[2];
  glGenFramebuffers(2, fbos);
  glGenTextures(2, texs);
  for (int i = 0; i < 2; i++) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
    glBindTexture(GL_TEXTURE_2D, texs[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 400, 400, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texs[i], 0);
  }

  // 2. Compile Path Trace Shader (Fragment)
  std::string fragSrc = loadShader("src/shader/shader.frag");
  const char* vSrc = "#version 410 core\nlayout(location=0) in vec2 p; out vec2 TexCoords; void main(){TexCoords=p; gl_Position=vec4(p,0,1);}";
  const char* fSrc = fragSrc.c_str();

  auto compile = [](unsigned int type, const char* s) {
      unsigned int id = glCreateShader(type);
      glShaderSource(id, 1, &s, NULL);
      glCompileShader(id);
      checkShaderErrors(id, "FRAGMENT");
      return id;
  };
  unsigned int program = glCreateProgram();
  glAttachShader(program, compile(GL_VERTEX_SHADER, vSrc));
  glAttachShader(program, compile(GL_FRAGMENT_SHADER, fSrc));
  glLinkProgram(program);
  checkLinkingErrors(program);

  // 3. Simple Screen Quad
  float quad[] = {-1,1, -1,-1, 1,-1, -1,1, 1,-1, 1,1};
  unsigned int vao, vbo;
  glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
  glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);

  int frame = 0;
  while (!glfwWindowShouldClose(window)) {
    int readIdx = frame % 2;
    int writeIdx = (frame + 1) % 2;

    // Step A: Draw to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[writeIdx]);
    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "u_time"), glfwGetTime());
    glUniform1i(glGetUniformLocation(program, "u_frame"), frame);
    int resLoc = glGetUniformLocation(program, "u_resolution");

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height); 
    glUniform2f(resLoc, (float)width, (float)height);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texs[readIdx]);
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Step B: Blit FBO to Screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, texs[writeIdx]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwSwapBuffers(window);
    glfwPollEvents();
    frame++;
  }
  glfwTerminate();
}