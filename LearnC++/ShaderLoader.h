#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glew.h>

class ShaderLoader {
public:
    // Loads vertex and fragment shaders, compiles them, and returns a shader program
    static GLuint LoadShaders(const char* vertexPath, const char* fragmentPath) {
        // 1. Read shader source code from files
        std::string vertexCode;
        std::string fragmentCode;

        try {
            // Read vertex shader
            std::ifstream vertexFile(vertexPath);
            std::stringstream vertexStream;
            vertexStream << vertexFile.rdbuf();
            vertexFile.close();
            vertexCode = vertexStream.str();

            // Read fragment shader
            std::ifstream fragmentFile(fragmentPath);
            std::stringstream fragmentStream;
            fragmentStream << fragmentFile.rdbuf();
            fragmentFile.close();
            fragmentCode = fragmentStream.str();
        }
        catch (std::exception& e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
            return 0;
        }

        const char* vertexShaderSource = vertexCode.c_str();
        const char* fragmentShaderSource = fragmentCode.c_str();

        // 2. Compile vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        CheckCompileErrors(vertexShader, "VERTEX");

        // 3. Compile fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        CheckCompileErrors(fragmentShader, "FRAGMENT");

        // 4. Link shaders into program
        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        CheckCompileErrors(shaderProgram, "PROGRAM");

        // 5. Delete shaders (no longer needed after linking)
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return shaderProgram;
    }

private:
    // Helper function to check for shader compilation/linking errors
    static void CheckCompileErrors(GLuint shader, std::string type) {
        GLint success;
        GLchar infoLog[1024];

        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                    << infoLog << "\n" << std::endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                    << infoLog << "\n" << std::endl;
            }
        }
    }
};
