/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: Scene.h
Purpose: base Game State interface
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/
#pragma once

#include <SDL_events.h>

/*
	Abstract class for Game State management
	Scenes like Game, Pause and MainMenu use this interface
	update and input methods normally return this Scene, but can return new Scenes
*/

struct Scene
{
	virtual Scene* update() = 0;
	virtual Scene* input(const SDL_Event&) = 0;
	virtual void render() const = 0;
	virtual ~Scene() {}
};

// current scene
extern Scene* scene;
