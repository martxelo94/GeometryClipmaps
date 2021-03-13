/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: main.cpp
Purpose: implement main loop and application creation/destruction
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/


#define SDL_MAIN_HANDLED

//#include "math_utils.h"
#include "graphics.h"
#include "scene.h"
#include "game_states.h"
#include "time_system.h"
#include "input.h"
//#include "object.h"
//#include "physics.h"
//#include "sound.h"

#include <stdio.h>
#include <SDL_events.h>
#include <SDL_assert.h>
#include <iostream>
#include <string>

#define INITIAL_SCENE TerrainDemo


//global scene declared in scene.h
Scene* scene = nullptr;
int main(int, const char**)
{

	//initialize SDL & openGL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);
	SDL_assert(SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == (SDL_INIT_VIDEO | SDL_INIT_EVENTS));
	//Use OpenGl 4.2 core version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	graphics.init();
	keyboard.init();
	mouse.init();

	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "This device supports OpenGL version " << version << std::endl;

	glClearColor(0, 0, 0, 1);
	//glClearColor(0.2f, 0.f, 0.2f, 1.f);

	//create STARTING scene
	scene = new INITIAL_SCENE();

	// FPS CONTROL
	is_controlled_frame_rate = true;

	//game loop
	while (scene)
	{

		start_frame();

		// INPUT
		{
			SDL_Event event;

			keyboard.update();
			mouse.update();


			while (SDL_PollEvent(&event))
			{
				keyboard.handle(event);
				mouse.handle(event);
				// application inputs
				switch (event.type)
				{
				case SDL_KEYDOWN:
				{
					//get virtual key codes
					switch (event.key.keysym.sym)
					{
					case SDLK_F1:
						break;
					case SDLK_F9:
						//graphics.toggle_fullscreen();
						is_controlled_frame_rate = !is_controlled_frame_rate;
						break;
					case SDLK_m:
						break;
					}

				}
				break;
				//window events (resize, click over, exit X button...)
				case SDL_WINDOWEVENT:
				{
					SDL_assert(graphics.window_id == event.window.windowID);
					switch (event.window.event)
					{
					case SDL_WINDOWEVENT_CLOSE:
						delete scene;
						scene = nullptr;
						break;
					case SDL_WINDOWEVENT_MOVED:
						SDL_SetWindowPosition(graphics.window, event.window.data1, event.window.data2);
						break;
					case SDL_WINDOWEVENT_RESIZED:
						graphics.resize_window(event.window.data1, event.window.data2);
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST:
						//SDL_MinimizeWindow(graphics.window);
						break;
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						//SDL_MaximizeWindow(graphics.window);
						break;
					default:
						break;
					}
					break;

				}
				default:
					break;
				}
				//update input, it might change main scene
				//be aware! scene pointer can be null after this
				if (scene)
					scene = scene->input(event);

				//process ImGui input
				ImGui_ImplSdlGL3_ProcessEvent(&event);
			}

		}

		// UPDATE
		{
			ImGui_ImplSdlGL3_NewFrame(graphics.window);
			//bool so_true = true;
			//ImGui::ShowDemoWindow(&so_true);

			int flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			bool gui_open = true;
			ImGui::Begin("Menu", &gui_open, flags);
			ImGui::SetWindowPos({ 0, 0 });
			//ImGui::SetWindowSize({ ImGui::GetWindowWidth(), 0.1 });
			if (ImGui::Button("Terrain")) {
				delete scene;
				scene = new TerrainDemo();
			}
#if 0
			if (ImGui::BeginMenu("Demos")) {
				if (ImGui::MenuItem("Planet")) {
					delete scene;
					scene = new DemoPlanet();
				}
				if (ImGui::MenuItem("SphereTriangleCollision")) {
					delete scene;
					scene = new DemoSphereTriangleCollision();
				}
				if (ImGui::MenuItem("Perlin")) {
					delete scene;
					scene = new DemoPerlin();
				}
				if (ImGui::MenuItem("Cardioid")) {
					delete scene;
					scene = new DemoCardioid();
				}
				if (ImGui::MenuItem("Parametric")) {
					delete scene;
					scene = new DemoParametric();
				}
				ImGui::EndMenu();
			}
#endif
			ImGui::End();
			
			//update scene logic
			if (scene)
				scene = scene->update();
		}

		// RENDER
		{
			//common render stuff
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//call scene->render here
			{
				if (scene)
					scene->render();
			}
			{
				//render lines instanced
				graphics.draw_lines();
				graphics.clear_lines();
			}

			//print frame rate on screen
			{
				int flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				bool gui_open = true;
				ImGui::Begin("Frame Control", &gui_open, flags);
				std::string tmp = "Time: " + std::to_string(frame_time);
				ImGui::Text(tmp.c_str());
				tmp = "Rate: " + std::to_string((int)frame_rate);
				ImGui::Text(tmp.c_str());
				ImGui::End();
			}
			//graphics.render_text("FPS: " + std::to_string((int)frame_rate), 0.9f, 0.9f, 0.001f);

			// update ImGUI
			ImGui::Render();
			ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());

			{
				//end render
				SDL_GL_SwapWindow(graphics.window);
			}
		}

		// FRAME CONTROLL
		{
			end_frame();
		}
	}

	// free stuff
	graphics.free();
	SDL_Quit();


	return 0;
}


