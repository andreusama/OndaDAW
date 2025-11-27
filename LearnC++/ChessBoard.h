#pragma once
#include <glew.h>
#include <glm.hpp>
#include <vector>

class ChessBoard {
public:
    ChessBoard();
    ~ChessBoard();

    void DrawBoard(glm::mat4& mvp);

private:
    void InitializeShader();
    void GenerateBoardGeometry();

    std::vector<float> vertices_;
    std::vector<unsigned int> indices_;
    int indexCount_;

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint shaderProgram_;

    const int BOARD_SIZE = 8;
    const float SQUARE_SIZE = 1.0f;
};
