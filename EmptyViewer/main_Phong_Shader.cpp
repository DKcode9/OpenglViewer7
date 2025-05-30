#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <fstream>
#include <sstream>

using namespace std;
using namespace glm;

// === 셰이더 파일을 로드하고 컴파일하는 함수 ===
GLuint LoadShaders(const string& vertex_file_path, const string& fragment_file_path) {
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    string VertexShaderCode;
    ifstream VertexShaderStream(vertex_file_path, ios::in);
    if (VertexShaderStream.is_open()) {
        stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }
    else {
        printf("Impossible to open\n");
        getchar();
        return 0;
    }

    //read fragment shader code
    string FragmentShaderCode;
    ifstream FragmentShaderStream(fragment_file_path, ios::in);
    if (FragmentShaderStream.is_open()) {
        stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    cout << "Compiling shader: " << vertex_file_path << endl;
    const char* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    //check vertex shader compile status
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        cerr << &VertexShaderErrorMessage[0] << endl;
    }

    // Compile Fragment Shader
    cout << "Compiling shader: " << fragment_file_path << endl;
    const char* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        vector<char> error(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &error[0]);
        cerr << &error[0] << endl;
    }

    // Link the program
    cout << "Linking program" << endl;
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        cerr << &ProgramErrorMessage[0] << endl;
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

// === VBO와 VAO 설정 ===
void UpdateMesh(GLuint& VAO, const vector<GLuint>& GLBuffers,
    const vector<vec3>& Positions, const vector<vec3>& Normals,
    const vector<vec3> Colors, const vector<unsigned int>& Indices) {
    glBindVertexArray(VAO);//must bound before bindding buffers

    glBindBuffer(GL_ARRAY_BUFFER, GLBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, Positions.size() * sizeof(Positions[0]), &Positions[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, GLBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, Normals.size() * sizeof(Normals[0]), &Normals[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, GLBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, Colors.size() * sizeof(Colors[0]), &Colors[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLBuffers[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(Indices[0]), &Indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
}

// === 구의 vertex 생성 ===
void generateSphere(float radius, int width, int height,
    std::vector<vec3>& Positions,
    std::vector<vec3>& Normals,
    std::vector<vec3>& Colors,
    std::vector<unsigned int>& Indices) {

    Positions.clear();
    Normals.clear();
    Colors.clear();
    Indices.clear();

    // 내부 영역 (극점 제외)
    for (int j = 1; j < height - 1; ++j) {
        float theta = float(j) / (height - 1) * pi<float>();
        for (int i = 0; i < width; ++i) {
            float phi = float(i) / (width - 1) * 2.0f * pi<float>();
            float x = sin(theta) * cos(phi);
            float y = cos(theta);
            float z = -sin(theta) * sin(phi);
            vec3 p = radius * vec3(x, y, z);
            Positions.push_back(p);
            Normals.push_back(normalize(p));
            Colors.push_back(vec3(1.0f)); // 흰색
        }
    }

    // 북극점
    Positions.push_back(vec3(0, radius, 0));
    Normals.push_back(vec3(0, 1, 0));
    Colors.push_back(vec3(1.0f)); // 흰색

    // 남극점
    Positions.push_back(vec3(0, -radius, 0));
    Normals.push_back(vec3(0, -1, 0));
    Colors.push_back(vec3(1.0f)); // 흰색

    int topIndex = Positions.size() - 2;
    int bottomIndex = Positions.size() - 1;

    // 내부 삼각형
    for (int j = 0; j < height - 3; ++j) {
        for (int i = 0; i < width - 1; ++i) {
            int idx0 = j * width + i;
            int idx1 = (j + 1) * width + (i + 1);
            int idx2 = j * width + (i + 1);
            int idx3 = (j + 1) * width + i;

            Indices.push_back(idx0);
            Indices.push_back(idx1);
            Indices.push_back(idx2);

            Indices.push_back(idx0);
            Indices.push_back(idx3);
            Indices.push_back(idx1);
        }
    }

    // 북극 삼각형
    for (int i = 0; i < width - 1; ++i) {
        Indices.push_back(topIndex);
        Indices.push_back(i);
        Indices.push_back(i + 1);
    }

    // 남극 삼각형
    for (int i = 0; i < width - 1; ++i) {
        int base = (height - 3) * width;
        Indices.push_back(bottomIndex);
        Indices.push_back(base + i + 1);
        Indices.push_back(base + i);
    }
}




//// Global variables
int Width = 512;
int Height = 512;

void resize_callback(GLFWwindow* window, int nw, int hw) {
    Width = nw;
    Height = hw;
    glViewport(0, 0, nw, hw);//viewport transformation
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

int main(int argc, char* argv[]) {
    // GLFW 초기화
    GLFWwindow* window;
    if (!glfwInit())
        return -1;

    // OpenGL 컨텍스트 버전 설정
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 창 생성
    window = glfwCreateWindow(512, 512, "OpenGL Sphere", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    // GLEW 초기화
    glewExperimental = true;
    if (glewInit() != GLEW_OK) return -1;
    
    // 깊이 테스트 활성화
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // 셰이더 프로그램
    GLuint shaderProgram = LoadShaders("Phong.vert", "Phong.frag");

    // 구 정점 생성 (가로 32, 세로 16)
    vector<vec3> Positions;
    vector<vec3> Normals;
    vector<vec3> Colors;
    vector<unsigned int> Indices;
    generateSphere(1.0f, 32, 16, Positions, Normals, Colors,Indices);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    const int numBuffers = 4;
    vector<GLuint> GLBuffers(numBuffers);
    glGenBuffers(numBuffers, &GLBuffers[0]);
    UpdateMesh(VAO, GLBuffers, Positions, Normals, Colors, Indices);

    // MVP 설정
    mat4 model = scale(translate(mat4(1.0f), vec3(0, 0, -7)), vec3(2.0f));
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));    
    mat4 projection = frustum(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, 1000.0f);
    //glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 1000.0f);

    // Uniform 위치
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

    // Phong 머티리얼/라이팅 uniform 위치
    GLint kaLoc = glGetUniformLocation(shaderProgram, "ka");
    GLint kdLoc = glGetUniformLocation(shaderProgram, "kd");
    GLint ksLoc = glGetUniformLocation(shaderProgram, "ks");
    GLint pLoc = glGetUniformLocation(shaderProgram, "p");
    GLint IaLoc = glGetUniformLocation(shaderProgram, "Ia");
    GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

    // 랜더링 루프
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        //버퍼 clear
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 셰이더 사용
        glUseProgram(shaderProgram);

        // MVP 행렬 uniform
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        // Phong 머티리얼/라이팅 uniform
        glUniform3f(kaLoc, 0.0f, 1.0f, 0.0f);
        glUniform3f(kdLoc, 0.0f, 0.5f, 0.0f);
        glUniform3f(ksLoc, 0.5f, 0.5f, 0.5f);
        glUniform1f(pLoc, 32.0f);
        glUniform1f(IaLoc, 0.2f);
        glUniform3f(lightPosLoc, -4.0f, 4.0f, -3.0f);
        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
        glUniform3f(viewPosLoc, 0.0f, 0.0f, 0.0f); // 카메라 위치

		// 그리기
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
		// 버퍼 스왑 및 이벤트 처리
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    if (GLBuffers[0] != 0)
        glDeleteBuffers(numBuffers, &GLBuffers[0]);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

