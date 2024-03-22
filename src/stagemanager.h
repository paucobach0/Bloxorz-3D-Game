#ifndef STAGE_H
#define STAGE_H
#include <iostream>
#include "texture.h"
#include "scene.h"


enum eStage_ID {MENU = 0, INTRO = 1, PLAY = 2, END = 3, EDITOR = 4, DEATH = 5, COUNT};
class Stage
{
public:
	
	Stage();

	Texture* back_to_menu;

	virtual eStage_ID GetId() = 0;
	virtual void Render() = 0;
	virtual void Update(double seconds_elapsed) = 0;

	bool wasLeftMousePressed = false;

	void renderGUI(float x, float y, float w, float h, bool flipYV, Texture* tex, Vector4 tex_range, Vector4 button_color);
	bool renderButton(float x, float y, float w, float h, bool flipYV, Texture* tex, Vector4 tex_range);

	EntityMesh* tile_temp = new EntityMesh("tile", eMeshType::STATIC, Mesh::Get("data/SM_Env_Path_4Way_01_43.obj"),
		Texture::Get("data/PolygonSamurai_Tex_01.png"), Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs"), Vector4(0.5, 0.5, 0.5, 1));

};

class StageManager {
public:

	std::vector<Stage*> stages;
	eStage_ID currentStage = eStage_ID::INTRO;

	StageManager();
	void SetStage(eStage_ID id);
	void CreateStage(eStage_ID id);
	Stage* GetStages(eStage_ID id);
	Stage* GetCurrentStage();
};

class DeathStage : public Stage {
public:

	
	DeathStage();

	Texture* tryagain;
	
	eStage_ID GetId() { return eStage_ID::DEATH; };
	void Render() override;
	void Update(double seconds_elapsed) override;

};

class IntroStage : public Stage {
public:

	IntroStage();
	bool act = false;	

	Texture* menu_tex;

	Texture* new_game;
	Texture* editor; 
	Texture* exit;
	Texture* tuto;
	Texture* tutowords;
	Texture* title;

	Mesh* intro_rectangle;
	Shader* intro_rectangle_shader;
	Texture* tex;

	eStage_ID GetId() { return eStage_ID::INTRO; };
	void Render() override;
	void Update(double seconds_elapsed) override;

	void renderIntroRectangle();
};

class MenuStage : public Stage {
public:
	
	eStage_ID GetId() { return eStage_ID::MENU; };
	void Render() override;
	void Update(double seconds_elapsed) override;
};

class PlayStage : public Stage {
public:
	
	PlayStage();
	
	int level_id;
	std::string level_name;
	
	eStage_ID GetId() { return eStage_ID::PLAY; };
	void Render() override;
	void Update(double seconds_elapsed) override;
};

class EndStage : public Stage {
public: 
	
	Texture* the_end;
	
	EndStage();

	eStage_ID GetId() { return eStage_ID::END; };
	void Render() override;
	void Update(double seconds_elapsed) override;
};

class EditorStage : public Stage {
public:

	EditorStage();

	Texture* standard;
	Texture* goal;
	Texture* switchtile;
	Texture* bridge;
	Texture* deletetex;
	Texture* save;
	Texture* player;


	EntityMesh* temporal_tile;

	eStage_ID GetId() { return eStage_ID::EDITOR; };
	void Render() override;
	void Update(double seconds_elapsed) override;
	
	void addTemporalTile();
	void manageTemporalTile();
};

#endif 