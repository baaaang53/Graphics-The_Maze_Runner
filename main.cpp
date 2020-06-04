
//      Mouse: Arcball manipulation
//      Keyboard: 'r' - reset arcball

#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>

#include <shader.h>
#include <cube.h>
#include <plane.h>
#include <arcball.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;

// the number of partitions
const int ANGLE_NUM = 40;


// Function Prototypes
GLFWwindow *glAllInit();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow *window, double x, double y);
unsigned int loadTexture(const char *path);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void render();

// Global variables
GLFWwindow *mainWindow = NULL;
Shader *shader = NULL;
Shader *shaderSingleColor = NULL;
unsigned int SCR_WIDTH = 600;
unsigned int SCR_HEIGHT = 600;
Cube *cube;
Plane *plane;
glm::mat4 projection, view, model;

// for sphere
float theta =  2 * M_PI / ANGLE_NUM;
GLfloat vertices[ANGLE_NUM+1][(ANGLE_NUM/4+1)*2 * 12];
GLfloat textcood[9600];
GLfloat normals[ANGLE_NUM + 1][(ANGLE_NUM/4+1)*2 * 12];
GLfloat colors[0];

float cx[ANGLE_NUM];
float cy[ANGLE_NUM /2];
float cz[ANGLE_NUM];
float cc[ANGLE_NUM/2];

// character vao, vbo
unsigned int smileVBO[4];
unsigned int smileVAO;

//item vao, vbo
unsigned int itemVBO[4];
unsigned int itemVAO;

unsigned int vSize = sizeof(vertices);
unsigned int tSize = sizeof(textcood);
unsigned int nSize = sizeof(normals);
unsigned int cSize = sizeof(colors);;

float lastTime = 0.0f;
// for camera
glm::vec3 cameraOrigPos(0.0f, 23.0f, 18.0f);
glm::vec3 cameraPos;
glm::vec3 camTarget(0.0f, 0.0f, 0.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);

// for arcball
float arcballSpeed = 0.1f;
static Arcball camArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);

// for texture
unsigned int cubeTexture, floorTexture, characTexture, itemTexture, finishTexture;

// for outline
float outlineScale = 1.1f;


int main()
{
	mainWindow = glAllInit();

	// shader loading and compile (by calling the constructor)
	shader = new Shader("2.stencil_testing.vs", "2.stencil_testing.fs");
	shaderSingleColor = new Shader("2.stencil_testing.vs", "2.stencil_single_color.fs");

    int i;
    
    //90도까지만 저장을 해 둬도 된다. 어차피 남반구 = -북반구
    for (i = 0; i < (ANGLE_NUM /4) + 1 ; i++) {
        theta = 2 * M_PI * i / ANGLE_NUM;
        cy[i] = sin(theta);
        cc[i] = cos(theta);
    }
    
    for (i = 0; i < ANGLE_NUM+1; i++)
    {
        theta = 2* M_PI * i / ANGLE_NUM;
        cx[i] = cos(theta);
        cz[i] = sin(theta);
    }
    
        //create buffers
    glGenVertexArrays(1, &smileVAO);
    glGenBuffers(4, smileVBO);
    
    glGenVertexArrays(1, &itemVAO);
    glGenBuffers(4, itemVBO);
    
     //smile character
    glBindVertexArray(smileVAO);
    
    // reserve space for position attributes
    glBindBuffer(GL_ARRAY_BUFFER, smileVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // reserve space for normal attributes
    glBindBuffer(GL_ARRAY_BUFFER, smileVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // reserve space for color attributes
    glBindBuffer(GL_ARRAY_BUFFER, smileVBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // reserve space for texture
    glBindBuffer(GL_ARRAY_BUFFER, smileVBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textcood), 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    
    // item
    glBindVertexArray(itemVAO);
    
    // reserve space for position attributes
    glBindBuffer(GL_ARRAY_BUFFER, itemVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // reserve space for normal attributes
    glBindBuffer(GL_ARRAY_BUFFER, itemVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // reserve space for color attributes
    glBindBuffer(GL_ARRAY_BUFFER, itemVBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // reserve space for texture
    glBindBuffer(GL_ARRAY_BUFFER, itemVBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textcood), 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //구 좌표, normal 좌표, texture mapping 위한 좌표 설정
    for (int i=0; i < ANGLE_NUM + 1; i++) {
        for (int j=0; j < (ANGLE_NUM/4) + 1; j++)
        { // 북반구
            vertices[i][j*24 +0] = cx[i] * cc[j];
            vertices[i][j*24 +1] = cy[j];
            vertices[i][j*24 +2] = cz[i] * cc[j];
            
            vertices[i][j*24 +3] = cx[(i+1)%ANGLE_NUM] * cc[j];
            vertices[i][j*24 +4] = cy[j];
            vertices[i][j*24 +5] = cz[(i+1)%ANGLE_NUM] * cc[j];
            
            vertices[i][j*24 +6] = cx[i] * cc[(j+1)%(ANGLE_NUM/2)];
            vertices[i][j*24 +7] = cy[(j+1)%(ANGLE_NUM/2)];
            vertices[i][j*24 +8] = cz[i] * cc[(j+1)%(ANGLE_NUM/2)];
            
            vertices[i][j*24 +9] = cx[(i+1)%ANGLE_NUM] * cc[(j+1)%(ANGLE_NUM/2)];
            vertices[i][j*24 +10] = cy[(j+1)%(ANGLE_NUM/2)];
            vertices[i][j*24 +11] = cz[(i+1)%ANGLE_NUM] * cc[(j+1)%(ANGLE_NUM/2)];
            
            normals[i][j*24 +0] = cx[i] * cc[j];
            normals[i][j*24 +1] = cy[j];
            normals[i][j*24 +2] = cz[i] * cc[j];
            
            normals[i][j*24 +3] = cx[(i+1)%ANGLE_NUM] * cc[j];
            normals[i][j*24 +4] = cy[j];
            normals[i][j*24 +5] = cz[(i+1)%ANGLE_NUM] * cc[j];
            
            normals[i][j*24 +6] = cx[i] * cc[(j+1)%(ANGLE_NUM/2)];
            normals[i][j*24 +7] = cy[(j+1)%(ANGLE_NUM/2)];
            normals[i][j*24 +8] = cz[i] * cc[(j+1)%(ANGLE_NUM/2)];
            
            normals[i][j*24 +9] = cx[(i+1)%ANGLE_NUM] * cc[(j+1)%(ANGLE_NUM/2)];
            normals[i][j*24 +10] = cy[(j+1)%(ANGLE_NUM/2)];
            normals[i][j*24 +11] = cz[(i+1)%ANGLE_NUM] * cc[(j+1)%(ANGLE_NUM/2)];
            
            textcood[176 * i + 16 * j + 0] = 0.025 * i;
            textcood[176 * i + 16 * j + 1] = 0.5 + 0.05 * j;
            
            textcood[176 * i + 16 * j + 2] = 0.025 * i + 0.025;
            textcood[176 * i + 16 * j + 3] = 0.5 + 0.05 * j;
            
            textcood[176 * i + 16 * j + 4] = 0.025 * i;
            textcood[176 * i + 16 * j + 5] = 0.5 + 0.05 * j + 0.05;
            
            textcood[176 * i + 16 * j + 6] = 0.025 * i + 0.025;
            textcood[176 * i + 16 * j + 7] = 0.5 + 0.05 * j + 0.05;
        }
        for (int j=0; j < (ANGLE_NUM/4) +1; j++)
        { // 남반구
            vertices[i][j*24 +12] = cx[i] * cc[j];
            vertices[i][j*24 +13] = -cy[j];
            vertices[i][j*24 +14] = cz[i] * cc[j];
            
            vertices[i][j*24 +15] = cx[(i+1)%ANGLE_NUM] * cc[j];
            vertices[i][j*24 +16] = -cy[j];
            vertices[i][j*24 +17] = cz[(i+1)%ANGLE_NUM] * cc[j];
            
            vertices[i][j*24 +18] = cx[i] * cc[(j+1)%(ANGLE_NUM/2)];
            vertices[i][j*24 +19] = -cy[(j+1)%(ANGLE_NUM/2)];
            vertices[i][j*24 +20] = cz[i] * cc[(j+1)%(ANGLE_NUM/2)];
            
            vertices[i][j*24 +21] = cx[(i+1)%ANGLE_NUM] * cc[(j+1)%(ANGLE_NUM/2)];
            vertices[i][j*24 +22] = -cy[(j+1)%(ANGLE_NUM/2)];
            vertices[i][j*24 +23] = cz[(i+1)%ANGLE_NUM] * cc[(j+1)%(ANGLE_NUM/2)];
            
            
            normals[i][j*24 +12] = cx[i] * cc[j];
            normals[i][j*24 +13] = -cy[j];
            normals[i][j*24 +14] = cz[i] * cc[j];
            
            normals[i][j*24 +15] = cx[(i+1)%ANGLE_NUM] * cc[j];
            normals[i][j*24 +16] = -cy[j];
            normals[i][j*24 +17] = cz[(i+1)%ANGLE_NUM] * cc[j];
            
            normals[i][j*24 +18] = cx[i] * cc[(j+1)%(ANGLE_NUM/2)];
            normals[i][j*24 +19] = -cy[(j+1)%(ANGLE_NUM/2)];
            normals[i][j*24 +20] = cz[i] * cc[(j+1)%(ANGLE_NUM/2)];
            
            normals[i][j*24 +21] = cx[(i+1)%ANGLE_NUM] * cc[(j+1)%(ANGLE_NUM/2)];
            normals[i][j*24 +22] = -cy[(j+1)%(ANGLE_NUM/2)];
            normals[i][j*24 +23] = cz[(i+1)%ANGLE_NUM] * cc[(j+1)%(ANGLE_NUM/2)];
            
            textcood[176 * i + 16 * j + 8] = 0.025 * i;
            textcood[176 * i + 16 * j + 9] = 0.5 - (0.05 * j);
        
            textcood[176 * i + 16 * j + 10] = 0.025 * i + 0.025;
            textcood[176 * i + 16 * j + 11] = 0.5-(0.05 * j);
            
            textcood[176 * i + 16 * j + 12] = 0.025 * i;
            textcood[176 * i + 16 * j + 13] = 0.5- (0.05 * j) - 0.05;
            
            textcood[176 * i + 16 * j + 14] = 0.025 * i + 0.025;
            textcood[176 * i + 16 * j + 15] = 0.5-(0.05 * j) - 0.05;
                
            
        }
    }
    
    //smile
    glBindVertexArray(smileVAO);
           
    glBindBuffer(GL_ARRAY_BUFFER, smileVBO[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, smileVBO[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(normals), normals);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, smileVBO[2]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(colors), colors);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, smileVBO[3]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(textcood), textcood);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    
    //item
    glBindVertexArray(itemVAO);
           
    glBindBuffer(GL_ARRAY_BUFFER, itemVBO[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, itemVBO[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(normals), normals);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, itemVBO[2]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(colors), colors);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, itemVBO[3]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(textcood), textcood);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    
    
	// projection matrix
	projection = glm::perspective(glm::radians(45.0f),
		(float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	shader->use();
	shader->setMat4("projection", projection);
	shaderSingleColor->use();
	shaderSingleColor->setMat4("projection", projection);

	// set defaul camera position
	cameraPos = cameraOrigPos;

	// load textures
	// -------------
	cubeTexture = loadTexture("stonewall2.png");
	floorTexture = loadTexture("metal.png");
    characTexture = loadTexture("smile.png");
    itemTexture = loadTexture("shiny.png");
    finishTexture = loadTexture("finish.png");

	shader->use();
	shader->setInt("texture1", 0);
    
	// create a cube and a plane
	cube = new Cube();
	plane = new Plane();

	while (!glfwWindowShouldClose(mainWindow)) {
		render();
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

// render
void render() {
    int i;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    float currentTime = glfwGetTime();
	// send view to shader
	view = glm::lookAt(cameraPos, camTarget, camUp);
	view = view * camArcBall.createRotationMatrix();
	shader->use();
	shader->setMat4("view", view);
	shaderSingleColor->use();
	shaderSingleColor->setMat4("view", view);
	shader->use();

	// drawing a floor
	glBindTexture(GL_TEXTURE_2D, floorTexture);
	model = glm::mat4(1.0);
    model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(19.0f, 19.0f, 0.0f));
	shader->setMat4("model", model);
	plane->draw(shader);
    
    // drawing a finish line
    glBindTexture(GL_TEXTURE_2D, finishTexture);
    model = glm::mat4(1.0);
    model = glm::translate(model, glm::vec3(4.0f, -0.4f, 9.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 0.2f));
    shader->setMat4("model", model);
    cube->draw(shader);

    //drawing a sphere ( smile )
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-4.0f, 0.0f, -9.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    shader->setMat4("model", model);
    
    glBindTexture(GL_TEXTURE_2D, characTexture);
    glBindVertexArray(smileVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3600);
    
    //drawing a sphere ( item )
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-8.0f, 0.2f, -8.0f));
    model = glm::rotate(model,(float)(30.0f * M_PI / 90.0f) * (currentTime - lastTime), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
    shader->setMat4("model", model);
    glBindTexture(GL_TEXTURE_2D, itemTexture);
    glBindVertexArray(itemVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3600);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-6.0f, 0.2f, 1.0f));
    model = glm::rotate(model,(float)(30.0f * M_PI / 90.0f) * (currentTime - lastTime), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
    shader->setMat4("model", model);
    glBindTexture(GL_TEXTURE_2D, itemTexture);
    glBindVertexArray(itemVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3600);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.2f, -8.0f));
    model = glm::rotate(model,(float)(30.0f * M_PI / 90.0f) * (currentTime - lastTime), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
    shader->setMat4("model", model);
    glBindTexture(GL_TEXTURE_2D, itemTexture);
    glBindVertexArray(itemVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3600);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(8.0f, 0.2f, -4.0f));
    model = glm::rotate(model,(float)(30.0f * M_PI / 90.0f) * (currentTime - lastTime), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
    shader->setMat4("model", model);
    glBindTexture(GL_TEXTURE_2D, itemTexture);
    glBindVertexArray(itemVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3600);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(6.0f, 0.2f, 8.0f));
    model = glm::rotate(model,(float)(30.0f * M_PI / 90.0f) * (currentTime - lastTime), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
    shader->setMat4("model", model);
    glBindTexture(GL_TEXTURE_2D, itemTexture);
    glBindVertexArray(itemVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3600);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.0f, 0.2f, 6.0f));
    model = glm::rotate(model,(float)(30.0f * M_PI / 90.0f) * (currentTime - lastTime), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
    shader->setMat4("model", model);
    glBindTexture(GL_TEXTURE_2D, itemTexture);
    glBindVertexArray(itemVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3600);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.0f, 0.2f, 0.0f));
    model = glm::rotate(model,(float)(30.0f * M_PI / 90.0f) * (currentTime - lastTime), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
    shader->setMat4("model", model);
    glBindTexture(GL_TEXTURE_2D, itemTexture);
    glBindVertexArray(itemVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3600);

    
    // drawing a textured cube
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cubeTexture);

    // drawing edge walls
        //top
    for (i = 0; i < 18 ; i++ ) {
        if (i == 5) continue;
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-9.0f+i, 0.0f, -9.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
        //bottom
    for (i = 0; i < 18 ; i++ ) {
        if (i == 12) continue;
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-8.0f+i, 0.0f, 9.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
        //left
    for (i = 0; i < 18 ; i++ ) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-9.0f, 0.0f, -8.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
        //right
    for (i = 0; i < 18 ; i++ ) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(9.0f, 0.0f, -9.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    
    //drawing inner walls
    for (i = 0; i < 7 ; i++ ) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-7.0f, 0.0f, -8.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 2; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-6.0f+i, 0.0f, -2.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 3; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-5.0f, 0.0f, -7.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 3; i++) {
           model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-5.0f+i, 0.0f, -4.0f));
           shader->setMat4("model", model);
           glDrawArrays(GL_TRIANGLES, 0, 36);
           cube->draw(shader);
       }
    for (i = 0; i < 3; i++) {
           model = glm::mat4(1.0);
           model = glm::translate(model, glm::vec3(-3.0f, 0.0f, -3.0f+i));
           shader->setMat4("model", model);
           glDrawArrays(GL_TRIANGLES, 0, 36);
           cube->draw(shader);
       }
    for (i = 0; i < 7; i++) {
           model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-2.0f+i, 0.0f, -1.0f));
           shader->setMat4("model", model);
           glDrawArrays(GL_TRIANGLES, 0, 36);
           cube->draw(shader);
       }
    for (i = 0; i < 7; i++) {
           model = glm::mat4(1.0);
           model = glm::translate(model, glm::vec3(4.0f, 0.0f, -8.0f+i));
           shader->setMat4("model", model);
           glDrawArrays(GL_TRIANGLES, 0, 36);
           cube->draw(shader);
       }
    for (i = 0; i < 3; i++) {
           model = glm::mat4(1.0);
           model = glm::translate(model, glm::vec3(5.0f, 0.0f, -1.0f+i));
           shader->setMat4("model", model);
           glDrawArrays(GL_TRIANGLES, 0, 36);
           cube->draw(shader);
       }
    for (i = 0; i < 3; i++) {
           model = glm::mat4(1.0);
           model = glm::translate(model, glm::vec3(-3.0f, 0.0f, -8.0f+i));
           shader->setMat4("model", model);
           glDrawArrays(GL_TRIANGLES, 0, 36);
           cube->draw(shader);
       }
    for (i = 0; i < 5; i++) {
           model = glm::mat4(1.0);
           model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -7.0f+i));
           shader->setMat4("model", model);
           glDrawArrays(GL_TRIANGLES, 0, 36);
           cube->draw(shader);
       }
    for (i = 0; i < 3; i++) {
           model = glm::mat4(1.0);
           model = glm::translate(model, glm::vec3(0.0f+i, 0.0f, -3.0f));
           shader->setMat4("model", model);
           glDrawArrays(GL_TRIANGLES, 0, 36);
           cube->draw(shader);
       }
    for (i = 0; i < 4; i++) {
           model = glm::mat4(1.0);
           model = glm::translate(model, glm::vec3(0.0f+i, 0.0f, -5.0f));
           shader->setMat4("model", model);
           glDrawArrays(GL_TRIANGLES, 0, 36);
           cube->draw(shader);
       }
    for (i = 0; i < 2; i++) {
           model = glm::mat4(1.0);
           model = glm::translate(model, glm::vec3(1.0f, 0.0f, -8.0f+i));
           shader->setMat4("model", model);
           glDrawArrays(GL_TRIANGLES, 0, 36);
           cube->draw(shader);
       }
    for (i = 0; i < 2; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(6.0f+i, 0.0f, -7.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 4; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(6.0f, 0.0f, -6.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 2; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(7.0f, 0.0f, -3.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 2; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 3; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(1.0f+i, 0.0f, 1.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 2; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-5.0f+i, 0.0f, 1.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 3; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-7.0f+i, 0.0f, 0.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 7; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-7.0f, 0.0f, 1.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 2; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-6.0f+i, 0.0f, 3.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 5; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-6.0f+i, 0.0f, 7.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 3; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 5.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 3; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f+i, 0.0f, 5.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 2; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 6.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 2; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, 7.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 2; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-5.0f+i, 0.0f, 5.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 3; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 3.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 3; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, 1.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 9; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-1.0f+i, 0.0f, 3.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 3; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(7.0f, 0.0f, 0.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 4; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(4.0f, 0.0f, 4.0f+i));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 3; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(5.0f+i, 0.0f, 7.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    for (i = 0; i < 3; i++) {
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(6.0f+i, 0.0f, 5.0f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        cube->draw(shader);
    }
    model = glm::mat4(1.0);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, -7.0f));
    shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    cube->draw(shader);
    
    model = glm::mat4(1.0);
    model = glm::translate(model, glm::vec3(5.0f, 0.0f, 8.0f));
    shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    cube->draw(shader);
    
    model = glm::mat4(1.0);
    model = glm::translate(model, glm::vec3(8.0f, 0.0f, -5.0f));
    shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    cube->draw(shader);
    
    model = glm::mat4(1.0);
    model = glm::translate(model, glm::vec3(8.0f, 0.0f, -3.0f));
    shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    cube->draw(shader);
    glfwSwapBuffers(mainWindow);
}

// glAllInit();
GLFWwindow *glAllInit()
{
	GLFWwindow *window;

	// glfw: initialize and configure
	if (!glfwInit()) {
		printf("GLFW initialisation failed!");
		glfwTerminate();
		exit(-1);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// glfw window creation
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "The Maze Runner", NULL, NULL);
	if (window == NULL) {
		cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// OpenGL states
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// Allow modern extension features
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		cout << "GLEW initialisation failed!" << endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	return window;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		else {
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		camArcBall.init(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
		cameraPos = cameraOrigPos;
	}
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	camArcBall.mouseButtonCallback(window, button, action, mods);
}

void cursor_position_callback(GLFWwindow *window, double x, double y) {
	camArcBall.cursorCallback(window, x, y);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	cameraPos[2] -= (yoffset * 0.5);
}
