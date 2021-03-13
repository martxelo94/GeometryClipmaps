/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: time_system.h
Purpose: define frame control utils
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/
#pragma once

#include "math_utils.h"

//public set
extern u32 fps;					// desired frames per second
extern f32 time_scale;			// time scale of the game
extern bool is_controlled_frame_rate;	// cap frame rate
	
//only get on public
extern f32 dt;					// frame_time * time_scale
extern f64 frame_time;			// times took the last frame
extern f64 frame_rate;		// desired frame rate
extern f64 frame_start_time ;		// time at the start of the frame
extern f64 time_count;			// time since launch
extern u64 frame_count;			// frames since launch

void start_frame();
void end_frame();
//get global time in seconds
f64 get_time();

struct Timer
{
	f32 t = 0.f;
	f32 max_t = 0.f;
	Timer() = default;
	Timer(f32 max_t) : max_t(max_t) {}
	__forceinline void update() { t += dt; }
	__forceinline void reset() { t = 0.f; }
	//true if current time > max time
	__forceinline bool operator()() const { return t > max_t; }
};