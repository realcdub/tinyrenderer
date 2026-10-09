/**
 * @file Object.h
 * @author chwh1
 *
 *
 */

#ifndef TINYRENDERER_OBJECT_H
#define TINYRENDERER_OBJECT_H

#include "geometry.h"
#include <vector>
#include <string>

class Object
{
private:
    std::vector<vec3> vertices;
    std::vector<std::vector<vec3>> faces;

    void parseFile(std::string path);
public:
    Object(std::string path);

    vec3 GetVertex(int face, int index) const;
    unsigned int GetNumberOfFaces() const;
};

#endif //TINYRENDERER_OBJECT_H
