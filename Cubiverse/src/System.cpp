#include "stdafx.h"

#include "System.h"
#include "Window.h"
#include "Game.h"
#include "Input.h"
#include "Config.h"

#include "graphics/Graphics.h"
#include "world/Block.h"

bool System::running;
FileLogger System::errorLog;

bool System::Init() {
	errorLog.SetPath("error.txt");
	errorLog.EnableAppend();

	srand((unsigned int)time(nullptr));

	Block::Init();

	Config::LoadConfigFile();

	if (!glfwInit()) {
		return false;
	}

	if (!Window::Init()) {
		return false;
	}

	if (!Graphics::Init()) {
		return false;
	}

	//Window::Maximize();

	Input::Init();

	//Window::Display();

	if (!Res::Init()) {
		return false;
	}

	if (!Game::Init()) {
		return false;
	}

	//Window::sfWindow.ShowMouseCursor(false);

	Graphics::InitGraphics();

	return true;
}

void System::Start() {
	if (running) return;
	running = true;

	Game::Start();
	Window::SetCallbacks();

	while(running) {
		Update();
	}
}

void System::Update() {
	Game::Update();
	Game::Render();
	Window::Display();
}

void System::Shutdown() {
	Game::Shutdown();
	Graphics::Shutdown();
	Res::Shutdown();
	glfwTerminate();
}

void System::Stop() {
	running = false;
}