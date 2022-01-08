#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>

#include "glad/include/glad.h"
#include "GL/include/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "obj_loader.h"

using namespace glm;

struct Mesh
{
    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> texCords;
    std::vector<unsigned int> indices;
    unsigned int specular;
    unsigned int diffuse;
    float shininess;
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    vec3 scale;
    vec3 translation;

    Mesh() = default;
    Mesh(
        std::string const& filename,
        unsigned int const diffuse,
        unsigned int const specular,
        float        const shininess,
        vec3         const scale,
        vec3         const translation
    )
        : diffuse{diffuse}
        , specular{specular}
        , shininess{shininess}
        , scale{scale}
        , translation{translation}
    {
        objl::Loader loader;
        loader.LoadFile(filename);
        for(auto i : loader.LoadedMeshes[0].Indices)
        {
            indices.push_back(i);
        }
        for(auto v : loader.LoadedMeshes[0].Vertices)
        {
            vertices.push_back(vec3(v.Position.X, v.Position.Y, v.Position.Z));
            normals .push_back(vec3(v.Normal.X, v.Normal.Y, v.Normal.Z));
            texCords.push_back(vec2(v.TextureCoordinate.X, v.TextureCoordinate.Y));
        }
        setupMesh();
    }

    void setupMesh()
    {
        auto const v_size = vertices.size() * sizeof(vec3);
        auto const n_size = normals .size() * sizeof(vec3);
        auto const t_size = texCords.size() * sizeof(vec2);
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, v_size +  n_size + texCords.size() * sizeof(vec2), NULL, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0              , v_size, &vertices[0]);
        glBufferSubData(GL_ARRAY_BUFFER, v_size         , n_size, &normals[0]);
        glBufferSubData(GL_ARRAY_BUFFER, v_size + n_size, t_size, &texCords[0]);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(v_size));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)(v_size + n_size));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void drawMesh()
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

#endif
