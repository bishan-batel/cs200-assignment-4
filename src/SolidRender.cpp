#include "SolidRender.h"
#include <array>
#include <stdexcept>

/**
 * @brief Fragment Shader Source
 */
constexpr const char *FRAGMENT_SOURCE = //
    "#version 130\n"
    "uniform vec4 color;"
    "out vec4 frag_color;"
    "void main(void) {"
    " frag_color = color;"
    "}";

/**
 * @brief Vertex Shader Source
 */
constexpr const char *VERTEX_SOURCE = //
    "#version 130\n"
    "in vec4 position;"
    "uniform mat4 transform;"
    "void main(void) {"
    " gl_Position = transform * position;"
    "}";

namespace cs200 {

  static GLuint compile_shader(const char *const source, GLenum shader_type) {
    const GLuint id = glCreateShader(shader_type);

    if (id == 0) throw std::runtime_error{"Failure to create shader (possible incorrect shader_type)"};

    glShaderSource(id, 1, &source, nullptr);

    glCompileShader(id);

    int32_t success{0};
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (not success) {
      std::array<char, 512> info_log{'\0'};
      glGetShaderInfoLog(id, info_log.size(), nullptr, info_log.data());
      throw std::runtime_error{std::string{"Failed to compile Shader: "} + info_log.data()};
    }

    return id;
  }

  static GLuint link_program(const GLuint vertex_shader, const GLuint fragment_shader) {
    const GLuint id = glCreateProgram();

    if (id == 0) throw std::runtime_error{"Failed to create program"};

    glAttachShader(id, vertex_shader);
    glAttachShader(id, fragment_shader);

    glLinkProgram(id);

    int32_t success{0};
    glGetProgramiv(id, GL_LINK_STATUS, &success);

    // clean up shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    if (not success) {
      std::array<char, 512> info_log{'\0'};
      glGetProgramInfoLog(id, info_log.size(), nullptr, info_log.data());
      throw std::runtime_error{std::string{"Failed to compile link program: "} + info_log.data()};
    }

    return id;
  }

  SolidRender::SolidRender() :
      ucolor{0}, //
      utransform{0}, //
      program{0}, //
      vao_edges{0}, //
      vao_faces{0}, //
      vertex_buffer{0}, //
      edge_buffer{0}, //
      face_buffer{0}, //
      mesh_edge_count{0}, //
      mesh_face_count{0} {

    program = link_program(
        compile_shader(VERTEX_SOURCE, GL_VERTEX_SHADER), //
        compile_shader(FRAGMENT_SOURCE, GL_FRAGMENT_SHADER) //
    );

    ucolor = glGetUniformLocation(program, "color");
    utransform = glGetUniformLocation(program, "transform");
  }

  SolidRender::~SolidRender() { glDeleteProgram(program); }

  void SolidRender::clearFrame(const glm::vec4 &c) {
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void SolidRender::setTransform(const glm::mat4 &M) { // NOLINT
    glUseProgram(program);
    glUniformMatrix4fv(utransform, 1, false, &M[0][0]);
  }

  void SolidRender::loadMesh(const Mesh &m) {
    // Generate vertex buffer & bind
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<std::intptr_t>(m.vertexCount() * sizeof(glm::vec4)),
        m.vertexArray(),
        GL_STATIC_DRAW);

    // Setup Edge VAO, EBO
    glGenVertexArrays(1, &vao_edges);
    glBindVertexArray(vao_edges);

    glUseProgram(program);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    glGenBuffers(1, &edge_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edge_buffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<std::intptr_t>(m.edgeCount() * sizeof(Mesh::Edge)),
        m.edgeArray(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(glm::vec4), nullptr);
    glEnableVertexAttribArray(0);

    // FACES VAO
    glGenVertexArrays(1, &vao_faces);
    glBindVertexArray(vao_faces);

    glUseProgram(program);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    glGenBuffers(1, &face_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<std::intptr_t>(m.faceCount() * sizeof(Mesh::Face)),
        m.faceArray(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(glm::vec4), nullptr);
    glEnableVertexAttribArray(0);


    // Cleanup 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
  }

  void SolidRender::unloadMesh() {
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &edge_buffer);
    glDeleteBuffers(1, &face_buffer);
    glDeleteVertexArrays(1, &vao_edges);
    glDeleteVertexArrays(1, &vao_faces);
  }

  void SolidRender::displayEdges(const glm::vec4 &c) {
    glUseProgram(program);

    glUniform4f(ucolor, c.r, c.g, c.b, c.a);

    glBindVertexArray(vao_edges);
    glDrawElements(GL_LINES, mesh_edge_count * 2, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glUseProgram(0);
  }

  void SolidRender::displayFaces(const glm::vec4 &c) {
    glUseProgram(program);
    glUniform4f(ucolor, c.r, c.g, c.b, c.a);

    glBindVertexArray(vao_faces);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glUseProgram(0);
  }
} // namespace cs200
