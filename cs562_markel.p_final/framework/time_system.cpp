/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: time_system.cpp
Purpose: count application's time
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/

#include "time_system.h"
#include <Windows.h>
#include <time.h>
#include <thread>

//public set
u32 fps = 60;					// desired frames per second
f32 time_scale = 1.f;			// time scale of the game
bool is_controlled_frame_rate = true;	// cap frame rate
//only get on public
f32 dt = 0.f;					// frame_time * time_scale
f64 frame_time = 0.0;			// times took the last frame
f64 frame_rate = 1.0 / fps;		// desired frame rate
f64 frame_start_time = 0.0;		// time at the start of the frame
f64 time_count = 0.0;			// time since launch
u64 frame_count = 0;			// frames since launch


void start_frame()
{
	frame_start_time = get_time();
}
void end_frame()
{
	const f64 controlled_frame_time = 1.0 / fps;
	frame_time = get_time() - frame_start_time;
	if (is_controlled_frame_rate)
	{
	  //sleep thread remaining time
		//const std::chrono::microseconds remaining_seconds { 1000000 * (int)(controlled_frame_time - frame_time) };
		//std::this_thread::sleep_for(remaining_seconds);

		while (frame_time < controlled_frame_time)
			frame_time = get_time() - frame_start_time;
	}
	frame_count++;
	frame_rate = 1.0 / frame_time;
	time_count += frame_time;
	dt = (f32)frame_time * time_scale;
}
f64 get_time()
{
	__int64 f, t;
	double r;

	// get the performance frequency
	QueryPerformanceFrequency((LARGE_INTEGER*)(&f));

	// get the performance counter
	QueryPerformanceCounter((LARGE_INTEGER*)(&t));

	// clock / frequency = seconds
	r = (double)t / (double)f;

	// return
	return r;
}
