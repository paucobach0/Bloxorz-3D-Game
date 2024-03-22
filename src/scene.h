#pragma once
#include <iostream>
#include "framework.h"
#include "mesh.h"
#include "texture.h"
#include "camera.h"
#include "shader.h"
#include "input.h"
#include <fstream>
#include "extra\textparser.h"

enum eMeshType { DYNAMIC, STATIC };
enum eMovement { STILL, FORWARD, BACKWARDS, LEFT, RIGHT };


class Entity
{
public:
	Entity(); //constructor
	//virtual ~Entity(); //destructor

	//some attributes
	std::string name;
	Matrix44 model;
	
	//methods overwritten by derived classes
	virtual void render() {};
	virtual void update(float elapsed_time) {};
	
	//some useful methods...
	Vector3 getPosition();
};
	
class EntityMesh : public Entity
{
public:
	//Attributes of this class
	Mesh* mesh;
	Texture* texture;
	Texture* normal_map;
	Shader* shader;
	Vector4 color;
	eMeshType type;
	Camera* cam;
	eMovement move = eMovement::STILL;
	EntityMesh(std::string name, eMeshType type, Mesh* mesh, Texture* texture, Shader* shader, Vector4 color);
	
	//methods overwritten
	void render();
	void renderMeshWithLight();
	void update(float elapsed_time);

	void managePlayer(bool isVert);
	void horizontalize(eMovement move);
	void verticalize(eMovement move);

	void verticalizeToRight();
	void verticalizeToLeft();
	void horizontalizeToRight();
	void horizontalizeToLeft();

};

class EntityLight : public Entity
{
public:
	Vector3 color; // color of the light
	float intensity; // amount of light emitted
	float max_distance; // how far the light can reach

	float cone_angle; // the angle in degrees of the cone for a spotlight
	float cone_exp; // the exponent of the cone decay
	float area_size; // the size of the volume for the directional light

	float shadow_bias;

	bool cast_shadows;
	FBO* fbo;
	Texture* shadowmap;
	Camera* light_camera;

	EntityLight();
};

class Scene
{
public: 
	
	std::vector<Entity*> entities;
	std::vector<EntityLight*> lights;

	Entity* selectedEntity = NULL;

	Mesh waterMesh;	

	int selected_tile = 0;
	int levelID;
	int n_levels;
	bool level_passed = false;
	bool addTempTile = true;
	bool activateBridge = false;
	Vector3 last_pos = Vector3(0, 0, 0);
	Vector3 xyz;

	EntityMesh* temporal_tile;

	Scene();

	void AddPlayer();
	void AddMesh(Camera* cam, std::string name, eMeshType type, Vector3 position, const char* meshName, const char* textName);


	void render();
	
	void update(float elapsed_time);
	void RayPick(Camera* cam);
	void RotateSelected(float angleDegrees);

	void tilePos();
	void AddTile(Camera* cam);
	void addVictoryTile();
	void addStartTile();
	void addBridgeActivator();
	void addBridgeTile();

	bool PlayerInTile(Vector3 curr_pos);

	void spitScene();
	void loadScene(const char* file);
	void getLastLevelID();
	void setLastLevelID();

	void renderWater();
	void renderFondo();
};