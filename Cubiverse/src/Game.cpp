#include "stdafx.h"

#include "Game.h"
#include "System.h"
#include "Window.h"
#include "Config.h"
#include "Input.h"

#include "player/Player.h"
#include "world/World.h"

#include "graphics/Graphics.h"
#include "graphics/WorldRenderer.h"

Player*		  Game::player;
World*		   Game::world;
WorldRenderer*   Game::worldRenderer;

/*double fpsTicks;
int fpsCount;
int currentFPS;*/

bool Game::Init() {
	world = new World();
	world->Init(4, 4, 4);
	world->Fill(Block::Air);
	world->Generate();
	
	worldRenderer = new WorldRenderer();

	if (!worldRenderer->Init(world)) {
		printerr("Failed to initialize world renderer.");
		return false;
	}

	player = new Player();
	player->pos = world->width.ToDouble() / 2.0;
	player->pos.z = world->width.z - 8;
	player->playerUp = Vector3I(0, 0, 1);
	player->smoothUp = Vector3D(0, 0, 1);

	player->Init();

	Graphics::things.push_back(worldRenderer);
	Graphics::things.push_back(player);

	//f.LoadFromFile("res/consola.ttf");
	
	Input::Unlock();

	return true;
}

void Game::Start() {
}

void Game::Shutdown() {
	player->Shutdown();
	worldRenderer->Shutdown();
	world->Shutdown();
}

void Game::KeyCallback(int key, int action) {
	if (key == Config::Key::NoClip) {
		Game::player->noclip = !Game::player->noclip;
	}
}

void Game::Update() {
	static double oldTime = glfwGetTime();
	static double newTime;
	static double tick = TickMS / 1000.0;
	static double accum = tick;

	newTime = glfwGetTime();

	double delta = newTime - oldTime;
	accum += delta;

	//fpsTicks += delta;
	//fpsCount++;

	while (accum >= tick) {
		Tick();
		accum -= tick;
	}

	oldTime = newTime;
}

void Game::Tick() {
	Window::DoEvents();
	player->Tick();
}

void Game::Render() {
	Graphics::ClearAll();
	/*glLineWidth(4);
	Model* m = ModelFactory::CreateWireframeDebugBox(BoundingBox(-world->level.offset.ToDouble() * Chunk::DIM, (-world->level.offset).Offset(world->level.Dim()).ToDouble() * Chunk::DIM), ColorF(1, 0, 0, 1));
	m->Render();
	m->Shutdown();*/
	Graphics::Render();
	DrawGUI();
}

void Game::DrawGUI() {
	Graphics::ClearDepth();
	player->DrawGUI();

	/*Window::sfWindow.SaveGLStates();

	double factor = (double)Window::Height() / 800.0;

	Vector3D p = Game::player->pos - Game::world->width.ToDouble() / 2.0;
	sf::Text text("Position: " + p.ToString(2), f, 20 * factor);
	text.SetPosition(5 * factor, 0);
	Window::sfWindow.Draw(text);*/

	/*if (fpsTicks >= freq / 2) {
		currentFPS = fpsCount;
		fpsTicks = 0;
		fpsCount = 0;
		glfwSetWindowTitle(("Cubiverse " + Config::Version + " FPS: " + str(currentFPS * 2)).c_str());
	}*/
}