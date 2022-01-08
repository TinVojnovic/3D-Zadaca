#ifndef LIGHT_H
#define LIGHT_H

#include "glad/include/glad.h"
#include "GL/include/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "mesh.h"

using namespace glm;

struct Light
{
    glm::vec3 color;
    glm::vec3 pos;
    Mesh obj;
    Light (
        glm::vec3 const color,
        glm::vec3 const pos
    )
        : color(color)
        , pos(pos)
    {
        this->obj = Mesh("sphere.obj", 0, 0, 0, vec3(0.2, 0.2, 0.2), pos);
    }
};

#endif
