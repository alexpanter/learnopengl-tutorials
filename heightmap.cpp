// STANDARD
#include <iostream>
#include <vector>
#include <string>

// GLEW
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp> // glm::to_string()


// CAMERA CLASS
class BasicRTSCamera
{
private:
protected:
    GLfloat _fov, _yaw, _pitch;
    const GLfloat MIN_FOV = 44.3f;
    const GLfloat MAX_FOV = 45.0f;

    // clipping planes
    GLfloat _near, _far;

    // camera's position vectors
    glm::vec3 _position, _front, _up;

    // transform matrices
    glm::mat4 _view, _projection;
    glm::mat4 _invView, _invProjection;

    // window properties
    glm::ivec2 _windowSize;
    GLfloat _aspectRatio;

    // mouse properties
    GLfloat _lastX, _lastY;

    void CalculateAspectRatio()
    {
        _aspectRatio = (GLfloat)_windowSize.x / (GLfloat)_windowSize.y;
    }

public:
    BasicRTSCamera(int width, int height, glm::vec3 pos,
                   glm::vec3 front, glm::vec3 up)
    {
        _near = 0.1f;
        _far = 200.0f;

        _windowSize.x = width;
        _windowSize.y = height;
        CalculateAspectRatio();

        _fov = 45.0f;
        _yaw = -90.f;
        _pitch = 0.0f;

        _lastX = (GLfloat)_windowSize.x / 2.0f;
        _lastY = (GLfloat)_windowSize.y / 2.0f;

        _position = pos;
        _front = glm::normalize(front);
        _up = glm::normalize(up);
    }
    ~BasicRTSCamera() {}

    void MoveNorth(GLfloat cameraSpeed)
    {
        _position.x += (_front.x + _up.x) * cameraSpeed;
        _position.y += (_front.y + _up.y) * cameraSpeed;
    }
    void MoveSouth(GLfloat cameraSpeed)
    {
        _position.x -= (_front.x + _up.x) * cameraSpeed;
        _position.y -= (_front.y + _up.y) * cameraSpeed;
    }
    void MoveWest(GLfloat cameraSpeed)
    {
        _position -= glm::normalize(glm::cross(_front, _up)) * cameraSpeed;
    }
    void MoveEast(GLfloat cameraSpeed)
    {
        _position += glm::normalize(glm::cross(_front, _up)) * cameraSpeed;
    }

    void MoveUp(GLfloat cameraSpeed) { _position.z += cameraSpeed; }
    void MoveDown(GLfloat cameraSpeed) { _position.z -= cameraSpeed; }

    void RotateCamera(GLfloat angle)
    {
        // camera ray intersection point with scene (where z = 0)
        GLfloat t = -_position.z / _front.z;
        glm::vec3 scene_intersection = _position + _front * t;

        // vector from camera point to intersection
        glm::vec3 dist = scene_intersection - _position;
        // printf("dist: (%g,%g,%g)\n", dist.x, dist.y, dist.z);

        // rotate front and up vectors
        _front = glm::normalize(glm::rotateZ(_front, angle));
        _up = glm::normalize(glm::rotateZ(_up, angle));

        // rotate a 3d vector around an axis
        _position = scene_intersection + glm::rotateZ(-dist, angle);
    }

    void RotateLeft(GLfloat cameraSpeed)
    {
        GLfloat angle = glm::pi<GLfloat>() * 0.1f * cameraSpeed;
        RotateCamera(angle);
    }

    void RotateRight(GLfloat cameraSpeed)
    {
        GLfloat angle = glm::pi<GLfloat>() * -0.1f * cameraSpeed;
        RotateCamera(angle);
    }

    // update the camera's view and projection matrices
    // according to any changes to the position vectors
    //
    // This function should be called at each iteration
    // of the game loop.
    void CalculateViewProjection()
    {
        _view = glm::lookAt(_position, _position + _front, _up);
        _projection = glm::perspective(_fov, _aspectRatio,
                                       _near, _far);
    }
    void CalculateInverseViewProjection()
    {
        _invView = glm::inverse(_view);
        _invProjection = glm::inverse(_projection);
    }

    void MouseScrollCallback(double yoffset)
    {
        if (_fov >= MIN_FOV && _fov <= MAX_FOV) { _fov -= yoffset * 0.1f; }
        if (_fov <= MIN_FOV)                    { _fov = MIN_FOV; }
        if (_fov >= MAX_FOV)                    { _fov = MAX_FOV; }
    }

    glm::vec3 GetViewDir() { return _front; }
    glm::vec3 GetPosition() { return _position; }
    const glm::vec3* GetPosition() const { return const_cast<glm::vec3*>(&_position); }
    const glm::mat4* GetViewMatrix() const { return const_cast<glm::mat4*>(&_view); }
    const glm::mat4* GetProjectionMatrix() const { return const_cast<glm::mat4*>(&_projection); }
    glm::mat4* GetInverseViewMatrix() { return &_invView; }
    glm::mat4* GetInverseProjectionMatrix() { return &_invProjection; }
};


// KEYBOARD
bool keyboard[1024] = { false };

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    static bool wireframe = false;
    scancode = scancode + 0; // disable annoying compiler warnings, cause I'm #pedantic!
    mode = mode + 0;
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
        case GLFW_KEY_SPACE:
            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_FILL : GL_LINE);
            wireframe = !wireframe;
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE); break;
        default:
            keyboard[key] = true; break;
        }
    }
    else if(action == GLFW_RELEASE)
    {
        switch(key)
        {
        default:
            keyboard[key] = false; break;
        }
    }
}

// CAMERA
BasicRTSCamera* camera;

bool MoveCamera(GLfloat deltaTime)
{
    GLfloat cameraSpeed = 15.0f * deltaTime;
    bool retval = false;

    if(keyboard[GLFW_KEY_W]) {
        camera->MoveNorth(cameraSpeed);
        retval = true;
    }
    else if(keyboard[GLFW_KEY_S]) {
        camera->MoveSouth(cameraSpeed);
        retval = true;
    }
    if(keyboard[GLFW_KEY_A]) {
        camera->MoveWest(cameraSpeed);
        retval = true;
    }
    else if(keyboard[GLFW_KEY_D]) {
        camera->MoveEast(cameraSpeed);
        retval = true;
    }
    if(keyboard[GLFW_KEY_Q]) {
        camera->RotateLeft(cameraSpeed / 3.0f);
        retval = true;
    }
    else if(keyboard[GLFW_KEY_E]) {
        camera->RotateRight(cameraSpeed / 3.0f);
        retval = true;
    }
    if(keyboard[GLFW_KEY_Z]) {
        if(keyboard[GLFW_KEY_LEFT_SHIFT]) {
            camera->MoveUp(cameraSpeed);
        } else {
            camera->MoveDown(cameraSpeed);
        }
        retval = true;
    }
    return retval;
}


// HEIGHT MAP DEFINITIONS
enum class TerrainType {
    Grass,
    Water
};

class HeightMapEntry
{
public:
    GLfloat Height;
    TerrainType Terrain;
};


// VERTEX SHADER
std::string vertexSource = R"(
#version 330 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
uniform mat4 projection;
uniform mat4 view;
void main() {
gl_Position = projection * view * vec4(vertexPosition, 1.0f);
}
)";

// FRAGMENT SHADER
std::string fragmentSource = R"(
#version 330 core
out vec4 color;
void main() {
color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
)";


// ENTRY POINT
int main()
{
    // INIT GLFW
    if(glfwInit() == GL_FALSE)
    {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return EXIT_FAILURE;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    // fix compilation on MacOSX
#if defined(__APPLE__) || defined(__MACH__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // CREATE WINDOW
    const char* title =  "LearnOpenGL: Height Map Rendering";
    GLFWwindow* window = glfwCreateWindow(800, 600, title, NULL, NULL);
    if(window == NULL)
    {
        std::cerr << "Failed to create GLFW windowed window" << std::endl;
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowTitle(window, title); // TODO: Needed?
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glfwSetKeyCallback(window, key_callback);

    // INIT GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Could not initialize GLEW!" << std::endl;
        return EXIT_FAILURE;
    }

    // APPLY SOME RENDERING SETTINGS
    // depth buffering
    glEnable(GL_DEPTH_TEST);
    // back-face culling
    glFrontFace(GL_CCW); // triangle vertices are read counter clock-wise
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // CREATE HEIGHT MAP
    const int N = 8;
    const int M = 8;
    const int SIZE = N * M;
    HeightMapEntry heightMap[SIZE];
    for(int i = 0; i < SIZE; i++)
    {
        heightMap[i].Height = 0.0f;
        heightMap[i].Terrain = TerrainType::Grass;
    }

    std::vector<glm::vec3> vertices;
    vertices.reserve(N * M);
    for (int y = 0; y < M; y++) {
        for (int x = 0; x < N; x++) {
            GLuint index = y*N + x;
            vertices.push_back(glm::vec3(x, y, heightMap[index].Height));
        }
    }
    GLuint T = (M-1) * (N-1) * 2;
    GLuint I = T * 3;

    std::vector<GLuint> indices;
    indices.reserve(I);
    for (int y = 0; y < M-1; y++) {
        for (int x = 0; x < N-1; x++) {
            // upper-left triangle
            GLuint topLeft = y*N + x;
            indices.push_back(topLeft);
            indices.push_back(topLeft + 1);
            indices.push_back(topLeft + M);
            // lower-right triangle
            indices.push_back(topLeft + 1);
            indices.push_back(topLeft + 1 + M);
            indices.push_back(topLeft + M);
        }
    }

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * N * M,
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * I,
                 indices.data(), GL_STATIC_DRAW);
    // attribute pointer - vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // SET-UP CAMERA
    glm::vec3 pos(14.2487f, -1.7683f, 15.3325f);
    glm::vec3 front(0.0f, 1.0f, -1.0f);
    glm::vec3 up(0.0f, 1.0f, 1.0f);
    camera = new BasicRTSCamera(800, 600, pos, front, up);
    camera->CalculateViewProjection();

    // HEIGHT MAP SHADER
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vSource = vertexSource.c_str();
    glShaderSource(vertex, 1, &vSource, NULL);
    glCompileShader(vertex);
    GLint vResult = GL_FALSE;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &vResult);
    if(!vResult)
    {
        fprintf(stderr, "---> ERROR: compiling vertex shader!\n");
        int logLength;
        glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> shader_err((logLength > 1) ? logLength : 1);
        glGetShaderInfoLog(vertex, logLength, NULL, &shader_err[0]);
        fprintf(stderr, "%s\n", &shader_err[0]);
    }

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fSource = fragmentSource.c_str();
    glShaderSource(fragment, 1, &fSource, NULL);
    glCompileShader(fragment);
    GLint fResult = GL_FALSE;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &fResult);
    if(!fResult)
    {
        fprintf(stderr, "---> ERROR: compiling fragment shader!\n");
        int logLength;
        glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> shader_err((logLength > 1) ? logLength : 1);
        glGetShaderInfoLog(vertex, logLength, NULL, &shader_err[0]);
        fprintf(stderr, "%s\n", &shader_err[0]);
    }

    GLuint shader = glCreateProgram();
    glAttachShader(shader, vertex);
    glAttachShader(shader, fragment);
    glLinkProgram(shader);
    GLint lResult = GL_FALSE;
    glGetProgramiv(shader, GL_LINK_STATUS, &lResult);
    if(!lResult)
    {
        fprintf(stderr, "---> ERROR: linking shader program:\n");
        int logLength;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> programError( (logLength > 1) ? logLength : 1 );
        glGetProgramInfoLog(shader, logLength, NULL, &programError[0]);
        std::cout << &programError[0] << std::endl;
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // TODO: Buffer initial view and projection matrices
    glUseProgram(shader);
    camera->CalculateViewProjection();
    GLint projLoc = glGetUniformLocation(shader, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(*camera->GetProjectionMatrix()));
    GLint viewLoc = glGetUniformLocation(shader, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(*camera->GetViewMatrix()));
    glUseProgram(0);

    // MEASURE TIME
    GLfloat lastTime = glfwGetTime();
    GLfloat nowTime;
    GLfloat deltaTime;

    // GAME LOOP
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        nowTime = glfwGetTime();
        deltaTime = nowTime - lastTime;
        lastTime = nowTime;
        if(MoveCamera(deltaTime))
        {
            glUseProgram(shader);
            camera->CalculateViewProjection();
            camera->CalculateViewProjection();
            GLint projLoc = glGetUniformLocation(shader, "projection");
            glUniformMatrix4fv(projLoc, 1, GL_FALSE,
                               glm::value_ptr(*camera->GetProjectionMatrix()));
            GLint viewLoc = glGetUniformLocation(shader, "view");
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE,
                               glm::value_ptr(*camera->GetViewMatrix()));
            glUseProgram(0);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, I, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(window);
    }

    // CLEANUP
    glUseProgram(0);
    glDeleteProgram(shader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
