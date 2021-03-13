/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: graphics.h
Purpose: define main graphic utils
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/
#pragma once

#include "math_utils.h"
#include "color.h"

#include <glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_video.h>
#include <string>
#include <SDL_ttf.h>	//font rendering!
#include <vector>
#include <map>
#include <imgui.h>
#include <imgui_impl_sdl_gl3.h>	// so imgui and sdl can be friends 4 eva...

#define MAX_LINES 1600	// MAX_LINES / 2 == LINE_COUNT
#define LINE_VBO_SIZE 2	//positions + colors
#define WINDOW_W 1080
#define WINDOW_H 900
#define SHADOW_MAP_W 1024
#define SHADOW_MAP_H 1024
#define MAX_G_BUFFER_CHANNELS 4	// position, normals, albedo, depth


enum Texture : int { no_texture = -1, t_text = 0, t_depth_map, t_light_acc, t_white, t_black, t_pattern, t_skybox, texture_count };
enum Shader : int {		no_shader = -1, 
	/*3D, 2D basic*/	sh_basic = 0, sh_text,
	/*DEBUG*/			sh_depth_color, sh_red_color, sh_alpha_color,
	/*Lines*/			sh_line2D, sh_line3D,
	/*Forward shading*/	sh_light, sh_skybox,
	/*Deferred shading*/sh_geometry_pass, sh_lighting_pass, sh_ssao, sh_hbao, sh_decal,
	/*Post-procesing*/	sh_sobel, sh_antialiasing, sh_hdr, sh_blur, sh_bloom, sh_gaussian_blur, sh_bilateral_blur,
	/*others(Deferred)*/sh_terrain_perlin, sh_terrain_map, sh_terrain_cubemap,
	shader_count };


struct Graphics
{
	//load data and initialize systems
	void init();
	//free data and systems (openGL, SDL)
	void free();

	mat3 window_to_ndc() const;
	mat3 ndc_to_window() const;
	bool toggle_fullscreen();
	void resize_window(int size_x, int size_y);
	//make text render requests per frame
	void render_text(const std::string &text, f32 ndc_x, f32 ndc_y, f32 fontSize, Color color = { 0xFFFFFFFF });
	//render a circle made with lines
	void draw_circle(const vec2& ndc_center, f32 radius, u32 segments, Color color = { 0xFFFFFFFF });
	//make line render requests per frame
	void draw_line(const vec2& ndc_start, const vec2& ndc_end, Color color_start = { 0xFFFFFFFF }, Color color_end = { 0xFFFFFFFF });
	//render all line requests
	void draw_lines();
	void draw_lines(const vec2* points, const Color* colors, const u32 point_count, GLuint draw_mode = GL_LINES);
	void clear_lines();
	//shader controll
	void delete_shaders();
	void compile_shaders();


	//window stuff
	SDL_Window* window = nullptr;
	SDL_Surface* window_surface = nullptr;
	Uint32 window_id = 0;
	SDL_GLContext gl_context_id = nullptr;
	bool is_fullscreen = false;

	//shader stuff
	GLuint shader_program[Shader::shader_count] = { 0 };

	//textures
	GLuint tex[Texture::texture_count] = { 0 };
	std::map<std::string, GLuint> texture_map;

	// SHADOW FrameBuffer & stuff
	GLuint depth_FBO;

private:
	//int uniform_position_starts, uniform_position_ends, uniform_color_starts, uniform_color_ends;
	GLuint line_vao;
	GLuint line_vbo[LINE_VBO_SIZE]; //positions, colors
									// lines are rendered and then cleared
	std::vector<vec2> line_positions_to_render;
	std::vector<Color> line_colors_to_render;

	// font texture
	TTF_Font *font = nullptr;
	//uniform locations	(for rendered text shader)
	int uniform_mtx_loc = -1, uniform_color_loc = -1, uniform_tex_loc = -1;
	//model vao and buffers (simple quad, no indices)
	GLuint text_vbo[2];	// positions, uvs
	GLuint text_vao = 0;

public:
	inline int getUniformMtxLoc() const { return uniform_mtx_loc; }
	inline int getUniformColorLoc() const { return uniform_color_loc; }
	inline int getUniformTexLoc() const { return uniform_tex_loc; }
	inline GLuint getQuadVao() const { return text_vao; }
};
extern Graphics graphics;
// outputs error message, returns number of errors
unsigned pop_gl_errors(const char* fun_name);
// LOAD TEXTURES
void load_texture(GLuint *texture_id, void const* tex_data, ivec2 tex_size, u32 format_pixels, bool invert_y = false, bool generate_mipmaps = false);	// caller handler data cleanup
void load_texture(GLuint *texture_id, SDL_Surface const* sdl_texture, bool invert_y = false, bool generate_mipmaps = false);						// caller handles SDL_Surface cleanup
void load_texture(GLuint *texture_id, const char* filepath, bool invert_y = false, bool generate_mipmaps = false);
void load_depth_texture(GLuint *texture_id, const char* filepath, bool invert_y = false, bool generate_mipmaps = false);
void load_cubemap(GLuint *texture_id, const char* filepaths[6], bool generate_mipmaps = false);
// color checker pattern texture
void load_pattern_texture(GLuint * texture_id, const u32 pixel_size = 6);
// Single pixel white texture
void load_color_texture(GLuint * texture_id, Color color);
// compute random noise texture
void load_noise_texture(GLuint *texture_id, size_t size, bool generate_mipmaps = false);
void load_noise_texture(GLuint *texture_id, size_t size, vec2 offset, f32 frequency, f32 lacunarity, f32 persistence, u8 oct, bool generate_mipmaps = false);
// create buffers for shadow shading
void create_depth_buffers(GLuint &depth_fbo, GLuint &depth_texture, u32 texture_width = SHADOW_MAP_W, u32 texture_height = SHADOW_MAP_H);
// create framebuffers for environment mapping
void create_cubemap_framebuffers(GLuint(&framebuffers)[6], GLuint &cubemap, u32 resolution = 1024);
// G-Buffer (geometry pass) for deferred shading
void create_g_buffer(GLuint &g_buffer, GLuint(&g_buffer_textures)[MAX_G_BUFFER_CHANNELS], u32 tex_width = WINDOW_W, u32 tex_height = WINDOW_H);
// buffer for light accumulation
void create_framebuffer(GLuint &framebuffer, GLuint &out_texture, GLuint *depth_buffer = nullptr, u32 texture_width = WINDOW_W, u32 texture_height = WINDOW_H);
// HDR: Hight Dynamic Range (texture with HALF_FLOAT channels)
void create_hdr_framebuffer(GLuint &framebuffer, GLuint &out_texture, GLuint *depth_buffer = nullptr, u32 texture_width = WINDOW_W, u32 texture_height = WINDOW_H);
// decals frambuffer
void create_decal_framebuffer(GLuint &framebuffer, GLuint &out_diffuse_tex, GLuint &out_normal_tex, GLuint* depth_buffer = nullptr, u32 texture_width = WINDOW_W, u32 texture_height = WINDOW_H);


