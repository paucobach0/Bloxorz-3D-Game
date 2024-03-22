#include"stagemanager.h"
#include "game.h"

Stage::Stage()
{
	back_to_menu = new Texture();
	back_to_menu = Texture::Get("data/backtomenu.png");
}

void Stage::renderGUI(float x, float y, float w, float h, bool flipYV, Texture* tex, Vector4 tex_range, Vector4 button_color)
{
	int width = Game::instance->window_width;
	int height = Game::instance->window_height;

	Mesh quad;
	quad.createQuad(x, y, w, h, flipYV);

	Camera cam2D;
	cam2D.setOrthographic(0, width, height, 0, -1, 1);

	Shader* a_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/gui.fs");
	//Texture* tex = Texture::Get("data/botblast.png");

	if (!a_shader)
		return;

	a_shader->enable();

	a_shader->setUniform("u_color", button_color);
	a_shader->setUniform("u_viewprojection", cam2D.viewprojection_matrix);

	if (tex != NULL)
		a_shader->setUniform("u_texture", tex, 0);

	a_shader->setUniform("u_text_tiling", 1.0f);
	a_shader->setUniform("u_time", time);
	a_shader->setUniform("u_tex_range", tex_range);

	Matrix44 quadModel;

	a_shader->setUniform("u_model", Matrix44());

	quad.render(GL_TRIANGLES);

	a_shader->disable();
}

bool Stage::renderButton(float x, float y, float w, float h, bool flipYV, Texture* tex, Vector4 tex_range)
{
	Vector2 mouse = Input::mouse_position;
	float halfw = w * 0.5;
	float halfh = h * 0.5;

	float min_x = x - halfw;
	float max_x = x + halfw;
	float min_y = y - halfh;
	float max_y = y + halfh;

	bool hover = (mouse.x >= min_x && mouse.x <= max_x && mouse.y >= min_y && mouse.y <= max_y);
	Vector4 buttonColor = hover ? Vector4(1, 1, 1, 1) : Vector4(1, 0.5, 1, 0.7);

	renderGUI(x, y, w, h, true, tex, tex_range, buttonColor);

	return wasLeftMousePressed && hover;
}


StageManager::StageManager() {

	//instance = this;
	//instance->currentStage = eStage_ID::PLAY;
	int all_stages = (int)eStage_ID::COUNT;
	stages.reserve(all_stages);

	stages.push_back(new MenuStage());
	stages.push_back(new IntroStage());
	stages.push_back(new PlayStage());
	stages.push_back(new EndStage());
	stages.push_back(new EditorStage());
	stages.push_back(new DeathStage());
}

void StageManager::CreateStage(eStage_ID id) {
	
	switch (id)
	{
	case MENU:
		stages.push_back(new MenuStage());
		break;
	case INTRO:
		stages.push_back(new IntroStage());
		break;
	case PLAY:
		stages.push_back(new PlayStage());
		break;
	case END:
		stages.push_back(new EndStage());
		break;
	case EDITOR:
		stages.push_back(new EditorStage());
		break;
	case DEATH:
		stages.push_back(new DeathStage());
		break;
	}
}
void StageManager::SetStage(eStage_ID id) {
	currentStage = id;
}
Stage* StageManager::GetStages(eStage_ID id) {
	return stages[(int)id];
}
Stage* StageManager::GetCurrentStage() {
	return stages[(int)currentStage];
}

// --------------------------------
// ------------DEATHSTAGE----------
// --------------------------------
DeathStage::DeathStage()
{
	tryagain = new Texture();
	tryagain = Texture::Get("data/tryagain.png");
}

void DeathStage::Render()
{
	if (renderButton(Game::instance->window_width/2.0, Game::instance->window_height / 2.0 + 200, 300, 100, true, tryagain, Vector4()))
	{
		Game::instance->stageManager->currentStage = eStage_ID::INTRO;
		Game::instance->PlayGameSound("data/ui_click.wav", false);

	}

	//Game::instance->scene->render();
}

void DeathStage::Update(double seconds_elapsed)
{
	//Game::instance->scene->update();
}



// --------------------------------
// ------------PLAYSTAGE-----------
// --------------------------------
PlayStage::PlayStage()
{
	level_id = 1;
	//scene->loadScene("data/levels/1.txt");
	//scene->AddPlayer();
}

void PlayStage::Render()
{	
	if (renderButton(50, 30, 100, 30, true, back_to_menu, Vector4()))
	{
		Game::instance->scene->entities.clear();
		Game::instance->stageManager->currentStage = eStage_ID::INTRO;
		Game::instance->PlayGameSound("data/ui_click.wav", false);
	}
	
	Game::instance->scene->render();
}

void PlayStage::Update(double seconds_elapsed)
{
	Scene* scene = Game::instance->scene;
	scene->update(seconds_elapsed);

	if (Game::instance->gameover == true)
	{
		
		if (level_id > 1)
		{
			level_id = 1; 
		}
		Game::instance->gameover = false;
		Game::instance->stageManager->currentStage = eStage_ID::DEATH;
	}


	if (scene->level_passed == true)
	{
		Game::instance->PlayGameSound("data/victory_sound.wav", false);
		scene->level_passed == false;
		std::cout << "LEVEL PASSED" << "\n";
		level_id += 1;
		if (level_id > scene->n_levels)
		{
			std::cout << "\n" << "		!!  ALL LEVELS COMPLETED  !!		" << "\n";
			
			this->level_id = 1;
			Game::instance->stageManager->SetStage(eStage_ID::END);
			return;
		}

		level_name = "data/levels/" + std::to_string(level_id) + ".txt";

		scene->loadScene(level_name.c_str());
	}

}

// --------------------------------
// ------------MENUSTAGE-----------
// --------------------------------

void MenuStage::Render()
{
	
}

void MenuStage::Update(double seconds_elapsed)
{
	return;
}

// --------------------------------
// ------------EDITORSTAGE---------
// --------------------------------
EditorStage::EditorStage()
{
	addTemporalTile();
	EntityLight* light = new EntityLight();
	light->color = Vector3(0.2, 0.7, 0.1);
	light->intensity = 2;
	float max_distance = 100;
	light->model.translateGlobal(0.0, 100, 0.0);

	Game::instance->scene->lights.push_back(light);

	standard = new Texture();
	goal = new Texture();
	switchtile = new Texture();
	bridge = new Texture();
	deletetex = new Texture();
	save = new Texture();
	player = new Texture();
	 
	standard = Texture::Get("data/standard.png");
	goal = Texture::Get("data/goal.png");
	switchtile = Texture::Get("data/switch.png");
	bridge = Texture::Get("data/bridge.png");
	deletetex = Texture::Get("data/delete.png");
	save = Texture::Get("data/save.png");
	player = Texture::Get("data/player.png");
}

void EditorStage::Render()
{ 
	Scene* scene = Game::instance->scene;

	float width = Game::instance->window_width;
	float height = Game::instance->window_height;
	
	if (renderButton(50, height - 40, 70, 50, true, standard, Vector4()))
	{
		scene->selected_tile = 0;
		Game::instance->PlayGameSound("data/ui_click.wav", false);
	}

	if (renderButton(120, height - 40, 70, 50, true, goal, Vector4()))
	{
		scene->selected_tile = 1;
		Game::instance->PlayGameSound("data/ui_click.wav", false);
	}

	if (renderButton(190, height - 40, 70, 50, true, player, Vector4()))
	{
		scene->AddPlayer();
		Game::instance->PlayGameSound("data/ui_click.wav", false);
	}

	if (renderButton(260, height - 40, 70, 50, true, switchtile, Vector4()))
	{
		scene->selected_tile = 2;
		Game::instance->PlayGameSound("data/ui_click.wav", false);
	}

	if (renderButton(330, height - 40, 70, 50, true, bridge, Vector4()))
	{
		scene->selected_tile = 3;
		Game::instance->PlayGameSound("data/ui_click.wav", false);
	}

	if (renderButton(width - 120, height - 40, 70, 50, true, save, Vector4()))
	{
		scene->spitScene();
		Game::instance->PlayGameSound("data/ui_click.wav", false);
	}
		

	if (renderButton(width - 50, height - 40, 70, 50, true, deletetex, Vector4()))
		if (scene->entities.size() > 1)
		{
			Game::instance->PlayGameSound("data/ui_click.wav", false);
			scene->entities.pop_back();
		}
			

	if (renderButton(50, 30, 100, 30, true, back_to_menu, Vector4()))
	{
		scene->entities.clear();
		Game::instance->stageManager->currentStage = eStage_ID::INTRO;
		Game::instance->addtemp == true;
		Game::instance->PlayGameSound("data/ui_click.wav", false);

	}

	if (Game::instance->addtemp == true)
	{
		addTemporalTile();

		Game::instance->addtemp = false;
	}

	
	scene->render();

}

void EditorStage::Update(double seconds_elapsed)
{
	Scene* scene = Game::instance->scene;
	// add here
	scene->tilePos();
	scene->update(seconds_elapsed);
	manageTemporalTile();
}

void EditorStage::addTemporalTile()
{
	Scene* scene = Game::instance->scene;

	Matrix44 model;
	//model.translate(spawnPos.x, spawnPos.y, spawnPos.z);
	model.translate(scene->last_pos.x, scene->last_pos.y, scene->last_pos.z);
	model.scale(5.0f, 5.0f, 5.0f);


	const char* mesh_name = "data/tile.obj";
	const char* tex_name = "data/ultimatealbedo.png";

	Mesh* mesh = Mesh::Get(mesh_name);
	Texture* tex = Texture::Get(tex_name);
	Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");


	temporal_tile = new EntityMesh("TEMPORAL_TILE", eMeshType::STATIC, mesh, tex, shader, Vector4(1, 1, 1, 0.5));
	temporal_tile->model = model;

	scene->entities.push_back(temporal_tile);
}

void EditorStage::manageTemporalTile()
{
	Scene* scene = Game::instance->scene;

	Matrix44 model;
	//model.translate(spawnPos.x, spawnPos.y, spawnPos.z);
	model.translate(scene->last_pos.x, scene->last_pos.y, scene->last_pos.z);
	model.scale(5.0f, 5.0f, 5.0f);

	temporal_tile->model = model;
}

// --------------------------------
// ------------ENDSTAGE------------
// --------------------------------
EndStage::EndStage()
{
	the_end = new Texture();
	the_end = Texture::Get("data/theend.png");
}
void EndStage::Render()
{
	Scene* scene = Game::instance->scene;

	float height = Game::instance->window_height;
	float width = Game::instance->window_width;

	if (renderButton(width/2, height/2 - 200, 100, 50, true, back_to_menu, Vector4()))
	{
		scene->entities.clear();
		Game::instance->stageManager->currentStage = eStage_ID::INTRO;
		Game::instance->PlayGameSound("data/ui_click.wav", false);

	}

	renderGUI(width / 2, height / 2, 300, 100, true, the_end, Vector4(), Vector4(1, 1, 1, 1));
}

void EndStage::Update(double seconds_elapsed)
{

}

// --------------------------------
// ------------INTROSTAGE----------
// --------------------------------
IntroStage::IntroStage()
{
	menu_tex = Texture::Get("data/botblast.png");
	intro_rectangle = new Mesh();
	intro_rectangle = Mesh::Get("data/godRectangle_centre.obj");
	intro_rectangle_shader = new Shader();
	intro_rectangle_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	tex = new Texture();
	tex = Texture::Get("data/averaver.png");

	new_game = new Texture();
	new_game = Texture::Get("data/startnewgame.png");

	editor = new Texture();
	editor = Texture::Get("data/editor.png");

	exit = new Texture();
	exit = Texture::Get("data/exit.png");

	tuto = new Texture();
	tuto = Texture::Get("data/tuto.png");

	tutowords = new Texture();
	tutowords = Texture::Get("data/tutowords.png");

	title = new Texture();
	title = Texture::Get("data/maintitle.png");
}

void IntroStage::Render()
{
	float height = Game::instance->window_height;

	if (act)
	{
		renderGUI(Game::instance->window_width / 2.0, Game::instance->window_height / 2.0, 500, 500, true, tutowords, Vector4(), Vector4(1, 1, 1, 1));

		if (renderButton(50, 30, 100, 30, true, back_to_menu, Vector4()) && act)
		{
			act = false;
			Game::instance->stageManager->currentStage = eStage_ID::INTRO;
			Game::instance->PlayGameSound("data/ui_click.wav", false);

		}
	}



	if (renderButton(Game::instance->window_width / 2.0, height / 2.0 + 60 * 2, 300, 50, true, tuto, Vector4()) && !act)
	{
		act = true;
		Game::instance->PlayGameSound("data/ui_click.wav", false);

	}

	if (renderButton(Game::instance->window_width / 2.0, height / 2.0, 300, 50, true, new_game, Vector4()) && !act)
	{
		Game::instance->gameover = false;
		Game::instance->PlayGameSound("data/ui_click.wav", false);
		Game::instance->stageManager->currentStage = eStage_ID::PLAY;
		Game::instance->scene->entities.clear();
		Game::instance->scene->loadScene("data/levels/1.txt");
		Game::instance->addtemp = true;
	}
		
	if (renderButton(Game::instance->window_width / 2.0, height / 2.0 + 60, 300, 50, true, editor, Vector4()) && !act)
	{
		Game::instance->PlayGameSound("data/ui_click.wav", false);
		Game::instance->stageManager->currentStage = eStage_ID::EDITOR;
		Game::instance->addtemp = true;
	}
	
	if (renderButton(Game::instance->window_width / 2.0, height / 2.0 + 60 * 3, 300, 50, true, exit, Vector4()) && !act)
	{
		Game::instance->must_exit = true;
		Game::instance->PlayGameSound("data/ui_click.wav", false);

	}



	if (!act)
	{
		renderGUI(Game::instance->window_width / 5.0, height / 7.0, 250, 100, true, title, Vector4(), Vector4(1,1,1,1));
		renderIntroRectangle();
	}
}

void IntroStage::Update(double seconds_elapsed)
{
	return;
}

void IntroStage::renderIntroRectangle()
{
	
	float angle = Game::instance->angle;
	Matrix44 m;
	m.translateGlobal(130, 50, 0);
	m.scale(6, 6, 6);
	m.rotate(angle * DEG2RAD * 5, Vector3(1, 1, 1));
	
	Camera* camera = Game::instance->camera;

	//enable shader
	intro_rectangle_shader->enable();

	//upload uniforms
	intro_rectangle_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	intro_rectangle_shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	intro_rectangle_shader->setUniform("u_texture", tex, 0);
	intro_rectangle_shader->setUniform("u_model", m);
	intro_rectangle_shader->setUniform("u_time", time);

	//do the draw call
	intro_rectangle->render(GL_TRIANGLES);

	//disable shader
	intro_rectangle_shader->disable();
}
