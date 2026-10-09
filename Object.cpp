/**
 * @file Object.cpp
 * @author chwh1
 */

#include "Object.h"

#include <string>
#include <fstream>
#include <sstream>

Object::Object(std::string path)
{
    parseFile(path);
}

void Object::parseFile(std::string path)
{
    std::ifstream obj_file(path);
    std::string file_line;

    while (std::getline(obj_file, file_line)) {
        if (file_line.empty()) continue;
        std::istringstream iss(file_line);

        std::string identifier;
        iss >> identifier;

        if (identifier == "v") {
            float x_normalized, y_normalized, z_normalized;
            iss >> x_normalized;
            iss >> y_normalized;
            iss >> z_normalized;

            vec3 new_vertex{x_normalized, y_normalized, z_normalized};
            vertices.push_back(new_vertex);
        } else if (identifier == "f") {
            std::string face[3];
            int vertex_indices[3] = {};

            iss >> face[0] >> face[1] >> face[2];

            for (int i = 0; i < 3; ++i) {
                std::size_t found_slash_index = face[i].find("/");
                if (found_slash_index != std::string::npos) {
                    vertex_indices[i] = std::stoi(face[i].substr(0, found_slash_index));
                }
            }

            vec3 first_vertex = vertices.at(vertex_indices[0] - 1);
            vec3 second_vertex = vertices.at(vertex_indices[1] - 1);
            vec3 third_vertex = vertices.at(vertex_indices[2] - 1);

            faces.push_back({first_vertex, second_vertex, third_vertex});
        }
    }
}

vec3 Object::GetVertex(int faceIndex, int vertexIndex) const
{
    return faces.at(faceIndex).at(vertexIndex);
}

unsigned int Object::GetNumberOfFaces() const
{
    return faces.size();
}
