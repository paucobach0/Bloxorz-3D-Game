#include "scene.h"
#include "game.h"
#include "framework.h"

#include <iostream>
#include <filesystem>

Entity::Entity()
{
	Matrix44 model;
	this->model = model;
}

EntityMesh::EntityMesh(std::string name, eMeshType type, Mesh* mesh, Texture* texture, Shader* shader, Vector4 color)
{
	this->name = name;
	this->type = type;
	this->mesh = mesh;
	this->texture = texture;
	this->shader = shader;
	this->color = color;
	this->cam = Game::instance->camera;
	normal_map = new Texture();
	normal_map = Texture::Get("data/ultimatenormal.png");
}

void EntityMesh::render()
{
	assert(mesh != NULL, "mesh in EntityMesh::render was null");
	assert(shader != NULL, "shader in EntityMesh::render was null");

	// Frustum culling
	Vector3 meshPos = model.getTranslation();
	if (!cam->testSphereInFrustum(meshPos, mesh->radius + 15)) { return; }

	// enable shader
	shader->enable();

	// upload uniforms
	shader->setUniform("u_model", model);
	shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	shader->setUniform("u_texture", texture, 1);
	shader->setUniform("u_color", this->color);
	shader->setUniform("u_camera_position", Game::instance->camera->eye);
	//shader->setUniform("u_texture", this->texture);
	shader->setUniform("u_time", Game::instance->time);
	//shader->setUniform("u_tex_tiling", this->tiling);
	shader->setUniform("u_normal_map", normal_map, 2);


	float width = Game::instance->window_width;
	float height = Game::instance->window_height;
	shader->setUniform("uiRes", Vector2(1.0 / (float)width, 1.0 / (float)height));

	Scene* scene = Game::instance->scene;
	EntityLight* light = scene->lights[0];

	shader->setUniform("u_light_color", light->color * light->intensity); // passar a gamma al shader
	shader->setUniform("u_light_pos", light->model * Vector3());
	shader->setUniform("u_light_max_distance", light->max_distance);
	shader->setVector3("u_light_direction", light->model.frontVector());

	if (this->name == "TEMPORAL_TILE")
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	// render the mesh using the shader
	mesh->render(GL_TRIANGLES);

	//mesh->renderBounding(model);

	// disable the shader after finishing rendering
	shader->disable();

	if (this->name == "TEMPORAL_TILE")
	{
		glDisable(GL_BLEND);
	}

	//renderMeshWithLight();
}

void EntityMesh::renderMeshWithLight()
{
	assert(mesh != NULL, "mesh in EntityMesh::render was null");
	assert(shader != NULL, "shader in EntityMesh::render was null");
	Scene* scene = Game::instance->scene;
	
	Camera* cam = Camera::current;

	// Frustum culling
	Vector3 meshPos = model.getTranslation();
	if (!cam->testSphereInFrustum(meshPos, mesh->radius + 15)) { return; }

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	// enable shader
	shader->enable();

	float width = Game::instance->window_width;
	float height = Game::instance->window_height;


	// upload uniforms
	shader->setUniform("u_model", model);
	shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	shader->setUniform("u_texture", texture, 0);
	shader->setUniform("u_color", this->color);
	shader->setUniform("u_time", Game::instance->time);
	shader->setUniform("uiRes", Vector2(1.0 / (float)width, 1.0 / (float)height));

	for (int i = 0; i < scene->lights.size(); i++)
	{
		EntityLight* light = scene->lights[i];

		shader->setUniform("u_light_color", light->color * light->intensity); // passar a gamma al shader
		shader->setUniform("u_light_pos", light->model * Vector3());
		shader->setUniform("u_light_max_distance", light->max_distance);
		shader->setVector3("u_light_direction", light->model.frontVector());

		mesh->render(GL_TRIANGLES);
		//shader->setUniform("u_ambient_light", Vector3());
	}

	shader->disable();
	glDisable(GL_BLEND);
	//glDepthFunc(GL_LESS);
}

void EntityMesh::update(float elapsed_time)
{

	Scene* scene = Game::instance->scene;
	Camera* camera = Game::instance->camera;

	bool isVertical = Game::instance->isVertical;

	//enum eMovement { STILL, FORWARD, BACKWARDS, LEFT, RIGHT };

	//eMovement move = eMovement::STILL;

	if (this->name == "PLAYER")
	{	
		Vector3 xyz;
		
		managePlayer(Game::instance->isVertical);
		
		if (Game::instance->isVertical)
		{
			xyz = this->model.getTranslation() + Vector3(0, -10 * 2, 0);
		}

		if (!Game::instance->isVertical)
		{
			if (move == eMovement::RIGHT)
			{
				xyz = this->model.getTranslation() + Vector3(11, -6, 0);
			}
			if (move == eMovement::LEFT)
			{
				xyz = this->model.getTranslation() + Vector3(-11, -6, 0);
			}
			if (move == eMovement::BACKWARDS)
			{
				xyz = this->model.getTranslation() + Vector3(0, -6, 11);
			}
			if (move == eMovement::FORWARD)
			{
				xyz = this->model.getTranslation() + Vector3(0, -6, -11);
			}
		}

		if (!Game::instance->scene->PlayerInTile(xyz))
		{

			this->model.translateGlobal(0, -elapsed_time*100, 0);
			if (this->model.getTranslation().y < -5)
			{
				//std::cout << "entro" << std::endl;
				if (Game::instance->stageManager->GetCurrentStage()->GetId() == eStage_ID::PLAY)
				{		
					Game::instance->PlayGameSound("data/death.wav", false);
					Game::instance->scene->levelID = 1;
					Game::instance->gameover = true;	
					Game::instance->scene->entities.clear();
					return;
				}

				Matrix44 m;
				m.translateGlobal(-12.5 / 2, 12.5 * 1.5, 12.5 / 2);
				m.scale(5.0f, 5.0f, 5.0f);
				this->model = m;
				Game::instance->isVertical = true;
			}
		}
	}
}

void EntityMesh::managePlayer(bool isVert)
{
	bool moving = false;

	if (Input::wasKeyPressed(SDL_SCANCODE_W))
	{
		move = eMovement::FORWARD;
		moving = true;
		Game::instance->PlayGameSound("data/rect sound.wav", false);

		if (!isVert && Game::instance->horizontalAxis == eAxis::X)
		{
			model.translateGlobal(0, 0, -12.5);
		}
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_S))
	{
		move = eMovement::BACKWARDS;
		moving = true;
		Game::instance->PlayGameSound("data/rect sound.wav", false);
		if (!isVert && Game::instance->horizontalAxis == eAxis::X)
		{
			model.translateGlobal(0, 0, 12.5);
		}
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_A))
	{
		move = eMovement::LEFT;
		moving = true;
		Game::instance->PlayGameSound("data/rect sound.wav", false);

		if (!isVert && Game::instance->horizontalAxis == eAxis::Y)
		{
			model.translateGlobal(-12.5, 0, 0);
		}
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_D))
	{
		move = eMovement::RIGHT;
		moving = true;
		Game::instance->PlayGameSound("data/rect sound.wav", false);

		if (!isVert && Game::instance->horizontalAxis == eAxis::Y)
		{
			model.translateGlobal(12.5, 0, 0);
		}
	}

	if (isVert && moving)
	{
		horizontalize(move);
		return;
	}

	if (!isVert && moving)
	{
		verticalize(move);
		return;
	}

	if (this->model.getTranslation().y < -50)
	{
		if (Game::instance->stageManager->GetCurrentStage()->GetId() == eStage_ID::PLAY)
		{
			//std::cout << "entro";
			//Game::instance->scene->entities.clear();
			//return;
		}
	}
}


void EntityMesh::horizontalize(eMovement move)
{

	switch (move)
	{
	case STILL:
		break;
	case FORWARD:
		this->model.translateGlobal(0, -12.5, -12.5*2);
		this->model.rotate(DEG2RAD * 90, Vector3(1, 0, 0));
		Game::instance->horizontalAxis = eAxis::Y;
		break;
	case BACKWARDS:
		this->model.translateGlobal(0, -12.5, 12.5*2);
		this->model.rotate(DEG2RAD * -90, Vector3(1, 0, 0));
		Game::instance->horizontalAxis = eAxis::Y;
		break;
	case LEFT:
		this->model.translateGlobal(-12.5*2, -12.5, 0.0);
		this->model.rotate(DEG2RAD * -90, Vector3(0, 0, 1));
		Game::instance->horizontalAxis = eAxis::X;
		break;
	case RIGHT:
		this->model.translateGlobal(12.5*2, -12.5, 0.0);
		this->model.rotate(DEG2RAD * 90, Vector3(0, 0, 1));
		Game::instance->horizontalAxis = eAxis::X;
		break;
	default:
		break;
	}

	Game::instance->isVertical = false;

}


void EntityMesh::verticalize(eMovement move)
{
	eAxis axis = Game::instance->horizontalAxis;

	if (axis == eAxis::X)
	{
		if (move == eMovement::RIGHT)
		{

			this->model.translateGlobal(12.5*2, 12.5, 0.0);
			this->model.rotate(DEG2RAD * -90, Vector3(0, 0, 1));
			Game::instance->isVertical = true;
		}

			
		

		if (move == eMovement::LEFT)
		{
			this->model.translateGlobal(-12.5 * 2, 12.5, 0.0);;
			this->model.rotate(DEG2RAD * 90, Vector3(0, 0, 1));
			Game::instance->isVertical = true;
		}
	}

	if (axis == eAxis::Y)
	{
		if (move == eMovement::FORWARD)
		{
			this->model.translateGlobal(0, 12.5, -12.5*2);
			this->model.rotate(DEG2RAD * -90, Vector3(1, 0, 0));
			Game::instance->isVertical = true;
		}

		if (move == eMovement::BACKWARDS)
		{
			this->model.translateGlobal(0, 12.5, 12.5*2);
			this->model.rotate(DEG2RAD * 90, Vector3(1, 0, 0));
			Game::instance->isVertical = true;
		}
	}
}

Scene::Scene()
{
	waterMesh.createPlane_v2(20000, -5);
	
	getLastLevelID();
	levelID = 1;
}

void Scene::render()
{
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i]->name == "BRIDGE_TILE" && !activateBridge)
		{
			continue;
		}
		entities[i]->render();

		if (EntityMesh* currentEntity = (EntityMesh*)entities[i])
		{
			//currentEntity->mesh->renderBounding(currentEntity->model);
		}
	}
}

void Scene::update(float elapsed_time)
{
	for (int i = 0; i < entities.size(); i++)
	{
		entities[i]->update(elapsed_time);
	}
}

void Scene::AddPlayer()
{
	Camera* cam = Game::instance->camera;
	std::string name = "PLAYER";
	eMeshType type = eMeshType::DYNAMIC;

	const char* meshName = "data/godRectangle_centre.obj";
	const char* texName = "data/averaver.png";
	
	Matrix44 model;

	model.translateGlobal(-12.5/2, 12.5 * 1.5, 12.5/2);
	model.scale(5.0f, 5.0f, 5.0f);


	// see spawn pos
	//std::cout << "(" << spawnPos.x << "," << spawnPos.z << ")";

	Mesh* mesh = Mesh::Get(meshName);
	Texture* tex = Texture::Get(texName);
	Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	EntityMesh* ent = new EntityMesh(name, type, mesh, tex, shader, Vector4(1, 1, 1, 1));
	ent->model = model;

	entities.push_back(ent);

}

void Scene::AddMesh(Camera* cam, std::string name, eMeshType type, Vector3 position, const char* meshName, const char* textName)
{

	Matrix44 model;
	model.translate(position.x, position.y, position.z);
	model.scale(5.0f, 5.0f, 5.0f);

	// see spawn pos
	//std::cout << "(" << spawnPos.x << "," << spawnPos.z << ")";

	Mesh* mesh = Mesh::Get(meshName);
	Texture* tex = Texture::Get(textName);
	Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	EntityMesh* ent = new EntityMesh(name, type, mesh, tex, shader, Vector4(1, 1, 1, 1));
	ent->model = model;

	entities.push_back(ent);
}

void Scene::tilePos()
{
	Vector3 curr_pos = last_pos;
	addTempTile = false;

	if (Input::wasKeyPressed(SDL_SCANCODE_UP))
	{
		curr_pos = (last_pos + Vector3(0, 0, -12.5f));
		addTempTile = true;
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_DOWN))
	{
		curr_pos = (last_pos + Vector3(0, 0, +12.5f));
		addTempTile = true;
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_LEFT))
	{
		curr_pos = (last_pos + Vector3(-12.5f, 0, 0));
		addTempTile = true;
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_RIGHT))
	{
		curr_pos = (last_pos + Vector3(+12.5f, 0, 0));
		addTempTile = true;
	}

	last_pos = curr_pos;
}

void Scene::AddTile(Camera* cam)
{
	Matrix44 model;
	//model.translate(spawnPos.x, spawnPos.y, spawnPos.z);
	model.translate(last_pos.x, last_pos.y, last_pos.z);
	model.scale(5.0f, 5.0f, 5.0f);

	// see spawn pos
	//std::cout << "(" << spawnPos.x << "," << spawnPos.z << ")";

	const char* mesh_name = "data/tile.obj";
	const char* tex_name = "data/ultimatealbedo.png";

	Mesh* mesh = Mesh::Get(mesh_name);
	Texture* tex = Texture::Get(tex_name);
	Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	EntityMesh* ent = new EntityMesh("TILE", eMeshType::STATIC, mesh, tex, shader, Vector4(1, 1, 1, 1));
	ent->model = model;

	entities.push_back(ent);
}

void Scene::addBridgeActivator()
{
	Matrix44 model;
	model.translate(last_pos.x, last_pos.y, last_pos.z);
	model.scale(5.0f, 5.0f, 5.0f);

	const char* mesh_name = "data/tile.obj";
	const char* tex_name = "data/ultimatealbedo.png";

	Mesh* mesh = Mesh::Get(mesh_name);
	Texture* tex = Texture::Get(tex_name);
	Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	EntityMesh* ent = new EntityMesh("BRIDGE_ACTIVATE_TILE", eMeshType::STATIC, mesh, tex, shader, Vector4(0.5, 0.5, 1, 1));
	ent->model = model;

	entities.push_back(ent);
}

void Scene::addBridgeTile()
{
	Matrix44 model;
	model.translate(last_pos.x, last_pos.y, last_pos.z);
	model.scale(5.0f, 5.0f, 5.0f);

	const char* mesh_name = "data/tile.obj";
	const char* tex_name = "data/ultimatealbedo.png";

	Mesh* mesh = Mesh::Get(mesh_name);
	Texture* tex = Texture::Get(tex_name);
	Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	if (Game::instance->stageManager->GetCurrentStage()->GetId() == eStage_ID::EDITOR)
		activateBridge = true;
	else
		activateBridge = false;

	EntityMesh* ent = new EntityMesh("BRIDGE_TILE", eMeshType::STATIC, mesh, tex, shader, Vector4(1, 1, 1, 1));
	ent->model = model;

	entities.push_back(ent);
}


void Scene::addVictoryTile()
{
	Matrix44 model;
	//model.translate(spawnPos.x, spawnPos.y, spawnPos.z);
	model.translate(last_pos.x, last_pos.y, last_pos.z);
	model.scale(5.0f, 5.0f, 5.0f);

	const char* mesh_name = "data/tile.obj";
	const char* tex_name = "data/ultimatealbedo.png";

	Mesh* mesh = Mesh::Get(mesh_name);
	Texture* tex = Texture::Get(tex_name);
	Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	EntityMesh* ent = new EntityMesh("VICTORY_TILE", eMeshType::STATIC, mesh, tex, shader, Vector4(0.1, 1, 0.1, 1));
	ent->model = model;

	entities.push_back(ent);
}

void Scene::addStartTile()
{
	Matrix44 model;
	//model.translate(spawnPos.x, spawnPos.y, spawnPos.z);
	model.translate(last_pos.x, last_pos.y, last_pos.z);
	model.scale(5.0f, 5.0f, 5.0f);

	// see spawn pos
	//std::cout << "(" << spawnPos.x << "," << spawnPos.z << ")";

	const char* mesh_name = "data/tile.obj";
	const char* tex_name = "data/ultimatealbedo.png";

	Mesh* mesh = Mesh::Get(mesh_name);
	Texture* tex = Texture::Get(tex_name);
	Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	EntityMesh* ent = new EntityMesh("START_TILE", eMeshType::STATIC, mesh, tex, shader, Vector4(1, 1, 1, 1));
	ent->model = model;

	entities.push_back(ent);
}

bool Scene::PlayerInTile(Vector3 curr_pos)
{

	bool isColliding = false;

	Vector3 character_center = curr_pos;// +Vector3(0, 4, 0);
	for (int i = 0; i < entities.size(); i++)
	{
		Entity* currentEntity = entities[i];

		if (currentEntity->name != "PLAYER")
		{
			if (EntityMesh* currentEntity = (EntityMesh*)entities[i])
			{
				Vector3 coll;
				Vector3 collnorm;

				Mesh* mesh = currentEntity->mesh;

				//comprobamos si colisiona el objeto con la esfera (radio 3)
				//std::cout << mesh->testSphereCollision(currentEntity->model, character_center, 3, coll, collnorm);
				if (mesh->testSphereCollision(currentEntity->model, character_center, 1, coll, collnorm))
				{
					isColliding = true;
					
					if (currentEntity->name == "VICTORY_TILE" && Game::instance->isVertical == true)
					{
						level_passed = true;			
					}

					if (currentEntity->name == "BRIDGE_ACTIVATE_TILE")
					{
						activateBridge = true;
					}

					if (currentEntity->name == "BRIDGE_TILE" && activateBridge == false)
					{
						return false;
					}
					break;
				}
			}
		}
	}

	return isColliding;
}

void Scene::spitScene()
{
	//std::string str = "data/levels/level.txt";
	std::string str = "data/levels/" + std::to_string(n_levels + 1) + ".txt";
	n_levels += 1;

	std::ofstream outfile(str);

	for (int i = 0; i < entities.size(); i++)
	{
		EntityMesh* currentEntity = (EntityMesh*)entities[i];

		if (currentEntity->name == "PLAYER" || currentEntity->name == "TEMPORAL_TILE")
			continue;

		// store mesh name
		outfile << currentEntity->name + "\n";

		// store mesh position
		Vector3 position = currentEntity->model.getTranslation();
		outfile << std::to_string(position.x) + " " + std::to_string(position.y) + " " + std::to_string(position.z) + "\n";

		// store mesh rotation
		// store mesh scale
		// store mesh shaders

		// store mesh file path
		std::string path;
		if (currentEntity->name == "TILE") { path = "data/tile.obj"; }
		if (currentEntity->name == "VICTORY_TILE") { path = "data/tile.obj"; }
		if (currentEntity->name == "TEMPORAL_TILE") { path = "data/tile.obj"; }
		if (currentEntity->name == "BRIDGE_TILE") { path = "data/tile.obj"; }
		if (currentEntity->name == "BRIDGE_ACTIVATE_TILE") { path = "data/tile.obj"; }

		outfile << path + "\n";

		// store texture file path
		if (currentEntity->name == "TILE") { path = "data/ultimatealbedo.png"; }
		if (currentEntity->name == "VICTORY_TILE") { path = "data/ultimatealbedo.png"; }
		if (currentEntity->name == "TEMPORAL_TILE") { path = "data/ultimatealbedo.png"; }
		if (currentEntity->name == "BRIDGE_TILE") { path = "data/ultimatealbedo.png"; }
		if (currentEntity->name == "BRIDGE_ACTIVATE_TILE") { path = "data/ultimatealbedo.png"; }

		outfile << path + "\n";

	}

	outfile << "END";


	std::cout << "scene spitted.";

	setLastLevelID();
}

void Scene::loadScene(const char* file)
{
	TextParser* parser = new TextParser(file);

	entities.clear();
	last_pos = Vector3(0, 0, 0);
	level_passed = false;
	Game::instance->isVertical = true;
	activateBridge = false;

	AddPlayer();

	while (!parser->eof())
	{

		// entity name
		std::string ent_name = parser->getword();

		if (ent_name == "END") { break; }

		//std::cout << ent_name << "\n";

		// matrix model
		float x = parser->getfloat();
		float y = parser->getfloat();
		float z = parser->getfloat();

		//std::cout << x << "\n";
		//std::cout << y << "\n";
		//std::cout << z << "\n";

		//	mesh file

		Mesh* mesh = Mesh::Get(parser->getword());
		// texture file
		Texture* tex = Texture::Get(parser->getword());

		Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

		Vector4 color;
		if (ent_name == "TILE")
			color = Vector4(1, 1, 1, 1);
		if (ent_name == "VICTORY_TILE")
			color = Vector4(0, 1, 0, 1);
		if (ent_name == "BRIDGE_TILE")
			color = Vector4(0, 0, 1, 1);
		if (ent_name == "BRIDGE_ACTIVATE_TILE")
			color = Vector4(0, 0, 0.7, 1);

		EntityMesh* ent = new EntityMesh(ent_name, eMeshType::STATIC, mesh, tex, shader, color);
		ent->model.translate(x, y, z);
		ent->model.scale(5.0f, 5.0f, 5.0f);
		entities.push_back(ent);

	}
}

EntityLight::EntityLight()
{
	color.set(1, 1, 1);
	intensity = 1;
	max_distance = 100;
	cone_angle = 60;
	cone_exp = 60;
	cast_shadows = false;
	shadow_bias = 0.001;
	area_size = 1000;

	fbo = NULL;
	shadowmap = NULL;
	light_camera = NULL;
}

void Scene::setLastLevelID()
{
	std::string str = "data/levels/n_of_levels.txt";
	std::ofstream outfile(str);

	outfile << std::to_string(this->n_levels);
}

void Scene::getLastLevelID()
{
	TextParser* parser = new TextParser("data/levels/n_of_levels.txt");

	std::string n_of_levels;

	n_of_levels = parser->getword();


	this->n_levels = std::stoi(n_of_levels);

}

void Scene::renderWater()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Texture* skyTex = Texture::Get("data/lava2.png");
	Texture* waterNormalMap = Texture::Get("data/lava_normal_ahora.png");

	Shader* a_shader = new Shader();
	a_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/water.fs");

	Camera* cam = Game::instance->camera;
	Vector3 eye = cam->eye;

	a_shader->enable();
	a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	a_shader->setUniform("u_sky_texture", skyTex, 0);
	a_shader->setUniform("u_normal_map", waterNormalMap, 4);
	a_shader->setUniform("u_cam_pos", cam->eye);

	a_shader->setUniform("u_time", Game::instance->time);
	a_shader->setUniform("u_tex_tiling", 1.0f);

	Matrix44 waterModel;
	waterModel.translate(eye.x, 0, eye.z);

	a_shader->setUniform("u_model", waterModel);

	waterMesh.render(GL_TRIANGLES);
	a_shader->disable();

	glDisable(GL_BLEND);
}

void Scene::renderFondo()
{
	Texture* fondoTex = Texture::Get("data/lava2.png");
	Mesh* fondoMesh = new Mesh();
	//fondoMesh->createPlane(999);
	fondoMesh->createPlane_v2(9999, -10);

	Shader* a_shader = new Shader();

	if(Game::instance->stageManager->GetCurrentStage()->GetId() == eStage_ID::INTRO)
		a_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture4.fs");

	if(Game::instance->stageManager->GetCurrentStage()->GetId() == eStage_ID::END)
		a_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture6.fs");

	else
		a_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture3.fs");



	Camera* cam = Game::instance->camera;
	Vector3 eye = cam->eye + Vector3(0, -50, 0);

	a_shader->enable();
	a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	a_shader->setUniform("u_texture", fondoTex, 0);
	a_shader->setUniform("u_cam_pos", cam->eye);
	a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));

	a_shader->setUniform("u_time", Game::instance->time);

	Matrix44 waterModel;
	waterModel.translate(eye.x, 0, eye.z);

	a_shader->setUniform("u_model", waterModel);

	fondoMesh->render(GL_TRIANGLES);
	a_shader->disable();

	glDisable(GL_BLEND);

}

