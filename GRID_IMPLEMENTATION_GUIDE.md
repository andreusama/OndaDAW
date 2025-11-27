# 3D Grid Implementation Guide

## Table of Contents
1. [Theory: 3D Graphics Pipeline](#theory-3d-graphics-pipeline)
2. [Coordinate Systems](#coordinate-systems)
3. [Matrix Transformations](#matrix-transformations)
4. [Grid Geometry Generation](#grid-geometry-generation)
5. [Shader Programming](#shader-programming)
6. [OpenGL Implementation](#opengl-implementation)
7. [Complete Code Example](#complete-code-example)

---

## Theory: 3D Graphics Pipeline

### How 3D Rendering Works

The 3D graphics pipeline transforms 3D coordinates into 2D screen pixels through several stages:

```
Vertex Data → Vertex Shader → Primitive Assembly → Rasterization → Fragment Shader → Screen
```

**Key Stages:**

1. **Vertex Shader**: Transforms 3D positions using matrices
2. **Primitive Assembly**: Connects vertices into lines/triangles
3. **Rasterization**: Converts primitives into fragments (potential pixels)
4. **Fragment Shader**: Determines final pixel color
5. **Depth Testing**: Determines which fragments are visible

---

## Coordinate Systems

A 3D point goes through multiple coordinate system transformations:

### 1. Local Space (Object Space)
The coordinates relative to the object's origin.

### 2. World Space
After applying the **Model Matrix** (M):
```
P_world = M × P_local
```

### 3. View Space (Camera Space)
After applying the **View Matrix** (V):
```
P_view = V × P_world
```

### 4. Clip Space
After applying the **Projection Matrix** (P):
```
P_clip = P × P_view
```

### 5. Normalized Device Coordinates (NDC)
After perspective division:
```
P_ndc = P_clip / P_clip.w
```

### Combined Transformation
The complete transformation is:
```
P_clip = P × V × M × P_local
```
This is called the **MVP Matrix**.

---

## Matrix Transformations

### Model Matrix (M)
Transforms from object space to world space. Combines:
- **Translation**: Move the object
- **Rotation**: Rotate the object
- **Scale**: Resize the object

**Identity Matrix** (no transformation):
```
M = [1  0  0  0]
    [0  1  0  0]
    [0  0  1  0]
    [0  0  0  1]
```

**Translation Matrix**:
```
T = [1  0  0  tx]
    [0  1  0  ty]
    [0  0  1  tz]
    [0  0  0  1 ]
```

### View Matrix (V)
Transforms from world space to camera space.

**Formula**:
```
V = [Rx  Ry  Rz  0]   [1  0  0  -Ex]
    [Ux  Uy  Uz  0] × [0  1  0  -Ey]
    [Dx  Dy  Dz  0]   [0  0  1  -Ez]
    [0   0   0   1]   [0  0  0   1 ]
```

Where:
- **E** = Camera position (eye)
- **R** = Right vector
- **U** = Up vector
- **D** = Direction vector (forward)

**LookAt Function**:
Given camera position (eye), target point, and up vector:
```
D = normalize(eye - target)      // Forward
R = normalize(cross(up, D))      // Right
U = cross(D, R)                  // Up
```

### Projection Matrix (P)

#### Perspective Projection
Creates depth perception (objects farther away appear smaller).

**Formula**:
```
f = 1 / tan(fov/2)
aspect = width / height

P = [f/aspect  0   0              0          ]
    [0         f   0              0          ]
    [0         0   (far+near)/(near-far)  (2×far×near)/(near-far)]
    [0         0   -1             0          ]
```

Where:
- **fov**: Field of view (in radians)
- **aspect**: Aspect ratio
- **near**: Near clipping plane
- **far**: Far clipping plane

#### Orthographic Projection
No perspective (parallel lines stay parallel).

**Formula**:
```
P = [2/(right-left)    0                 0                -(right+left)/(right-left)]
    [0                 2/(top-bottom)    0                -(top+bottom)/(top-bottom)]
    [0                 0                 -2/(far-near)    -(far+near)/(far-near)   ]
    [0                 0                 0                 1                        ]
```

---

## Grid Geometry Generation

### Grid on XZ Plane

A grid consists of lines running parallel to X-axis and Z-axis.

**Parameters**:
- Grid size: `N × N` units
- Spacing: `s` units between lines
- Number of lines: `(N/s) + 1` in each direction

### Vertex Generation Algorithm

For a 10×10 grid centered at origin with spacing 1:

```cpp
std::vector<float> vertices;
int gridSize = 10;     // Total size
float spacing = 1.0f;  // Distance between lines
int halfSize = gridSize / 2;

// Lines parallel to X-axis (varying Z)
for (int z = -halfSize; z <= halfSize; z += spacing) {
    // Start point
    vertices.push_back(-halfSize);  // x
    vertices.push_back(0.0f);       // y
    vertices.push_back(z);          // z

    // End point
    vertices.push_back(halfSize);   // x
    vertices.push_back(0.0f);       // y
    vertices.push_back(z);          // z
}

// Lines parallel to Z-axis (varying X)
for (int x = -halfSize; x <= halfSize; x += spacing) {
    // Start point
    vertices.push_back(x);          // x
    vertices.push_back(0.0f);       // y
    vertices.push_back(-halfSize);  // z

    // End point
    vertices.push_back(x);          // x
    vertices.push_back(0.0f);       // y
    vertices.push_back(halfSize);   // z
}
```

**Result**: Array of vertex positions (x, y, z) defining line segments.

### Adding Axis Lines

To highlight the main axes with different colors, add separate geometry:

```cpp
// X-axis (Red): from (-10,0,0) to (10,0,0)
// Y-axis (Green): from (0,-10,0) to (0,10,0)
// Z-axis (Blue): from (0,0,-10) to (0,0,10)
```

---

## Shader Programming

Shaders are programs that run on the GPU.

### Vertex Shader Theory

**Purpose**: Transform vertex positions from local space to clip space.

**Input**: Vertex attributes (position, color, etc.)
**Output**: `gl_Position` (clip space coordinates)

**GLSL Vertex Shader**:
```glsl
#version 330 core

layout(location = 0) in vec3 aPos;  // Input vertex position

uniform mat4 MVP;  // Model-View-Projection matrix

void main()
{
    // Transform vertex to clip space
    gl_Position = MVP * vec4(aPos, 1.0);
}
```

**Explanation**:
- `in vec3 aPos`: Receives vertex position from VBO
- `uniform mat4 MVP`: 4×4 transformation matrix (same for all vertices)
- `vec4(aPos, 1.0)`: Convert 3D position to 4D homogeneous coordinates
- Matrix multiplication: `MVP × position` transforms the vertex

### Fragment Shader Theory

**Purpose**: Determine the color of each pixel.

**Input**: Interpolated data from vertex shader
**Output**: Final pixel color

**GLSL Fragment Shader**:
```glsl
#version 330 core

out vec4 FragColor;       // Output color
uniform vec4 gridColor;   // Grid line color

void main()
{
    FragColor = gridColor;  // Simply output the uniform color
}
```

**Explanation**:
- `out vec4 FragColor`: RGBA color output
- `uniform vec4 gridColor`: Color set from CPU code
- All fragments get the same solid color

### Shader Compilation Process

```cpp
// 1. Create shader object
GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

// 2. Attach source code
const char* vertexSource = "...shader code...";
glShaderSource(vertexShader, 1, &vertexSource, NULL);

// 3. Compile
glCompileShader(vertexShader);

// 4. Check for errors
GLint success;
glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    // Handle error
}

// 5. Repeat for fragment shader
// ...

// 6. Link shaders into program
GLuint shaderProgram = glCreateProgram();
glAttachShader(shaderProgram, vertexShader);
glAttachShader(shaderProgram, fragmentShader);
glLinkProgram(shaderProgram);

// 7. Cleanup
glDeleteShader(vertexShader);
glDeleteShader(fragmentShader);
```

---

## OpenGL Implementation

### Vertex Buffer Object (VBO)

**Theory**: VBO stores vertex data in GPU memory for fast access.

```cpp
GLuint VBO;
glGenBuffers(1, &VBO);                    // Generate buffer ID
glBindBuffer(GL_ARRAY_BUFFER, VBO);       // Bind as array buffer
glBufferData(GL_ARRAY_BUFFER,             // Upload data to GPU
             vertices.size() * sizeof(float),
             vertices.data(),
             GL_STATIC_DRAW);              // Data won't change
```

**Buffer Usage Hints**:
- `GL_STATIC_DRAW`: Set once, used many times
- `GL_DYNAMIC_DRAW`: Modified occasionally
- `GL_STREAM_DRAW`: Modified every frame

### Vertex Array Object (VAO)

**Theory**: VAO stores the configuration of vertex attributes.

```cpp
GLuint VAO;
glGenVertexArrays(1, &VAO);
glBindVertexArray(VAO);

// Tell OpenGL how to interpret the vertex data
glVertexAttribPointer(
    0,                   // Attribute location (matches shader)
    3,                   // Number of components (x, y, z)
    GL_FLOAT,            // Data type
    GL_FALSE,            // Don't normalize
    3 * sizeof(float),   // Stride (bytes between vertices)
    (void*)0             // Offset (start at beginning)
);
glEnableVertexAttribArray(0);  // Enable attribute 0
```

**Stride Calculation**:
```
Stride = (number of components) × sizeof(component type)
For vec3: 3 × 4 bytes = 12 bytes
```

### Drawing the Grid

```cpp
glUseProgram(shaderProgram);   // Activate shader
glBindVertexArray(VAO);        // Bind vertex data

// Set uniforms
GLint mvpLoc = glGetUniformLocation(shaderProgram, "MVP");
glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvpMatrix[0][0]);

GLint colorLoc = glGetUniformLocation(shaderProgram, "gridColor");
glUniform4f(colorLoc, 0.5f, 0.5f, 0.5f, 1.0f);  // Gray

// Draw all lines
int vertexCount = vertices.size() / 3;  // 3 components per vertex
glDrawArrays(GL_LINES, 0, vertexCount);
```

**GL_LINES**: Each pair of vertices forms a line segment.

---

## Complete Code Example

### Grid.h
```cpp
#pragma once
#include <vector>
#include <glew.h>
#include <glm/glm.hpp>

class Grid {
public:
    Grid(int size = 10, float spacing = 1.0f);
    ~Grid();

    void Render(const glm::mat4& mvp);

private:
    void GenerateGeometry();
    void SetupBuffers();
    void CreateShaders();

    GLuint VAO, VBO;
    GLuint shaderProgram;
    std::vector<float> vertices;
    int vertexCount;

    int gridSize_;
    float spacing_;
};
```

### Grid.cpp
```cpp
#include "Grid.h"
#include <glm/gtc/type_ptr.hpp>

// Shader sources
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 gridColor;

void main() {
    FragColor = gridColor;
}
)";

Grid::Grid(int size, float spacing)
    : gridSize_(size), spacing_(spacing), VAO(0), VBO(0), shaderProgram(0)
{
    GenerateGeometry();
    SetupBuffers();
    CreateShaders();
}

Grid::~Grid() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

void Grid::GenerateGeometry() {
    vertices.clear();
    int halfSize = gridSize_ / 2;

    // Lines parallel to X-axis
    for (int z = -halfSize; z <= halfSize; z += spacing_) {
        vertices.push_back(-halfSize); vertices.push_back(0.0f); vertices.push_back(z);
        vertices.push_back(halfSize);  vertices.push_back(0.0f); vertices.push_back(z);
    }

    // Lines parallel to Z-axis
    for (int x = -halfSize; x <= halfSize; x += spacing_) {
        vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(-halfSize);
        vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(halfSize);
    }

    vertexCount = vertices.size() / 3;
}

void Grid::SetupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float),
                 vertices.data(),
                 GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Grid::CreateShaders() {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check compilation
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        // Log error: infoLog
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        // Log error: infoLog
    }

    // Link program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        // Log error: infoLog
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Grid::Render(const glm::mat4& mvp) {
    glUseProgram(shaderProgram);

    // Set MVP matrix
    GLint mvpLoc = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    // Set color (gray)
    GLint colorLoc = glGetUniformLocation(shaderProgram, "gridColor");
    glUniform4f(colorLoc, 0.5f, 0.5f, 0.5f, 1.0f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, vertexCount);
    glBindVertexArray(0);
}
```

### Camera.h (Helper for View Matrix)
```cpp
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

    Camera()
        : position(0.0f, 5.0f, 10.0f),
          target(0.0f, 0.0f, 0.0f),
          up(0.0f, 1.0f, 0.0f)
    {}

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(position, target, up);
    }

    glm::mat4 GetProjectionMatrix(float aspect) const {
        return glm::perspective(
            glm::radians(45.0f),  // FOV
            aspect,               // Aspect ratio
            0.1f,                 // Near plane
            100.0f                // Far plane
        );
    }
};
```

### Usage in ViewportPanel.cpp
```cpp
#include "Grid.h"
#include "Camera.h"

class ViewportPanel : public Panel {
private:
    std::unique_ptr<Grid> grid_;
    Camera camera_;

public:
    ViewportPanel(int w, int h)
        : Panel("Viewport"),
          viewport_(std::make_unique<Viewport>(0,0,0, w, h)),
          grid_(std::make_unique<Grid>(20, 1.0f))  // 20x20 grid
    {
        CreateViewportFramebuffer();
    }

    void RenderScene() override {
        // Calculate matrices
        glm::mat4 model = glm::mat4(1.0f);  // Identity (no transformation)
        glm::mat4 view = camera_.GetViewMatrix();

        float aspect = (float)viewport_->width / (float)viewport_->height;
        glm::mat4 projection = camera_.GetProjectionMatrix(aspect);

        glm::mat4 mvp = projection * view * model;

        // Render grid
        grid_->Render(mvp);
    }
};
```

---

## Mathematical Details

### Homogeneous Coordinates

Why use 4D vectors for 3D positions?

**Reason**: Allows translation to be represented as matrix multiplication.

A 3D point (x, y, z) becomes (x, y, z, 1):
```
[x']   [M00 M01 M02 M03]   [x]
[y'] = [M10 M11 M12 M13] × [y]
[z']   [M20 M21 M22 M23]   [z]
[w']   [M30 M31 M32 M33]   [1]
```

After transformation: (x'/w', y'/w', z'/w')

### Perspective Division

Converts clip space to NDC:
```
x_ndc = x_clip / w_clip
y_ndc = y_clip / w_clip
z_ndc = z_clip / w_clip
```

This creates perspective effect:
- Points farther from camera have larger w
- Division by larger w makes them appear smaller

### Depth Buffer Algorithm

```
for each fragment:
    if (fragment.depth < depthBuffer[x][y]):
        depthBuffer[x][y] = fragment.depth
        colorBuffer[x][y] = fragment.color
    else:
        discard fragment
```

**Depth Range**: [0, 1] where:
- 0 = near plane
- 1 = far plane

---

## Performance Considerations

### Vertex Count Calculation

For grid size N with spacing s:
```
Lines in each direction = (N/s) + 1
Vertices per line = 2
Total vertices = 2 × 2 × ((N/s) + 1)
```

Example: 20×20 grid with spacing 1:
```
Lines = 21 + 21 = 42
Vertices = 42 × 2 = 84 vertices
```

### GPU Memory Usage

```
Memory = vertices × components × sizeof(float)
84 vertices × 3 components × 4 bytes = 1,008 bytes (~1 KB)
```

Very lightweight!

### Draw Call Optimization

- **Single draw call**: All grid lines in one `glDrawArrays()` call
- **Static geometry**: Use `GL_STATIC_DRAW` since grid doesn't change
- **Avoid state changes**: Minimize shader switches

---

## Debugging Tips

### Verify Matrix Math
```cpp
glm::mat4 mvp = projection * view * model;  // Correct order!
// NOT: model * view * projection
```

### Check Depth Testing
```cpp
glEnable(GL_DEPTH_TEST);         // Enable depth testing
glDepthFunc(GL_LESS);            // Closer fragments win
glClear(GL_DEPTH_BUFFER_BIT);    // Clear depth buffer each frame
```

### Shader Debugging
Add color output to verify shader is running:
```glsl
FragColor = vec4(1.0, 0.0, 1.0, 1.0);  // Magenta for debugging
```

### Verify Viewport
```cpp
glViewport(0, 0, width, height);  // Must match framebuffer size
```

---

## Extensions

### Infinite Grid Effect
Use shader-based approach:
- Generate grid in fragment shader
- Fade based on distance from camera
- No vertex data needed!

### Dynamic Grid Density
Adjust spacing based on camera distance:
```cpp
float distance = glm::length(camera.position - camera.target);
float spacing = pow(10.0f, floor(log10(distance)));
```

### Axis Highlighting
Render center lines (X and Z axis) with brighter colors:
```cpp
// Add separate geometry for axes
// Use different color uniform
```

---

## Summary

**Key Concepts**:
1. Grid = collection of line segments on XZ plane
2. MVP matrix transforms 3D → 2D
3. Shaders run on GPU for each vertex/fragment
4. VAO/VBO store geometry on GPU
5. Depth testing handles 3D occlusion

**Rendering Pipeline**:
```
Vertex Data (CPU) → Upload to GPU (VBO) → Vertex Shader (Transform) →
Rasterization → Fragment Shader (Color) → Depth Test → Screen
```
