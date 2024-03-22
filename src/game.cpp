#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"

#include <cmath>



//some globals
Mesh* mesh = NULL;
Texture* texture = NULL;
Shader* shader = NULL;
Animation* anim = NULL;
//float angle = 0;
float mouse_speed = 100.0f;
FBO* fbo = NULL;

Game* Game::instance = NULL;

// PAU
Scene* scene;
EntityMesh* house;
EntityMesh* tile;
//StageManager* stageManager;
Texture* lavalava;

HSAMPLE Game::loadSample(const char* filename, bool loop)
{
	//El handler para un sample
	HSAMPLE hSample;

	//El handler para un canal
	HCHANNEL hSampleChannel;

	//Cargamos un sample del disco duro (memoria, filename, offset, length, max, flags)
	//use BASS_SAMPLE_LOOP in the last param to have a looped sound
	if (loop)
		hSample = BASS_SampleLoad(false, filename, 0, 0, 3, BASS_SAMPLE_LOOP);
	if(!loop)
		hSample = BASS_SampleLoad(false, filename, 0, 0, 3, 0);

	if (hSample == 0)
	{
		//file not found
		std::cout << "ERROR load audio";
	}
	//std::cout << " + AUDIO load " << filename << std::endl;
	return hSample;
}

void Game::PlayGameSound(const char* filename, bool loop)
{

	HSAMPLE hSample = loadSample(filename, loop);

	HCHANNEL hSampleChannel;
	hSampleChannel = BASS_SampleGetChannel(hSample, false);

	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);
}

Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	gameover = false;

	//OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->move(Vector3(-80, 0, 0));
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective

	//load one texture without using the Texture Manager (Texture::Get would use the manager)
	texture = new Texture();
 	texture->load("data/texture.tga");

	// example of loading Mesh from Mesh Manager
	mesh = Mesh::Get("data/box.ASE");

	// example of shader loading using the shaders manager
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse

	// PAU MODIFICATIONS
	scene = new Scene();
	Mesh* mesh2 = new Mesh();
	mesh2 = Mesh::Get("data/farm-house_4.obj");

	Texture* texture2 = new Texture();
	texture2 = Texture::Get("data/color-atlas-new.png");
	
	house = new EntityMesh("house", eMeshType::STATIC, mesh2, texture2, shader, Vector4(1, 1, 1, 1));

	stageManager = new StageManager();

	lavalava = new Texture();
	lavalava = Texture::Get("data/lava2.png");

	angle = 0;
	addtemp = false;	

	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	{
		//error abriendo la tarjeta de sonido...
		std::cout << "ERROR initializing audio" << std::endl;
	}

	PlayGameSound("data/lavasound.wav", true);

}


//what to do when the image has to be draw
void Game::render(void)
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	/*
	std::cout << camera->center.x << std::endl;
	std::cout << camera->center.y << std::endl;
	std::cout << camera->center.z << std::endl;
	std::cout << "_______" << std::endl;
	*/

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the camera as default
	camera->enable();

	//set flags
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
   
	//create model matrix for cube
	
	/*
	Matrix44 m;
	m.rotate(angle*DEG2RAD, Vector3(0, 1, 0));

	if(shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		shader->setUniform("u_color", Vector4(1,1,1,1));
		shader->setUniform("u_viewprojection", camera->viewprojection_matrix );
		shader->setUniform("u_texture", texture, 0);
		shader->setUniform("u_model", m);
		shader->setUniform("u_time", time);

		//do the draw call
		mesh->render( GL_TRIANGLES );

		//disable shader
		shader->disable();
	}
	
	*/
	


	//Draw the floor grid
	//drawGrid();

	// PAU RENDER
	scene->renderFondo();
	stageManager->GetCurrentStage()->Render();
	
	//getCurrentStage->render();
	//scene->render();
	//render the FPS, Draw Calls, etc
	//drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	
	//scene->renderSky();
	stageManager->GetCurrentStage()->wasLeftMousePressed = false;
	
	//scene->renderWater();
	

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{
	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	//example
	angle += (float)seconds_elapsed * 10.0f;

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT) || mouse_locked ) //is left button pressed?
	{
		//camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f,-1.0f,0.0f));
		//camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector( Vector3(-1.0f,0.0f,0.0f)));
	}

	//async input to move the camera around
	
	
	if (0)
	{
	if(Input::isKeyPressed(SDL_SCANCODE_LSHIFT) ) speed *= 10; //move faster with left shift
	if ( Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if ( Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f,-1.0f) * speed);
	if ( Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f,0.0f, 0.0f) * speed);
	}
	
	
	//scene->tilePos();
	//scene->update(seconds_elapsed);
	stageManager->GetCurrentStage()->Update(seconds_elapsed);



	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: Shader::ReloadAll(); break; 
		case SDLK_7: scene->spitScene(); break;
		case SDLK_8: scene->loadScene("data/levels/2.txt"); break;
		case SDLK_1: stageManager->currentStage = eStage_ID::MENU; break;

		case SDLK_9: stageManager->currentStage = eStage_ID::EDITOR; break;

		if (stageManager->GetCurrentStage()->GetId() == eStage_ID::EDITOR)
		{
			
			case SDLK_t: 
				
				if(scene->selected_tile == 0)
					scene->AddTile(camera); 
				if (scene->selected_tile == 1)
					scene->addVictoryTile(); 
				if (scene->selected_tile == 2)
					scene->addBridgeActivator();
				if (scene->selected_tile == 3)
					scene->addBridgeTile();
				break;

		}
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
	if (event.button == SDL_BUTTON_LEFT)
	{
		stageManager->GetCurrentStage()->wasLeftMousePressed = true;
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	mouse_speed *= event.y > 0 ? 1.1 : 0.9;
}

void Game::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

