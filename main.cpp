#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* 
Joyce Huang 
March 1st, 2020
CS-UY 3113 Intro to Game Programming 
Project 2: Pong
*/
/* 
Instructions: Make Pong 
- Needs a paddle on each side that can move individually 
- Needs an object that bounces off the paddle and top/bottom wall 
- Does not need to keep score but must detect when someone wins 
- Can use images or untextured polygons 
- Can use keyboard, mouse or joystick input 
- You can have both players using the keyboard if you want 
*/


SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, projectionMatrix;
glm::mat4 modelMatrix; 
glm::mat4 modelMatrix_arrows;
glm::mat4 modelMatrix_wasd; 

glm::vec3 player_position = glm::vec3(0, 0, 0); 
glm::vec3 player_movement = glm::vec3(0, 0, 0); 

glm::vec3 player_position_arrows = glm::vec3(0, 0, 0);
glm::vec3 player_movement_arrows = glm::vec3(0, 0, 0);

glm::vec3 player_position_wasd = glm::vec3(0, 0, 0);
glm::vec3 player_movement_wasd = glm::vec3(0, 0, 0); 

bool startGame = false;

bool xmove = false; 
bool ymove = true;  

float player_speed = 1.0f; 
float xdist_wasd, ydist_wasd, xdist_arrows, ydist_arrows; 

GLuint LoadTexture(const char* filepath);
// GLuint playerTextureID;
// GLuint playerTextureID_2;

void draw_object(float* vertices, float* texCoords, GLuint textureID); 
void draw_square(float* vertices); 
void draw_rectangle(float* sq_left, float* sq_center, float* sq_right); 

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Project 2: Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 480);

	program.Load("shaders/vertex.glsl", "shaders/fragment.glsl"); // for the triangles
	// Load the shaders for handling textures 
	// program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl"); // for the textures 

	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	modelMatrix_arrows = glm::mat4(1.0f);
	modelMatrix_wasd = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(0.6f, 0.1f, 0.3f, 1.0f);

	glUseProgram(program.programID);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

}

void ProcessInput() {

	player_movement_arrows = glm::vec3(0); 
	player_movement_wasd = glm::vec3(0); 
	player_movement = glm::vec3(0); 

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_LEFT:
				// Move the player left
				break;

			case SDLK_RIGHT:
				// Move the player right
				break;

			case SDLK_SPACE:
				// Some sort of action
				break;
			}
			break; // SDL_KEYDOWN
		}
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	/*
	if (keys[SDL_SCANCODE_LEFT]) {
		player_movement_arrows.x = -1.0f; 
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		player_movement_arrows.x = 1.0f; 
	}
	*/

	if (keys[SDL_SCANCODE_UP]) { // up
		player_movement_arrows.y = 1.0f;
	}
	else if (keys[SDL_SCANCODE_DOWN]) { // down
		player_movement_arrows.y = -1.0f; 
	}
	
	/*
	if (keys[SDL_SCANCODE_A]) {
		player_movement_wasd.x = -1.0f; 
	}
	else if (keys[SDL_SCANCODE_D]) {
		player_movement_wasd.x = 1.0f; 
	}
	*/

	if (keys[SDL_SCANCODE_W]) { // up
		player_movement_wasd.y = 1.0f;
	}
	else if (keys[SDL_SCANCODE_S]) { // down
		player_movement_wasd.y = -1.0f;
	}

	if (keys[SDL_SCANCODE_SPACE]) {
		startGame = true; 
	}

	// holding x and y axis movement keys won't make you go faster 
	if (glm::length(player_movement_arrows) > 1.0f) {
		player_movement_arrows = glm::normalize(player_movement_arrows);
	}

	// holding x and y axis movement keys won't make you go faster 
	if (glm::length(player_movement_wasd) > 1.0f) {
		player_movement_wasd = glm::normalize(player_movement_wasd);
	}

	// holding x and y axis movement keys won't make you go faster 
	if (glm::length(player_movement) > 1.0f) {
		player_movement = glm::normalize(player_movement);
	}

}

float lastTicks = 0.0f;
void Update() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	if (startGame) {
		// true move up or right and false to move left or down 

		if ((player_position.y + 0.2f) >= 3.75f) {  // hitting top wall 
			ymove = false; 
		}
		else if ((player_position.y - 0.2f) <= -3.75f) { // hitting bottom wall 
			ymove = true; 
		} 

		// collided with paddle 
		// (player_position.x + 0.2) >= (player_position_arrows.x - 0.2)
		// (player_position_arrows.y - 0.6) <= player_position.y <= (player_position_arrows.y + 0.6)
		// (player_position_wasd.x + 0.2) >= (player_position.x - 0.2)
		// (player_position_wasd.y - 0.6) <= player_position.y <= (player_position_wasd.y + 0.6)

		// if we arrived at the area where the left paddle would touch the pong
		if ((player_position.x + 0.2f) >= 4.6f) { 
			if ((player_position_arrows.y - 0.6f) <= player_position.y <= (player_position_arrows.y + 0.6f)) {
				xmove = !xmove; 
			}
		}
		// if we arrived at the area where the right paddle would touch the pong
		else if (-4.6f >= (player_position.x - 0.2f)) {
			// if ((player_position_wasd.y - 3.0f) <= player_position.y <= (player_position_wasd.y + 2.0f)) {
			if ((player_position_wasd.y - 3.0f) <= player_position.y <= (player_position_wasd.y + 3.0f)) {
				xmove = !xmove;
			}

		}

		//xdist_wasd = fabs(player_position_wasd.x - player_position.x) - ((0.4f + 0.4f) / 2.0f);
		//ydist_wasd = fabs(player_position_wasd.y - player_position.y) - ((0.4f + 1.2f) / 2.0f);
		//
		//xdist_arrows = fabs(player_position_arrows.x - player_position.x) - ((0.4f + 0.4f) / 2.0f);
		//ydist_arrows = fabs(player_position_arrows.y - player_position.y) - ((0.4f + 1.2f) / 2.0f);

		//if (xdist_wasd < 0 && ydist_wasd < 0) { // Colliding!
		//	// bounce pong 
		//	xmove = !xmove; 
		//} 
		//if (xdist_arrows < 0 && ydist_arrows < 0) { // Colliding!
		//	// bounce pong 
		//	xmove = !xmove; 
		//} 

		if (xmove) {
			player_movement.x += 1.0f; 
		}
		else {
			player_movement.x -= 1.0f; 
		}
		if (ymove) {
			player_movement.y += 1.0f; 
		}
		else {
			player_movement.y -= 1.0f; 
		}

	}
	
	if (((player_position.x + 0.2) >= 5.0) || ((player_position.x - 0.2) <= -5.0)) {
		startGame = false;
		player_movement.x = 0;
		player_movement.y = 0;
		player_movement_arrows.x = 0;
		player_movement_arrows.y = 0;
		player_movement_wasd.x = 0;
		player_movement_wasd.y = 0;
	} 
	
	// Add (direction * units per second * elapsed time)
	player_position += player_movement * player_speed * deltaTime;
	player_position_arrows += player_movement_arrows * player_speed * deltaTime;
	player_position_wasd += player_movement_wasd * player_speed * deltaTime; 

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, player_position);

	modelMatrix_arrows = glm::mat4(1.0f); 
	modelMatrix_arrows = glm::translate(modelMatrix_arrows, player_position_arrows); 

	modelMatrix_wasd = glm::mat4(1.0f); 
	modelMatrix_wasd = glm::translate(modelMatrix_wasd, player_position_wasd);  
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT);
	
	float pong[] = { -0.2f, -0.2f, 0.2f, -0.2f, 0.2f, 0.2f, -0.2f, -0.2f, 0.2f, 0.2f, -0.2f, 0.2f };

	float sqLeft1[] = { -5.0f, 3.35f, -4.6f, 3.35f, -4.6f, 3.75f, -5.0f, 3.35f, -4.6f, 3.75f, -5.0f, 3.75f };
	float sqCenter1[] = { -5.0f, 2.95f, -4.6f, 2.95f, -4.6f, 3.35f, -5.0f, 2.95f, -4.6f, 3.35f, -5.0f, 3.35f };
	float sqRight1[] = { -5.0f, 2.55f, -4.6f, 2.55f, -4.6f, 2.95f, -5.0f, 2.55f, -4.6f, 2.95f, -5.0f, 2.95f }; 

	float sqLeft2[] = { 4.6f, -3.75f, 5.0f, -3.75f, 5.0f, -3.35f, 4.6f, -3.75f, 5.0f, -3.35f, 4.6f, -3.35f }; 
	float sqCenter2[] = { 4.6f, -3.35f, 5.0f, -3.35f, 5.0f, -2.95f, 4.6f, -3.35f, 5.0f, -2.95f, 4.6f, -2.95f };
	float sqRight2[] = { 4.6f, -2.95f, 5.0f, -2.95f, 5.0f, -2.55f, 4.6f, -2.96f, 5.0f, -2.55f, 4.6f, -2.55f }; 

	program.SetModelMatrix(modelMatrix_wasd);
	draw_rectangle(sqLeft1, sqCenter1, sqRight1); 

	program.SetModelMatrix(modelMatrix_arrows);
	draw_rectangle(sqLeft2, sqCenter2, sqRight2); 

	program.SetModelMatrix(modelMatrix);
	draw_square(pong);

	SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
	SDL_Quit();
}

// Helper Methods: 

GLuint LoadTexture(const char* filepath) {
	int w, h, n;
	unsigned char* image = stbi_load(filepath, &w, &h, &n, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(image);
	return textureID;
}

void draw_object(float* vertices, float* texCoords, GLuint textureID) {

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

void draw_square(float* vertices) {

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);

}

void draw_rectangle(float* sq_left, float* sq_center, float* sq_right) {
	draw_square(sq_left); 
	draw_square(sq_center); 
	draw_square(sq_right); 
}

int main(int argc, char* argv[]) {
	std::cout << "hereeeeeeeeeeeeeeeeeeeeeeeeeeeeee";
	Initialize();
	
	while (gameIsRunning) {
		ProcessInput();
		Update();
		Render();
	}

	Shutdown();
	return 0;
}