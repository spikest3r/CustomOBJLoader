#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

bool ParseOBJ(std::string path, std::vector<Vertex>* vertices, std::vector<GLuint>* indices);