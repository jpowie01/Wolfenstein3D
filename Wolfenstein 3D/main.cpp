//
//  main.cpp
//  Wolfenstein 3D
//
//  Created by Jakub Powierza on 26.11.2014.
//  Copyright (c) 2014 Jakub Powierza. All rights reserved.
//

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <stdio.h>
#include <string>
#include <fstream>

using namespace std;


// Door structure
struct DoorStruct {
public:
    float animation;
    float animationTime;
};

// End of game
bool endOfGameFlag = false;

// Keyboard
bool arrowUp = false;
bool arrowDown = false;
bool arrowLeft = false;
bool arrowRight = false;
bool spacebar = false;

// Camera settings
float cameraX = 0.0f;
float cameraY = 0.0f;

// Player position
float playerPositionX = 2.5f;
float playerPositionY = 0.0f;
float playerPositionZ = 2.5f;

// Player's speed
float playerSpeed = 0.08f;

// Map's settings
int mapWidth;
int mapHeight;
int **map;
DoorStruct **allDoors;
const float wallThickness = 0.20;
const float doorThickness = 0.07;

// Screen settings
const int screenWidth = 1200;
const int screenHeight = 800;

// SDL initialization
bool initializeSDL();

// OpenGL initialization
bool initializeOpenGL();

// Keyboard manipulation
void keyboardManipulation(unsigned char key);

// Frame updater
void updateFrame();

// Scene renderer
void renderScene();

// Exit SDL
void exitSDL();

// Draw single wall
void drawSigleWall(float x1, float y1, float x2, float y2);

// Draw double wall
void drawDoubleWall(float x, float y, int drawingMode);

// Draw single door
void drawSingleDoor(float x1, float y1, float x2, float y2);

// Draw double door
void drawDoubleDoor(float x, float y, int drawingMode);

// Draw floor
void drawFloor(float x1, float y1, float x2, float y2);

// Main window
SDL_Window* mainWindow = NULL;

// Drawing context
SDL_GLContext drawingContext;

// Textures
SDL_Surface* textures;

// All read textures
GLuint readTextures[2];

// Reading textures from file
SDL_Surface* readTexturesFromFile(char* fileName);

// Load single texture
void loadSingleTexture(int id, SDL_Surface *textures);

// Delete texture
void deleteTexture(GLuint texture);

// Trim texture
SDL_Surface* trimTexture(SDL_Surface* texture, int x, int y, int width, int height);

// Get pixel color from a texture
Uint32 getPixelColor(SDL_Surface *texture, int x, int y);

// Main function
int main (int argc, char* args[]) {
    // Initialization
    if(!initializeSDL()) {
        printf( "Error while initializing SDL...\n" );
        return 1;
    } else {
        // Read map file
        SDL_Surface *mapFile = readTexturesFromFile("map.bmp");
        mapWidth = mapFile->w;
        mapHeight = mapFile->h;
        map = new int* [mapWidth];
        allDoors = new DoorStruct* [mapWidth];
        for (int x = 0; x < mapWidth; x++) {
            map[x] = new int[mapHeight];
            allDoors[x] = new DoorStruct[mapHeight];
        }
        for (int y = 0; y < mapHeight; y++) {
            for (int x = 0; x < mapWidth; x++) {
                int color = getPixelColor(mapFile, x, y);
                map[x][y] = 0;
                allDoors[x][y].animation = 0.0f;
                if (color == 16777215) {
                    map[x][y] = 1;
                }
                else if (color == 16711680) {
                    map[x][y] = 2;
                    allDoors[x][y].animation = 1.0f;
                }
                else if (color == 255) {
                    playerPositionX = x + 0.5f;
                    playerPositionZ = y + 0.5f;
                    if (getPixelColor(mapFile, x-1, y) == 16776960) {
                        cameraX = 270;
                    }
                    else if (getPixelColor(mapFile, x, y-1) == 16776960) {
                        cameraX = 0;
                    }
                    else if (getPixelColor(mapFile, x+1, y) == 16776960) {
                        cameraX = 90;
                    }
                    else if (getPixelColor(mapFile, x, y+1) == 16776960) {
                        cameraX = 180;
                    }
                }
            }
        }
        
        // Read all textures
        textures = readTexturesFromFile("textures.bmp");
        
        // Trim it to smaller textures
        loadSingleTexture(0, trimTexture(textures, 128, 128, 64, 64));
        loadSingleTexture(1, trimTexture(textures, 128, 1024, 64, 64));
        
        // Event handler
        SDL_Event event;
        
        // Turn on typing
        SDL_StartTextInput();
        
        // Main game loop
        while (!endOfGameFlag) {
            // Get event from queue
            while (SDL_PollEvent(&event) != 0) {
                // Quit game
                if (event.type == SDL_QUIT) {
                    endOfGameFlag = true;
                }
                // Input text
                else if (event.type == SDL_TEXTINPUT) {
                    keyboardManipulation(event.text.text[0]);
                }
                // Walking
                else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
                        arrowUp = true;
                    } else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                        arrowDown = true;
                    } else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                        arrowLeft = true;
                    } else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                        arrowRight = true;
                    } else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                        spacebar = true;
                    }
                } else if (event.type == SDL_KEYUP) {
                    if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
                        arrowUp = false;
                    } else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                        arrowDown = false;
                    } else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                        arrowLeft = false;
                    } else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                        arrowRight = false;
                    }
                }
            }
            
            // Camera
            if (arrowLeft) cameraX -= 2.0f;
            if (arrowRight) cameraX += 2.0f;
            
            // Walking
            if (arrowDown) {
                if (map[(int)(playerPositionX - sin(cameraX*M_PI/180.0f) * playerSpeed)][(int)(playerPositionZ)] == 0 ||
                    map[(int)(playerPositionX - sin(cameraX*M_PI/180.0f) * playerSpeed)][(int)(playerPositionZ)] == 3) {
                    playerPositionX -= sin(cameraX*M_PI/180.0f) * playerSpeed;
                }
                if (map[(int)(playerPositionX)][(int)(playerPositionZ + cos(cameraX*M_PI/180.0f) * playerSpeed)] == 0 ||
                    map[(int)(playerPositionX)][(int)(playerPositionZ + cos(cameraX*M_PI/180.0f) * playerSpeed)] == 3) {
                    playerPositionZ += cos(cameraX*M_PI/180.0f) * playerSpeed;
                }
            }
            if (arrowUp) {
                if (map[(int)(playerPositionX + sin(cameraX*M_PI/180.0f) * playerSpeed)][(int)(playerPositionZ)] == 0 ||
                    map[(int)(playerPositionX + sin(cameraX*M_PI/180.0f) * playerSpeed)][(int)(playerPositionZ)] == 3) {
                    playerPositionX += sin(cameraX*M_PI/180.0f) * playerSpeed;
                }
                if (map[(int)(playerPositionX)][(int)(playerPositionZ - cos(cameraX*M_PI/180.0f) * playerSpeed)] == 0 ||
                    map[(int)(playerPositionX)][(int)(playerPositionZ - cos(cameraX*M_PI/180.0f) * playerSpeed)] == 3) {
                    playerPositionZ -= cos(cameraX*M_PI/180.0f) * playerSpeed;
                }
            }
            
            // Opening doors
            for (int y = 0; y < mapHeight; y++) {
                for (int x = 0; x < mapWidth; x++) {
                    if (map[x][y] == 2 && allDoors[x][y].animation < 1.0f) {
                        allDoors[x][y].animation -= 0.05f;
                        if (allDoors[x][y].animation <= 0.0501f) {
                            allDoors[x][y].animation = 0.05f;
                            map[x][y] = 3;
                        }
                    } else if (map[x][y] == 3) {
                        if (allDoors[x][y].animationTime <= SDL_GetTicks() && ((int)playerPositionX != x || (int)playerPositionZ != y)) {
                            map[x][y] = 4;
                        }
                    } else if (map[x][y] == 4 && allDoors[x][y].animation < 1.0f) {
                        allDoors[x][y].animation += 0.05f;
                        if (allDoors[x][y].animation >= 0.9501f) {
                            allDoors[x][y].animation = 1.0f;
                            map[x][y] = 2;
                        }
                    }
                }
            }
            
            // Open doors
            if (spacebar == true) {
                for (int y = -1; y <= 1; y++) {
                    for (int x = -1; x <= 1; x++) {
                        if (map[(int)playerPositionX + x][(int)playerPositionZ + y] == 2 && allDoors[(int)playerPositionX + x][(int)playerPositionZ + y].animation >= 0.9999f) {
                            float xKwadrat = playerPositionX - ((int)playerPositionX + x + 0.5);
                            xKwadrat *= xKwadrat;
                            float yKwadrat = playerPositionZ - ((int)playerPositionZ + y + 0.5);
                            yKwadrat *= yKwadrat;
                            if (xKwadrat + yKwadrat <= 1.0) {
                                allDoors[(int)playerPositionX + x][(int)playerPositionZ + y].animation -= 0.05;
                                allDoors[(int)playerPositionX + x][(int)playerPositionZ + y].animationTime = SDL_GetTicks() + 3000;
                            }
                        }
                    }
                }
                spacebar = false;
            }
            
            // Update frame
            updateFrame();
            
            // Render sceen
            renderScene();
            
            // Update window
            SDL_GL_SwapWindow(mainWindow);
        }
        
        // Disable text input
        SDL_StopTextInput();
    }
    
    // Remove all SDL things
    exitSDL();
    
    return 0;
}

// Initialize SDL
bool initializeSDL() {
    // Success flag
    bool success = true;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE) < 0) {
        printf("SDL initialization error: %s\n", SDL_GetError());
        success = false;
    } else {
        // Use OpenGL 2.1
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
        
        // Create window
        mainWindow = SDL_CreateWindow("Wolfenstein 3D", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (mainWindow == NULL) {
            printf("Window cannot be created! Error: %s\n", SDL_GetError());
            success = false;
        } else {
            // Create drawing context
            drawingContext = SDL_GL_CreateContext(mainWindow);
            if (drawingContext == NULL) {
                printf("OpenGL context could not be created! Error: %s\n", SDL_GetError());
                success = false;
            } else {
                // Enable V-Sync
                if (SDL_GL_SetSwapInterval(1) < 0) {
                    printf("Could not enable V-Sync! Error: %s\n", SDL_GetError());
                }
                
                // Initialize OpenGL
                if (!initializeOpenGL()) {
                    printf("Could not initialize OpenGL!\n");
                    success = false;
                }
            }
        }
    }
    
    return success;
}

// Initialize OpenGL
bool initializeOpenGL()
{
    // Flags
    bool success = true;
    GLenum error = GL_NO_ERROR;
    
    // Initialize projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Check errors
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("OpenGL initialization failed! Error: %s\n", gluErrorString(error));
        success = false;
    }
    
    // Initialize view model matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Check errors
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("OpenGL initialization failed! Error: %s\n", gluErrorString(error));
        success = false;
    }
    
    // Turn on depth buffer
    glEnable(GL_DEPTH_TEST);
    
    // Turn on normalization
    glEnable(GL_NORMALIZE);
    
    // Turn on blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Turn on smoothing
    glShadeModel(GL_SMOOTH);
    glDepthRange(0.0f, 1.0f);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    // Clear screen
    glClearColor(42.0f/255.0f, 42.0f/255.0f, 42.0f/255.0f, 1.0f);
    
    // Check errors
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("OpenGL initialization failed! Error: %s\n", gluErrorString(error));
        success = false;
    }
    
    return success;
}

// Key manipulation
void keyboardManipulation(unsigned char key)
{
    // Turn off game
    //    if (key == 'q') {
    //        endOfGameFlag = true;
    //    }
}

// Update whole frame
void updateFrame()
{
    // Set view
    glViewport((screenWidth-screenHeight) / 2, 0, screenHeight, screenHeight );
}

// Render whole scene
void renderScene()
{
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup view
    glViewport(0, 0, screenWidth, screenHeight);
    
    // Turn on matrix view
    glMatrixMode(GL_MODELVIEW);
    
    // Reset view and transformation
    glLoadIdentity();
    
    // Set perspective
    gluPerspective(60.0, 1.0*screenWidth/screenHeight, 0.05f, 500.0f);
    
    // Rotate camera
    glRotatef(cameraY, 1.0, 0.0, 0.0);
    glRotatef(cameraX, 0.0, 1.0, 0.0);
    
    // Push matrix on stack
    glPushMatrix();
    
    // Translate matrix over player's position
    glTranslatef(-playerPositionX, -playerPositionY-0.5f, -playerPositionZ);
    
    // Draw map
    for (int x = 0; x < mapWidth; x++) {
        for (int y = 0; y < mapHeight; y++) {
            // Draw wall
            if (map[x][y] == 1) {
                // Prepare corners and walls
                int drawingMode = 6;
                bool corners[4] = {false, false, false, false};
                int cornersCount = 0;
                if ((y >= 1 && map[x][y-1] != 0) || (y <= mapHeight-2 && map[x][y+1] != 0)) drawingMode = 0;
                if ((x >= 1 && map[x-1][y] != 0) || (x <= mapWidth-2 && map[x+1][y] != 0)) drawingMode = 1;
                if ((x >= 1 && map[x-1][y] != 0) && (y <= mapWidth-2 && map[x][y+1] != 0)) {drawingMode = 2; corners[0] = true; cornersCount++;}
                if ((x <= mapWidth-2 && map[x+1][y] != 0) && (y <= mapWidth-2 && map[x][y+1] != 0)) {drawingMode = 3; corners[1] = true; cornersCount++;}
                if ((x <= mapWidth-2 && map[x+1][y] != 0) && (y >= 1  && map[x][y-1] != 0)) {drawingMode = 4; corners[2] = true; cornersCount++;}
                if ((x >= 1  && map[x-1][y] != 0) && (y >= 1  && map[x][y-1] != 0)) {drawingMode = 5; corners[3] = true; cornersCount++;}
                
                // Default wall and wall type L
                if (cornersCount == 0 || cornersCount == 1) {
                    drawDoubleWall(x, y, drawingMode);
                }
                // Type T
                else if (cornersCount == 2) {
                    if (corners[0] && corners[3]) {
                        drawDoubleWall(x, y, 0);
                        // Half of the wall
                        drawSigleWall(x, y+0.5-wallThickness, x+0.5, y+0.5-wallThickness);
                        drawSigleWall(x, y+0.5+wallThickness, x+0.5, y+0.5+wallThickness);
                        drawSigleWall(x, y+0.5-wallThickness, x, y+0.5+wallThickness);
                    }
                    if (corners[0] && corners[1]) {
                        drawDoubleWall(x, y, 1);
                        // Half of the wall
                        drawSigleWall(x+0.5-wallThickness, y+0.5, x+0.5-wallThickness, y+1.0);
                        drawSigleWall(x+0.5+wallThickness, y+0.5, x+0.5+wallThickness, y+1.0);
                        drawSigleWall(x+0.5-wallThickness, y+1.0, x+0.5+wallThickness, y+1.0);
                    }
                    if (corners[1] && corners[2]) {
                        drawDoubleWall(x, y, 0);
                        // Half of the wall
                        drawSigleWall(x+0.5, y+0.5-wallThickness, x+1.0, y+0.5-wallThickness);
                        drawSigleWall(x+0.5, y+0.5+wallThickness, x+1.0, y+0.5+wallThickness);
                        drawSigleWall(x+1.0, y+0.5-wallThickness, x+1.0, y+0.5+wallThickness);
                    }
                    if (corners[2] && corners[3]) {
                        drawDoubleWall(x, y, 1);
                        // Half of the wall
                        drawSigleWall(x+0.5-wallThickness, y, x+0.5-wallThickness, y+0.5);
                        drawSigleWall(x+0.5+wallThickness, y, x+0.5+wallThickness, y+0.5);
                        drawSigleWall(x+0.5-wallThickness, y, x+0.5+wallThickness, y);
                    }
                }
                // Type X
                else if (cornersCount == 4) {
                    drawDoubleWall(x, y, 0);
                    drawDoubleWall(x, y, 1);
                }
            }
            // Draw doors
            else if (map[x][y] == 2 || map[x][y] == 3 || map[x][y] == 4) {
                if ((y >= 1 && map[x][y-1] != 0) || (y <= mapHeight-2 && map[x][y+1] != 0)) drawDoubleDoor(x, y, 0);
                else if ((x >= 1 && map[x-1][y] != 0) || (x <= mapWidth-2 && map[x+1][y] != 0)) drawDoubleDoor(x, y, 1);
            }
            
            // Draw floor
            drawFloor(x, y, x + 1, y + 1);
        }
    }
    
    // Release matrix from stack
    glPopMatrix();
    
    // Clear buffers
    glFlush();
}

// End up using SDL
void exitSDL()
{
    // Destroy the main window
    SDL_DestroyWindow(mainWindow);
    mainWindow = NULL;
    
    // Quit SDL
    SDL_Quit();
}

// Draw single wall
void drawSigleWall(float x1, float y1, float x2, float y2) {
    // Choose texture to draw
    glBindTexture(GL_TEXTURE_2D, readTextures[0]);
    
    // Setup texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Enable using texture
    glEnable(GL_TEXTURE_2D);
    
    // Setup color
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    // Calculate width
    float width = (x2 - x1) + (y2 - y1);
    
    // Draw
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(x1, 0.0f, y1);
        glTexCoord2f(width, 1.0f);
        glVertex3f(x2, 0.0f, y2);
        glTexCoord2f(width, 0.0f);
        glVertex3f(x2, 1.0f, y2);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x1, 1.0f, y1);
    glEnd();
    
    // Disable using textures
    glDisable(GL_TEXTURE_2D);
}

// Draw double wall
void drawDoubleWall(float x, float y, int drawingMode) {
    if (drawingMode == 0) {
        // Wall
        drawSigleWall(x+0.5-wallThickness, y, x+0.5-wallThickness, y+1.0);
        drawSigleWall(x+0.5+wallThickness, y, x+0.5+wallThickness, y+1.0);
        // Ending
        drawSigleWall(x+0.5-wallThickness, y+1.0, x+0.5+wallThickness, y+1.0);
        drawSigleWall(x+0.5-wallThickness, y, x+0.5+wallThickness, y);
    } else if (drawingMode == 1) {
        // Wall
        drawSigleWall(x, y+0.5-wallThickness, x+1.0, y+0.5-wallThickness);
        drawSigleWall(x, y+0.5+wallThickness, x+1.0, y+0.5+wallThickness);
        // Ending
        drawSigleWall(x+1.0, y+0.5-wallThickness, x+1.0, y+0.5+wallThickness);
        drawSigleWall(x, y+0.5-wallThickness, x, y+0.5+wallThickness);
    } else if (drawingMode == 2) {
        drawSigleWall(x, y+0.5-wallThickness, x+0.5+wallThickness, y+0.5-wallThickness);
        drawSigleWall(x, y+0.5+wallThickness, x+0.5-wallThickness, y+0.5+wallThickness);
        drawSigleWall(x+0.5-wallThickness, y+0.5+wallThickness, x+0.5-wallThickness, y+1.0);
        drawSigleWall(x+0.5+wallThickness, y+0.5-wallThickness, x+0.5+wallThickness, y+1.0);
    } else if (drawingMode == 3) {
        drawSigleWall(x+0.5-wallThickness, y+0.5-wallThickness, x+1.0, y+0.5-wallThickness);
        drawSigleWall(x+0.5+wallThickness, y+0.5+wallThickness, x+1.0, y+0.5+wallThickness);
        drawSigleWall(x+0.5-wallThickness, y+0.5-wallThickness, x+0.5-wallThickness, y+1.0);
        drawSigleWall(x+0.5+wallThickness, y+0.5+wallThickness, x+0.5+wallThickness, y+1.0);
    } else if (drawingMode == 4) {
        drawSigleWall(x+0.5+wallThickness, y+0.5-wallThickness, x+1.0, y+0.5-wallThickness);
        drawSigleWall(x+0.5-wallThickness, y+0.5+wallThickness, x+1.0, y+0.5+wallThickness);
        drawSigleWall(x+0.5-wallThickness, y, x+0.5-wallThickness, y+0.5+wallThickness);
        drawSigleWall(x+0.5+wallThickness, y, x+0.5+wallThickness, y+0.5-wallThickness);
    } else if (drawingMode == 5) {
        drawSigleWall(x+0.5-wallThickness, y, x+0.5-wallThickness, y+0.5-wallThickness);
        drawSigleWall(x+0.5+wallThickness, y, x+0.5+wallThickness, y+0.5+wallThickness);
        drawSigleWall(x, y+0.5-wallThickness, x+0.5-wallThickness, y+0.5-wallThickness);
        drawSigleWall(x, y+0.5+wallThickness, x+0.5+wallThickness, y+0.5+wallThickness);
    } else if (drawingMode == 6) {
        drawSigleWall(x+0.5+wallThickness, y+0.5-wallThickness, x+0.5+wallThickness, y+0.5+wallThickness);
        drawSigleWall(x+0.5-wallThickness, y+0.5-wallThickness, x+0.5-wallThickness, y+0.5+wallThickness);
        drawSigleWall(x+0.5-wallThickness, y+0.5+wallThickness, x+0.5+wallThickness, y+0.5+wallThickness);
        drawSigleWall(x+0.5-wallThickness, y+0.5-wallThickness, x+0.5+wallThickness, y+0.5-wallThickness);
    }
}

// Draw single door
void drawSingleDoor(float x1, float y1, float x2, float y2) {
    // Choose texture to draw
    glBindTexture(GL_TEXTURE_2D, readTextures[1]);
    
    // Setup textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Enable using texture
    glEnable(GL_TEXTURE_2D);
    
    // Choose color
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    // Drawing
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(x1, 0.0f, y1);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(x2, 0.0f, y2);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(x2, 1.0f, y2);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x1, 1.0f, y1);
    glEnd();
    
    // Disable using textures
    glDisable(GL_TEXTURE_2D);
}

// Draw double door
void drawDoubleDoor(float x, float y, int drawingMode) {
    if (drawingMode == 0) {
        // Wall
        drawSingleDoor(x+0.5-doorThickness, y+(1.0-allDoors[(int)x][(int)y].animation), x+0.5-doorThickness, y+1.0+(1.0-allDoors[(int)x][(int)y].animation));
        drawSingleDoor(x+0.5+doorThickness, y+(1.0-allDoors[(int)x][(int)y].animation), x+0.5+doorThickness, y+1.0+(1.0-allDoors[(int)x][(int)y].animation));
        // Ending
        drawSingleDoor(x+0.5-doorThickness, y+1.0+(1.0-allDoors[(int)x][(int)y].animation), x+0.5+doorThickness, y+1.0+(1.0-allDoors[(int)x][(int)y].animation));
        drawSingleDoor(x+0.5-doorThickness, y+(1.0-allDoors[(int)x][(int)y].animation), x+0.5+doorThickness, y+(1.0-allDoors[(int)x][(int)y].animation));
    } else if (drawingMode == 1) {
        // Wall
        drawSingleDoor(x+(1.0-allDoors[(int)x][(int)y].animation), y+0.5-doorThickness, x+1.0+(1.0-allDoors[(int)x][(int)y].animation), y+0.5-doorThickness);
        drawSingleDoor(x+(1.0-allDoors[(int)x][(int)y].animation), y+0.5+doorThickness, x+1.0+(1.0-allDoors[(int)x][(int)y].animation), y+0.5+doorThickness);
        // Ending
        drawSingleDoor(x+1.0+(1.0-allDoors[(int)x][(int)y].animation), y+0.5-doorThickness, x+1.0+(1.0-allDoors[(int)x][(int)y].animation), y+0.5+doorThickness);
        drawSingleDoor(x+(1.0-allDoors[(int)x][(int)y].animation), y+0.5-doorThickness, x+(1.0-allDoors[(int)x][(int)y].animation), y+0.5+doorThickness);
    }
}

// Draw floor
void drawFloor(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
        glColor3f(93.0f/255.0f, 93.0f/255.0f, 93.0f/255.0f);
        glVertex3f(x1, 0.0f, y1);
        glVertex3f(x2, 0.0f, y1);
        glVertex3f(x2, 0.0f, y2);
        glVertex3f(x1, 0.0f, y2);
    glEnd();
}

// Load all textures from file
SDL_Surface* readTexturesFromFile(char* fileName) {
    // Load file from disk
    SDL_Surface* textures = SDL_LoadBMP(fileName);
    
    // Error
    if (textures == NULL) {
        printf("Error while loading file: \"%s\"\n", SDL_GetError());
    }
    
    // Return textures
    return textures;
}

// Load texture
void loadSingleTexture(int id, SDL_Surface *texture) {
    // Prepare space
    glGenTextures(1, &readTextures[id]);
    
    // Generate texture
    glBindTexture(GL_TEXTURE_2D, readTextures[id]);
    
    // Copy texture into graphics memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_BGR, GL_UNSIGNED_BYTE, texture->pixels);
}

// Delete texture from memory
void deleteTexture(GLuint texture) {
    // Delete texture from memory
    glDeleteTextures(1, &texture);
}

// Trim part of a picture
SDL_Surface* trimTexture(SDL_Surface* texture, int x, int y, int width, int height) {
    SDL_Surface* newTexture = SDL_CreateRGBSurface(texture->flags, width, height, texture->format->BitsPerPixel, texture->format->Rmask, texture->format->Gmask, texture->format->Bmask, texture->format->Amask);
    SDL_Rect rect = {x, y, width, height};
    SDL_BlitSurface(texture, &rect, newTexture, 0);
    return newTexture;
}

// Get pixel color from texture
Uint32 getPixelColor(SDL_Surface *texture, int x, int y)
{
    int bpp = texture->format->BytesPerPixel;
    Uint8 *ptr = (Uint8 *)texture->pixels + y * texture->pitch + x * bpp;
    
    switch (bpp) {
        case 1:
            return *ptr;
            break;
            
        case 2:
            return *(Uint16 *)ptr;
            break;
            
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return ptr[0] << 16 | ptr[1] << 8 | ptr[2];
            else
                return ptr[0] | ptr[1] << 8 | ptr[2] << 16;
            break;
            
        case 4:
            return *(Uint32 *)ptr;
            break;
            
        default:
            // Shouldn't happen
            return 0;
    }
}
