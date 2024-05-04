#include <iostream>
#define GLEW_STATIC   //static버전 사용하려면
#include <GL/glew.h>  //GLFW보다 먼저 include
#include <GLFW/glfw3.h>
#include "toys.h"
#include <glm/glm.hpp>  //glm은 header파일만 있음
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "j3a.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;

Program program;


void render(GLFWwindow* window);
void init(GLFWwindow* window);
void setupFBO(GLFWwindow* window);
//float gauss(float x, float sigma2);
void setMatrices();
void pass1(GLFWwindow* window);
void pass2(GLFWwindow* window);
void pass3(GLFWwindow* window);
void pass4(GLFWwindow* window);
void pass5(GLFWwindow* window);




GLuint vertexBuffer = 0;
GLuint normalBuffer = 0;
GLuint texCoordBuffer = 0;
GLuint indexBuffer = 0;
GLuint vertexArray = 0;
GLuint diffTex = 0;
GLuint bumpTex = 0;

GLuint fsQuad;

//FBO
GLuint FBO1 = 0;
GLuint FBO2 = 0;


GLuint Tex1 = 0;
GLuint Tex2 = 0;

mat4 projMat;
mat4 modelMat;
mat4 viewMat;



float theta = 0;
float phi = 0;
double lastX = 0, lastY = 0;
float cameraDistance = 7;
float fovy = 0.8;
vec3 lightPosition = vec3(3, 10, 4);

// input parameter
int number = 0;
int Pass = 1;
int gaussian = 0;

int width, height;


void cursorCB(GLFWwindow* w, double xpos, double ypos) {
    if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_1)) {
        theta -= (xpos - lastX) / 300;
        phi -= (ypos - lastY) / 300;
        lastX = xpos;
        lastY = ypos;
    }
}
void mouseBtnCB(GLFWwindow* w, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        glfwGetCursorPos(w, &lastX, &lastY);
    }
}
void scrollCB(GLFWwindow* window, double xoffset, double yoffset) {
    //cameraDistance = cameraDistance * pow(1.01, yoffset);
    fovy *= pow(1.1, yoffset);
}

void processInput(GLFWwindow* window)
{

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        number = 1;
    }
    //camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        number = 2;

    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        number = 3;

    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        if (gaussian == 1) {
            gaussian = 0;
        }
        else {
            gaussian = 1;

        }
    }

}



int main()
{

    if (!glfwInit()) return 0;
    //multisampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
    
    //glfwSetCursorPosCallback(window, cursorCB);
    //glfwSetMouseButtonCallback(window, mouseBtnCB);
    //glfwSetScrollCallback(window, scrollCB);


    glfwMakeContextCurrent(window);

    glewInit(); //context 만들고 current까지 한 뒤에 Init()하기
    init(window);



    while (!glfwWindowShouldClose(window)) {
        processInput(window);


        render(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}

GLuint loadTexture(const std::string& filename) {
    GLuint tex = 0;
    int w, h, n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* buf = stbi_load(filename.c_str(), &w, &h, &n, 4);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    stbi_image_free(buf);
    return tex;
}


void init(GLFWwindow* window) {
    loadJ3A("Trex.j3a");
    program.loadShaders("shader.vert", "shader.frag");

    int w, h, n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* buf = stbi_load(diffuseMap[0].c_str(), &w, &h, &n, 4 );

    diffTex = loadTexture(diffuseMap[0]);
    bumpTex = loadTexture(bumpMap[0]);



    setupFBO(window);

    // Array for full-screen quad
    GLfloat verts[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
    };
    GLfloat tc[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };



    //data를 얻어오기 위한 buffer
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(vec3), vertices[0], GL_STATIC_DRAW);
     
    //normal vector buffer
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(vec3), normals[0], GL_STATIC_DRAW);

    //texture coordinate buffer
    glGenBuffers(1, &texCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(vec2), texCoords[0], GL_STATIC_DRAW);
    
    //primitive assembly용 buffer (index저장용)
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, nTriangles[0] * sizeof(u32vec3), triangles[0], GL_STATIC_DRAW);

    // Set up the vertex array object

    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
    //vertexBuffer 연결
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
    ///normalBuffer 연결
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, 0, 0, 0);
    ///texCoordBuffer 연결
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, 0, 0, 0);


    // Set up the buffers

    unsigned int handle[2];
    glGenBuffers(2, handle);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);

    // Set up the vertex array object

    glGenVertexArrays(1, &fsQuad);
    glBindVertexArray(fsQuad);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte*)NULL + (0)));
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte*)NULL + (0)));
    glEnableVertexAttribArray(2);  // Texture coordinates

    glBindVertexArray(0);


    /*
    char uniName[20];
    float weights[5], sum, sigma2 = 4.0f;
    // Compute and sum the weights 
    weights[0] = gauss(0, sigma2); // The 1-D Gaussian function 
    sum = weights[0];
    for (int i = 1; i < 5; i++) {
        weights[i] = gauss(i, sigma2);
        sum += 2 * weights[i];
    }
    // Normalize the weights and set the uniform 
    for (int i = 0; i < 5; i++) {
        snprintf(uniName, 20, "Weight[%d]", i);
        //program.setUniform(uniName, weights[i] / sum);
        GLuint loc = glGetUniformLocation(program.programID, uniName);
        glUniform1f(loc, weights[i] / sum);
    }
    */
}

const float PI = 3.141592;


void render(GLFWwindow* window) {
    if (number == 1) {
        pass1(window);
        if (gaussian == 1) {
            pass2(window);
            pass3(window);
        }
    }
    else if (number == 2) {
        pass1(window);
        if (gaussian == 1) {
            pass2(window);
            pass3(window);
        }
        pass5(window);
    }
    else if (number == 3) {
        pass1(window);
        if (gaussian == 1) {
            pass2(window);
            pass3(window);
        }
        pass4(window);
        pass5(window);
    }
    else if (number == 0) {
        pass1(window);
    }


}


void pass1(GLFWwindow* window) { // 3D scene rendering
    GLuint loc = glGetUniformLocation(program.programID, "Pass");
    glUniform1i(loc, 1);
    
    //program.setUniform("Pass", 1);

    if (number == 1 || number == 0) {
        if (gaussian == 1) {
            glBindFramebuffer(GL_FRAMEBUFFER, FBO1);
        }
        else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO1);
    }


    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    vec3 cameraPosition = vec3(0, 0, cameraDistance);
    cameraPosition = vec3(rotate(theta, vec3(0, 1, 0)) * rotate(phi, vec3(1, 0, 0)) * vec4(cameraPosition, 1));

    projMat = perspective(fovy, width / float(height), 0.1f, 100.f);
    viewMat = lookAt(cameraPosition, vec3(0), vec3(0, 1, 0));
    modelMat = mat4(1);

    //Depth Test
    glEnable(GL_DEPTH_TEST);
    

    glClearColor(0, 0, 0.2, 0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glUseProgram(program.programID);

    loc = glGetUniformLocation(program.programID, "modelMat");
    glUniformMatrix4fv(loc, 1, false, value_ptr(modelMat));

    loc = glGetUniformLocation(program.programID, "viewMat");
    glUniformMatrix4fv(loc, 1, false, value_ptr(viewMat));

    loc = glGetUniformLocation(program.programID, "projMat");
    glUniformMatrix4fv(loc, 1, false, value_ptr(projMat));

    loc = glGetUniformLocation(program.programID, "lightPosition");
    glUniform3fv(loc, 1, value_ptr(lightPosition));

    loc = glGetUniformLocation(program.programID, "cameraPosition");
    glUniform3fv(loc, 1, value_ptr(cameraPosition));

    loc = glGetUniformLocation(program.programID, "diffColor");
    glUniform3fv(loc, 1, value_ptr(vec3(0)));

    loc = glGetUniformLocation(program.programID, "specColor");
    glUniform3fv(loc, 1, value_ptr(specularColor[0]));

    loc = glGetUniformLocation(program.programID, "shininess");
    glUniform1f(loc, shininess[0]);


    //number, gaussian
    loc = glGetUniformLocation(program.programID, "number");
    glUniform1i(loc, number);


    loc = glGetUniformLocation(program.programID, "gaussian");
    glUniform1i(loc, gaussian);




    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffTex);
    loc = glGetUniformLocation(program.programID, "diffTex");
    glUniform1i(loc, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bumpTex);
    loc = glGetUniformLocation(program.programID, "bumpTex");
    glUniform1i(loc, 1);


    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawElements(GL_TRIANGLES, nTriangles[0]*3, GL_UNSIGNED_INT, 0);  //삼각형의 개수 * 삼각형 내 vertex수 (element몇개를 그리느냐)

}

void pass2(GLFWwindow* window) { // gaussian 1
    GLuint loc = glGetUniformLocation(program.programID, "Pass");
    glUniform1i(loc, 2);


    glBindFramebuffer(GL_FRAMEBUFFER, FBO2);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Tex1);
    loc = glGetUniformLocation(program.programID, "Texture0");
    glUniform1i(loc, 0);

    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    modelMat = mat4(1.0f);
    viewMat = mat4(1.0f);
    projMat = mat4(1.0f);
    setMatrices();

    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);



}
void pass3(GLFWwindow* window) { // gaussian 2
    GLuint loc = glGetUniformLocation(program.programID, "Pass");
    glUniform1i(loc, 3);

    if (number == 1) {       
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO1);
    }


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Tex2);
    loc = glGetUniformLocation(program.programID, "Texture0");
    glUniform1i(loc, 0);

    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    modelMat = mat4(1.0f);
    viewMat = mat4(1.0f);
    projMat = mat4(1.0f);
    setMatrices();

    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);



}
void pass4(GLFWwindow* window) { // edge detection
    GLuint loc = glGetUniformLocation(program.programID, "Pass");
    glUniform1i(loc, 4);


    glBindFramebuffer(GL_FRAMEBUFFER, FBO2);



    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Tex1);
    loc = glGetUniformLocation(program.programID, "Texture0");
    glUniform1i(loc, 0);

    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    modelMat = mat4(1.0f);
    viewMat = mat4(1.0f);
    projMat = mat4(1.0f);
    setMatrices();

    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
   

}

void pass5(GLFWwindow* window) { // toon shading
    GLuint loc = glGetUniformLocation(program.programID, "Pass");
    glUniform1i(loc, 5);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);




    if (number == 2) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Tex1);

    
    }
    else if (number == 3) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Tex2);
    }
    loc = glGetUniformLocation(program.programID, "Texture0");
    glUniform1i(loc, 0);



    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1, 1, 1, 0);


    modelMat = mat4(1.0f);
    viewMat = mat4(1.0f);
    projMat = mat4(1.0f);
    setMatrices();

    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);



}



void setMatrices()
{
    GLuint loc = glGetUniformLocation(program.programID, "modelMat");
    glUniformMatrix4fv(loc, 1, false, value_ptr(modelMat));

    loc = glGetUniformLocation(program.programID, "viewMat");
    glUniformMatrix4fv(loc, 1, false, value_ptr(viewMat));

    loc = glGetUniformLocation(program.programID, "projMat");
    glUniformMatrix4fv(loc, 1, false, value_ptr(projMat));


}

/*

float gauss(float x, float sigma2)
{
    double coeff = 1.0 / (two_pi<double>() * sigma2);
    double expon = -(x * x) / (2.0 * sigma2);
    return (float)(coeff * exp(expon));
}
*/


void setupFBO(GLFWwindow* window) {
    // Generate and bind the framebuffer
    //FBO1
    glGenFramebuffers(1, &FBO1);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO1);

    glfwGetFramebufferSize(window, &width, &height);

    // Create the texture object
    //Tex1
    glGenTextures(1, &Tex1);
    //glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, Tex1);
#ifdef __APPLE__
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Tex1, 0);

    // Create the depth buffer
    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    // Bind the depth buffer to the FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER, depthBuf);

    // Set the targets for the fragment output variables
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);


    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);




    // Generate and bind the framebuffer
    //FBO2
    glGenFramebuffers(1, &FBO2);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO2);

    // Create the texture object
    //Tex2
    glGenTextures(1, &Tex2);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, Tex2);
#ifdef __APPLE__
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Tex2, 0);

    // No depth buffer needed for this FBO

    // Set the targets for the fragment output variables
    glDrawBuffers(1, drawBuffers);

    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}