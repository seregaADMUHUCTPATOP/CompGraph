#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include "GpuProgram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Model {
public:
    Model(std::string const& path) {
        loadModel(path);
    }

    void Draw(const GpuProgram& shader, glm::mat4 OX1_transform, glm::mat4 OX2_transform, glm::mat4 OX3_transform) {
        // ИСПРАВЛЕНО: Координаты выставлены строго по вашим точным метрикам из Blender
        // X = 1.1024, Y (высота OpenGL = Z Blender) = 1.0019, Z (глубина OpenGL = -Y Blender) = 0.84674
        glm::vec3 turret_pivot = glm::vec3(1.1024f, 1.0019f, 0.84674f);

        glm::mat4 T1 = glm::translate(glm::mat4(1.0f), turret_pivot);
        glm::mat4 T1_inv = glm::translate(glm::mat4(1.0f), -turret_pivot);

        // Матрица башни: изолированное вращение вокруг собственной оси на месте
        glm::mat4 turret_matrix = T1 * OX1_transform * T1_inv;

        // Вторая деталь (дуло) наследует вращение башни и смещается по вертикали (KL)
        glm::mat4 gun_matrix = turret_matrix * OX2_transform;

        // Третья деталь (пушки/поршни) наследует всё и двигается вперед-назад (OP)
        glm::mat4 extension_matrix = gun_matrix * OX3_transform;

        for (unsigned int i = 0; i < meshes.size(); i++) {
            glm::mat4 currentModelMatrix = glm::mat4(1.0f);

            switch (i) {
            case 0:
                currentModelMatrix = glm::mat4(1.0f); // Корпус танка
                break;
            case 1:
                currentModelMatrix = turret_matrix;   // Башня (Вращение NM)
                break;
            case 2:
                currentModelMatrix = gun_matrix;      // Дуло (Ход KL)
                break;
            case 3:
                currentModelMatrix = extension_matrix; // Выдвижная часть (Ход OP)
                break;
            default:
                currentModelMatrix = glm::mat4(1.0f);
                break;
            }

            shader.SetUniform("model", glm::value_ptr(currentModelMatrix));
            meshes[i].Draw(shader);
        }
    }

private:
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(std::string const& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            if (mesh->HasNormals())
                vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            if (mesh->mTextureCoords[0])
                vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        return Mesh(vertices, indices);
    }
};