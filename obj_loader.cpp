#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <GL/glew.h>
#include <unordered_map>

#include "obj_loader.hpp"

// helpers
bool startsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delimiter)) tokens.push_back(token);
    return tokens;
}

// Loader method
bool ParseOBJ(std::string fileName, std::vector<Vertex>* vertices, std::vector<GLuint>* indices) {
    bool usemtl = true;
    if (fileName.length() >= 4) {
        fileName.resize(fileName.length() - 4);
    }
    else {
        return false;
    }

    std::ifstream mtlFile(fileName + ".mtl");
    if (!mtlFile) {
        printf("No materials, using white color for each vertex");
        usemtl = false;
    }

    std::unordered_map<std::string, glm::vec3> materials; // store simple color
    
    if (usemtl) {
        std::string matName;
        std::string line2;
        while (std::getline(mtlFile, line2)) {
            if (startsWith(line2, "newmtl ")) {
                std::stringstream ss(line2);
                std::string junk;
                ss >> junk >> matName;
            }
            else if (startsWith(line2, "Kd ")) {
                std::stringstream ss(line2);
                std::string junk;
                glm::vec3 color;
                ss >> junk >> color.x >> color.y >> color.z; // xyz -> rgb
                materials[matName] = color;
            }
        }
    }

    std::ifstream file(fileName + ".obj");
    if (!file) {
        printf("Cannot open file");
        return false;
    }

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_colors;
    std::string line;
    glm::vec3 currentColor;
    bool resize = true;
    while (std::getline(file, line)) {
        if (startsWith(line, "v ")) {
            std::stringstream ss(line);
            std::string junk;
            glm::vec3 pos;
            ss >> junk >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        }
        else if (startsWith(line, "f ")) {
            if (resize) {
                resize = false;
                temp_colors.resize(temp_positions.size());
            }
            std::vector<std::string> parts = split(line, ' ');
            std::vector<GLuint> face_indices;
            for (size_t i = 1; i < parts.size(); i++) {
                auto p = split(parts[i], '/');
                int idx = std::stoi(p[0]) - 1;
                face_indices.push_back(idx);
                temp_colors[idx] = currentColor;
            }
            for (size_t i = 1; i + 1 < face_indices.size(); i++) {
                indices->push_back(face_indices[0]);
                indices->push_back(face_indices[i]);
                indices->push_back(face_indices[i + 1]);
            }
        }
        else if (startsWith(line, "usemtl ")) {
            if (usemtl) {
                std::stringstream ss(line);
                std::string matName, junk;
                ss >> junk >> matName;
                if (materials.find(matName) != materials.end()) {
                    currentColor = materials[matName];
                }
                else {
                    currentColor = glm::vec3(1, 1, 1); // default to white color
                }
            }
        }
    }

    vertices->resize(temp_positions.size());
    for (size_t i = 0; i < vertices->size(); ++i) {
        (*vertices)[i].position = temp_positions[i];
        (*vertices)[i].color = temp_colors[i];
    }

    // Compute normals (sometimes not included into obj)
    for (size_t i = 0; i < indices->size(); i += 3) {
        GLuint i0 = (*indices)[i];
        GLuint i1 = (*indices)[i + 1];
        GLuint i2 = (*indices)[i + 2];

        glm::vec3 v0 = (*vertices)[i0].position;
        glm::vec3 v1 = (*vertices)[i1].position;
        glm::vec3 v2 = (*vertices)[i2].position;

        glm::vec3 normal = normalize(cross(v1 - v0, v2 - v0));

        (*vertices)[i0].normal += normal;
        (*vertices)[i1].normal += normal;
        (*vertices)[i2].normal += normal;
    }

    for (auto& v : (*vertices))
        v.normal = normalize(v.normal);

    return true;
}