#pragma once
#include <glew.h>
#include <glm.hpp>
#include <vector>
#include <memory>
#include "Square.h"

class ChessBoard {
public:
    ChessBoard();
    ~ChessBoard();

    void DrawBoard(glm::mat4& mvp);

private:
    void InitializeShader();
    void GenerateBoardGeometry();

    std::vector<std::unique_ptr<Square>> squares_;

    // White squares data
    std::vector<float> whiteVertices_;
    std::vector<unsigned int> whiteIndices_;
    int whiteIndexCount_;
    GLuint whiteVAO_;
    GLuint whiteVBO_;
    GLuint whiteEBO_;

    // Black squares data
    std::vector<float> blackVertices_;
    std::vector<unsigned int> blackIndices_;
    int blackIndexCount_;
    GLuint blackVAO_;
    GLuint blackVBO_;
    GLuint blackEBO_;

    GLuint shaderProgram_;

    std::unique_ptr<Texture> whiteSquareTexture_;
    std::unique_ptr<Texture> blackSquareTexture_;

    const int BOARD_SIZE = 8;
    const float SQUARE_SIZE = 1.0f;
};
