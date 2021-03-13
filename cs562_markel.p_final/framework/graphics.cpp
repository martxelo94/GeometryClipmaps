/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: graphics.cpp
Purpose: implement main graphics utils
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/
#include "graphics.h"
#include "file_system.h"
#include "mesh.h"
#include "noise.h"
#include <assert.h>
#include <Windows.h>
#include <iostream>
#include <vector>


Graphics graphics;

#pragma region UTILS
unsigned pop_gl_errors(const char* fun_name)
{
	unsigned error_count = 0;
	while (GLuint error = glGetError())
	{
		error_count++;
		//MessageBox(NULL, std::to_string(error).c_str(), "GL error code:", MB_ICONERROR | MB_OK);
		std::cout << "GL error code: " << std::to_string(error) << " \"" << (const char*)glewGetErrorString(error) << "\" " << " at \"" << fun_name << "\""<< std::endl;
	}
	return error_count;
}
namespace
{

	void check_gl_program_link_errors(GLuint program_handle)
	{
		GLint status = GL_FALSE;
		glGetProgramiv(program_handle, GL_LINK_STATUS, &status);
		if (status != GL_TRUE) {
			int infoLogLength = 0, maxLength = 0;
			glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &maxLength);
			std::string infoLog(maxLength, 0);
			glGetProgramInfoLog(program_handle, maxLength, &infoLogLength, &infoLog[0]);
			//pop error window
			MessageBox(NULL, infoLog.c_str(), "Program compile error", MB_ICONERROR | MB_OK);
		}
	}

	void check_gl_shader_compile_errors(GLuint shader_handle)
	{
		GLint status = GL_FALSE;
		glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &status);
		if (status != GL_TRUE) {
			int infoLogLength = 0, maxLength = 0;
			glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &maxLength);
			std::string infoLog(maxLength, 0);
			glGetShaderInfoLog(shader_handle, maxLength, &infoLogLength, &infoLog[0]);
			//pop error window
			MessageBox(NULL, infoLog.c_str(), "Shader compile error", MB_ICONERROR | MB_OK);
		}
	}
	GLuint compile_shader(GLuint shader_type, const std::string &source)
	{
		GLuint shader_handle = glCreateShader(shader_type);
		const char* c_source = source.c_str();
		glShaderSource(shader_handle, 1, &c_source, NULL);
		glCompileShader(shader_handle);
		check_gl_shader_compile_errors(shader_handle);
		return shader_handle;
	}

	// knows the shader type by the file extension
	GLuint compile_shader(const char* filepath)
	{
		std::string extension = get_extension(filepath);
		GLuint shader_type = (GLuint)-1;
		if (extension == ".vert")
			shader_type = GL_VERTEX_SHADER;
		else if (extension == ".frag")
			shader_type = GL_FRAGMENT_SHADER;
		else if (extension == ".geo")
			shader_type = GL_GEOMETRY_SHADER;
		else if (extension == ".tcs")
			shader_type = GL_TESS_CONTROL_SHADER;
		else if (extension == ".tes")
			shader_type = GL_TESS_EVALUATION_SHADER;
		else {
			std::cerr << "\"" << filepath << "\" has not a valid shader extension.\n";
			return 0;	//no shader handle
		}
		//open file
		std::string shader_source = read_to_string(filepath);
		if (shader_source.empty()) {
			std::cerr << "Shader source is empty.\n";
			return 0;
		}
		return compile_shader(shader_type, shader_source);
	}
	void create_quad_mesh_for_rendered_text(GLuint *vao, GLuint vbo[2])
	{
		const size_t vertexCount = 4;
		// defined ccw plane on XY axis
		float positions[vertexCount * 3] = {
			1.f, -1.f, 0.f,		// 0
			1.f, 1.f, 0.f,		// 1
			-1.f, -1.f, 0.f,	// 2
			-1.f, 1.f, 0.f		// 3
		};
		float uvs[vertexCount * 2] = {
			1.f, 0.f,			// 0
			1.f, 1.f,			// 1
			0.f, 0.f,			// 2
			0.f, 1.f			// 3
		};
		glGenVertexArrays(1, vao);
		glBindVertexArray(*vao);
		//generate buffers
		glGenBuffers(2, &vbo[0]);
		//load positions buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(positions), &positions[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//load uvs buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), &uvs[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		//Unbind buffers
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		pop_gl_errors(__FUNCTION__);
	}
	void create_line_mesh(GLuint *vao, GLuint vbo[LINE_VBO_SIZE], vec3 const* positions, Color const* colors, size_t vertex_count)
	{
		glGenVertexArrays(1, vao);
		glBindVertexArray(*vao);
		//generate buffers
		glGenBuffers(LINE_VBO_SIZE, vbo);

		//NO INSTANCING
		//load positions buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vec3), positions, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//create buffers for color and matrices but dont fill
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		//initialize with NULL buffer
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Color), colors, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

		//Unbind buffers
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		pop_gl_errors(__FUNCTION__);
	}
	void create_line_mesh_dynamic(GLuint *vao, GLuint vbo[LINE_VBO_SIZE])
	{
		glGenVertexArrays(1, vao);
		glBindVertexArray(*vao);
		//generate buffers
		glGenBuffers(LINE_VBO_SIZE, vbo);

		//NO INSTANCING
		//load positions buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, MAX_LINES * sizeof(vec2) * 2, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		//create buffers for color and matrices but dont fill
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		//initialize with NULL buffer
		glBufferData(GL_ARRAY_BUFFER, MAX_LINES * sizeof(Color) * 2, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

		//Unbind buffers
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		pop_gl_errors(__FUNCTION__);
	}
	void invert_image_y(u8 *image_bytes, u32 width, u32 height, u32 bytes_per_pixel)
	{
		u32 half_h = height / 2;
		u32 row_bytes = width * bytes_per_pixel;
		u8 *tmp_row = new u8[row_bytes];

		for (u32 i = 0; i < half_h; i++) {
			u32 idx_top = row_bytes * i;
			u32 idx_bot = row_bytes * (height - i - 1);
			memcpy(tmp_row, image_bytes + idx_top, row_bytes);
			memcpy(image_bytes + idx_top, image_bytes + idx_bot, row_bytes);
			memcpy(image_bytes + idx_bot, tmp_row, row_bytes);
		}

		delete tmp_row;
	}
}
#pragma endregion

#pragma region TEXTURES


void load_texture(GLuint *texture_id, void const* tex_data, ivec2 tex_size, u32 format_pixels, bool invert_y, bool generate_mipmaps)
{
	assert(texture_id);
	glDeleteTextures(1, texture_id);

	// invert image vertically for OpenGL (hackly removing constness...)
	if(invert_y)
		invert_image_y(reinterpret_cast<u8*>(const_cast<void*>(tex_data)), tex_size.x, tex_size.y, format_pixels);

	//std::vector<u8> pixels(reinterpret_cast<u8*>(const_cast<void*>(tex_data)), reinterpret_cast<u8*>(const_cast<void*>(tex_data)) + tex_size.x * tex_size.y * format_pixels);

	const GLuint format= format_pixels == 4 ? GL_RGBA : format_pixels == 3? GL_RGB : GL_R;
	const GLuint internalFormat = format_pixels == 4 ? GL_RGBA8 : format_pixels == 3 ? GL_RGB8 : GL_R8;
	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_2D, *texture_id);
	
	if (generate_mipmaps) {
		int levels = log2(max(tex_size.x, tex_size.y));
		glTexStorage2D(GL_TEXTURE_2D, levels, internalFormat, tex_size.x, tex_size.y);
		pop_gl_errors("AKI LOKO 1");
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_size.x, tex_size.y, format, GL_UNSIGNED_BYTE, tex_data);
		pop_gl_errors("AKI LOKO 2");
		glEnable(GL_TEXTURE_2D);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels);
	}
	else
		glTexImage2D(GL_TEXTURE_2D, 0, format, tex_size.x, tex_size.y, 0, format, GL_UNSIGNED_BYTE, tex_data);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generate_mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generate_mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	glBindTexture(GL_TEXTURE_2D, 0);

	pop_gl_errors(__FUNCTION__);
}
//caller handles SDL_Surface cleanup
void load_texture(GLuint *texture_id, SDL_Surface const *sdl_texture, bool invert_y, bool generate_mipmaps)
{
	SDL_assert(sdl_texture);
	load_texture(texture_id, sdl_texture->pixels, { sdl_texture->w, sdl_texture->h }, sdl_texture->format->BytesPerPixel, invert_y, generate_mipmaps);
}
void load_texture(GLuint *texture_id, const char* filepath, bool invert_y, bool generate_mipmaps)
{
	SDL_Surface *sdl_tex = IMG_Load(filepath);
	if (sdl_tex == nullptr) {
		std::cout << "Failed loading \"" << filepath << "\"" << std::endl;
		return;
	}
	load_texture(texture_id, sdl_tex, invert_y, generate_mipmaps);
	SDL_FreeSurface(sdl_tex);
}
void load_depth_texture(GLuint *texture_id, const char* filepath, bool invert_y, bool generate_mipmaps)
{
	SDL_Surface *sdl_tex = IMG_Load(filepath);
	if (sdl_tex == nullptr) {
		std::cout << "Failed loading \"" << filepath << "\"" << std::endl;
		return;
	}
	assert(texture_id);
	glDeleteTextures(1, texture_id);

	// invert image vertically for OpenGL (hackly removing constness...)
	//if (invert_y)
	//	invert_image_y(reinterpret_cast<u8*>(const_cast<void*>(sdl_tex->pixels)), sdl_tex->w, sdl_tex->h, sdl_tex->format->BytesPerPixel);

	std::vector<f32> depths(sdl_tex->w * sdl_tex->h, 0.f);
	for (int i = 0; i < depths.size(); i++)
		depths[i] = (f32)reinterpret_cast<u8*>(sdl_tex->pixels)[i * sdl_tex->format->BytesPerPixel] / 255;

	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_2D, *texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, sdl_tex->w, sdl_tex->h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depths.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generate_mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (generate_mipmaps) {
		glGenerateMipmap(GL_TEXTURE_2D);
		f32 w = sdl_tex->w / 2;
		f32 h = sdl_tex->h / 2;
		int levels = log2(max(sdl_tex->w, sdl_tex->h)) + 1;
		for (int i = 1; i < levels; ++i, w /= 2, h /= 2) {
			glTexImage2D(GL_TEXTURE_2D, i, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depths.data());
		}
	}
	SDL_FreeSurface(sdl_tex);

	pop_gl_errors(__FUNCTION__);
}
void load_cubemap(GLuint *texture_id, const char* filepaths[6], bool generate_mipmaps)
{
	assert(texture_id);
	glDeleteTextures(1, texture_id);

	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, *texture_id);

	for (int i = 0; i < 6; i++) {
		SDL_Surface *sdl_tex = IMG_Load(filepaths[i]);
		if (sdl_tex) {
			const unsigned format_offset = sdl_tex->format->BytesPerPixel == 4 ? 1 : 0;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB + format_offset, sdl_tex->w, sdl_tex->h, 0, GL_RGB + format_offset, GL_UNSIGNED_BYTE, sdl_tex->pixels
			);
			SDL_FreeSurface(sdl_tex);
		}
		else {
			// failed loading map, erase texture and return
			glDeleteTextures(1, texture_id);
			std::cerr << "Failed loading " << filepaths[i] << " as cubemap." << std::endl;
			return;
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, generate_mipmaps? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	if (generate_mipmaps)	// DOES IT WORK? DUNNO!
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	pop_gl_errors(__FUNCTION__);
}
void load_pattern_texture(GLuint * texture_id, const u32 pixel_size)
{
	assert(texture_id);
	glDeleteTextures(1, texture_id);

	const Color colors[] = {
		0x0000ffff /*blue*/, 0x00ffffff /*cyan*/, 0x00ff00ff /*green*/,
		0xffff00ff/*yellow*/, 0xff0000ff/*red*/, 0xff69b4ff /*pink*/
	};
	const u32 colorCount = sizeof(colors) / sizeof(Color);
	assert(pixel_size >= colorCount);
	const u32 colorByteSize = 4;	/* RGBA -> MUST BE 4-byte aligned */
	const u32 widthBytes = pixel_size * colorByteSize;
	const u32 colorQuadByteWidth = widthBytes / colorCount;
	const u32 colorQuadByteHeight = pixel_size / colorCount;
	//allocate mem
	std::vector<u8> data(pixel_size * widthBytes, 0);
	for (u32 y = 0; y < pixel_size; ++y)
	{
		for (u32 colorChunk = 0; colorChunk < colorCount; ++colorChunk)
		{
			const u32 idx = (pixel_size - y - 1) * widthBytes + colorChunk * colorQuadByteWidth;
			const u32 color_idx = (colorChunk + (y / colorQuadByteHeight)) % colorCount;
			const Color& c = colors[color_idx];
			for (u32 currPixIdx = 0; currPixIdx < colorQuadByteWidth; currPixIdx += colorByteSize)
			{
				std::memcpy(data.data() + idx + currPixIdx, c.v, colorByteSize * sizeof(u8));
			}
		}
	}
	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_2D, *texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pixel_size, pixel_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	pop_gl_errors(__FUNCTION__);
}
// Single pixel white texture
void load_color_texture(GLuint * texture_id, Color color) {
	assert(texture_id);
	glDeleteTextures(1, texture_id);
	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_2D, *texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color.v[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	pop_gl_errors(__FUNCTION__);
}
void load_noise_texture(GLuint *texture_id, size_t size,bool generate_mipmaps)
{
	assert(texture_id);

	std::vector<f32> pixels(size * size * 4, 0);
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			vec2 xy = glm::circularRand(1.0f);
			f32 z = glm::linearRand(0.f, 1.f);
			f32 w = glm::linearRand(0.f, 1.f);

			int idx = 4 * (x + y * size);
			pixels[idx] = xy.x;
			pixels[idx + 1] = xy.y;
			pixels[idx + 2] = z;
			pixels[idx + 3] = w;
		}
	}

	glDeleteTextures(1, texture_id);
	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_2D, *texture_id);

	if (generate_mipmaps) {
		int levels = log2(size);
		glTexStorage2D(GL_TEXTURE_2D, levels, GL_RGBA16F, size, size);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size, size, GL_RGBA16F, GL_FLOAT, pixels.data());
		glEnable(GL_TEXTURE_2D);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels);
	}
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, size, size, 0, GL_RGBA, GL_FLOAT, pixels.data());


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	const f32 borderColor[] = { 1.f, 1.f, 1.f, 1.f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	pop_gl_errors(__FUNCTION__);
}
void load_noise_texture(GLuint *texture_id, size_t size, vec2 offset, f32 frequency, f32 lacunarity, f32 persistence, u8 oct, bool generate_mipmaps)
{
	assert(texture_id);

	std::vector<f32> pixels = Noise::create_depth_texture(Noise::perlin2D, size, offset, frequency, lacunarity, persistence, oct);

	glDeleteTextures(1, texture_id);
	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_2D, *texture_id);

	if (generate_mipmaps) {
		int levels = log2(size);
		glTexStorage2D(GL_TEXTURE_2D, levels, GL_DEPTH_COMPONENT32F, size, size);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size, size, GL_DEPTH_COMPONENT32F, GL_FLOAT, pixels.data());
		glEnable(GL_TEXTURE_2D);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels);
	}
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, pixels.data());


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	const f32 borderColor[] = { 1.f, 1.f, 1.f, 1.f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	pop_gl_errors(__FUNCTION__);
}
#pragma endregion

#pragma region BUFFERS
// create buffers for shadow shading
void create_depth_buffers(GLuint &depth_fbo, GLuint &depth_texture, u32 texture_width, u32 texture_height)
{
	// delete posible buffers first
	glDeleteFramebuffers(1, &depth_fbo);
	glDeleteTextures(1, &depth_texture);

	// SHADOW MAP INITIALIZATION
	glGenFramebuffers(1, &depth_fbo);	// init frame buffer to store depth map
										// generate texture
	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texture_width, texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const f32 borderColor[] = { 1.f, 1.f, 1.f, 1.f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// tell this buffer we are not using colors
	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);	// unbind depth buffer

}
// create framebuffers for environment mapping
void create_cubemap_framebuffers(GLuint(&framebuffers)[6], GLuint &cubemap, u32 resolution)
{
	// delete posible buffers first
	glDeleteFramebuffers(6, framebuffers);
	glDeleteTextures(1, &cubemap);

	// create 6 frame buffers
	glGenFramebuffers(6, framebuffers);
	glDeleteTextures(1, &cubemap);	// delete texture por si acaso
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &cubemap);	// use last cubemap to render on it
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	//create 2D images
	for (int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, resolution, resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// attach 6 buffers

	for (int i = 0; i < 6; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, 0);
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);
		GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			std::cerr << "Cubemap's framebuffer " << i << " failed to attach!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	pop_gl_errors(__FUNCTION__);

}
// create a geometry buffer for deferred shading technique
void create_g_buffer(GLuint &g_buffer, GLuint (&g_buffer_textures)[MAX_G_BUFFER_CHANNELS], u32 tex_width, u32 tex_height)
{
	// delete posible buffers first
	glDeleteFramebuffers(1, &g_buffer);
	glDeleteTextures(MAX_G_BUFFER_CHANNELS, g_buffer_textures);

	glGenFramebuffers(1, &g_buffer);
	glGenTextures(MAX_G_BUFFER_CHANNELS, g_buffer_textures);
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

	// position color
	glBindTexture(GL_TEXTURE_2D, g_buffer_textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, tex_width, tex_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_buffer_textures[0], 0);
	pop_gl_errors("position buffer");

	// normal + shininess color
	glBindTexture(GL_TEXTURE_2D, g_buffer_textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, tex_width, tex_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_buffer_textures[1], 0);
	pop_gl_errors("normal buffer");

	// albedo (diffuse) + specular (shininess)
	glBindTexture(GL_TEXTURE_2D, g_buffer_textures[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_buffer_textures[2], 0);
	pop_gl_errors("albedo buffer");

	// depth
	glBindTexture(GL_TEXTURE_2D, g_buffer_textures[3]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, tex_width, tex_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width, tex_height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const f32 borderColor[] = { 1.f, 1.f, 1.f, 1.f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// tell this buffer we are not using colors
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_buffer_textures[3], 0);
	pop_gl_errors("depth buffer");
	
	// tell OpenGL which color attachments we'll use for rendering
	GLuint attachments[MAX_G_BUFFER_CHANNELS - 1] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(sizeof(attachments) / sizeof(GLuint), attachments);


	//finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Geometry Buffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	pop_gl_errors(__FUNCTION__);
}
void create_framebuffer(GLuint &framebuffer, GLuint &out_texture, GLuint *depth_buffer, u32 texture_width, u32 texture_height)
{
	// delete posible buffers first
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &out_texture);
	
	glCreateFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glCreateTextures(GL_TEXTURE_2D, 1, &out_texture);

	// light color channels (RGB)
	glBindTexture(GL_TEXTURE_2D, out_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out_texture, 0);

	// tell OpenGL which color attachments we'll use for rendering
	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	// depth buffer
	if (depth_buffer) {
		if (*depth_buffer == 0) {
#if 0
			glDeleteRenderbuffers(1, depth_buffer);
			glGenRenderbuffers(1, depth_buffer);
			glBindRenderbuffer(GL_RENDERBUFFER, *depth_buffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texture_width, texture_height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *depth_buffer);
#else	// with texture
			glGenTextures(1, depth_buffer);
			glBindTexture(GL_TEXTURE_2D, *depth_buffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texture_width, texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width, tex_height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			const f32 borderColor[] = { 1.f, 1.f, 1.f, 1.f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}
#endif
		// tell this buffer we are not using colors
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth_buffer, 0);
}

	//finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	pop_gl_errors(__FUNCTION__);

}
// buffer for light accumulation
void create_hdr_framebuffer(GLuint &framebuffer, GLuint &out_texture, GLuint *depth_buffer, u32 texture_width , u32 texture_height)
{
	// delete posible buffers first
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &out_texture);

	glCreateFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glCreateTextures(GL_TEXTURE_2D, 1, &out_texture);

	// color channels (RGB)
	glBindTexture(GL_TEXTURE_2D, out_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, texture_width, texture_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out_texture, 0);

	// tell OpenGL which color attachments we'll use for rendering
	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	// depth buffer
	if (depth_buffer) {
		if (*depth_buffer == 0) {
#if 0
			glDeleteRenderbuffers(1, depth_buffer);
			glGenRenderbuffers(1, depth_buffer);
			glBindRenderbuffer(GL_RENDERBUFFER, *depth_buffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texture_width, texture_height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *depth_buffer);
#else	// with texture
			glGenTextures(1, depth_buffer);
			glBindTexture(GL_TEXTURE_2D, *depth_buffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texture_width, texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width, tex_height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			const f32 borderColor[] = { 1.f, 1.f, 1.f, 1.f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}
#endif
	// tell this buffer we are not using colors
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth_buffer, 0);
}

	//finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	pop_gl_errors(__FUNCTION__);
}
// decals frambuffer
void create_decal_framebuffer(GLuint &framebuffer, GLuint &out_diffuse_tex, GLuint &out_normal_tex, GLuint* depth_buffer, u32 texture_width, u32 texture_height)
{
	// delete posible buffers first
	glDeleteFramebuffers(1, &framebuffer);
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// diffuse texture
	if (out_diffuse_tex == 0) {
		glGenTextures(1, &out_diffuse_tex);
		glBindTexture(GL_TEXTURE_2D, out_diffuse_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out_diffuse_tex, 0);
	pop_gl_errors("diffuse buffer");

	// normal texture
	if (out_normal_tex == 0) {
		glGenTextures(1, &out_normal_tex);
		glBindTexture(GL_TEXTURE_2D, out_normal_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, texture_width, texture_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, out_normal_tex, 0);
	pop_gl_errors("normal buffer");
	// depth buffer
	if (depth_buffer) {
		if (*depth_buffer == 0) {
#if 0
		glDeleteRenderbuffers(1, depth_buffer);
		glGenRenderbuffers(1, depth_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, *depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texture_width, texture_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *depth_buffer);
#else	// with texture
		glGenTextures(1, depth_buffer);
		glBindTexture(GL_TEXTURE_2D, *depth_buffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texture_width, texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width, tex_height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		const f32 borderColor[] = { 1.f, 1.f, 1.f, 1.f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		}
#endif
		// tell this buffer we are not using colors
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth_buffer, 0);
	}

	// tell OpenGL which color attachments we'll use for rendering
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(sizeof(attachments) / sizeof(GLuint), attachments);


	//finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Decals Buffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	pop_gl_errors(__FUNCTION__);

}
#pragma endregion


// these function are globally visible and might modify data from Graphics namespace

void Graphics::init()
{
	//initialize window & gl_context
	window = SDL_CreateWindow("CS562 - MARTXELO", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_assert(window);
	window_surface = SDL_GetWindowSurface(window);
	SDL_assert(window_surface);
	window_id = SDL_GetWindowID(window);
	SDL_assert(window_id);
	gl_context_id = SDL_GL_CreateContext(window);
	SDL_assert(gl_context_id);

	//initialize openGL system
	glewExperimental = GL_TRUE;
	GLenum error = glewInit();
	if (error != GLEW_OK)
		MessageBox(NULL, (const LPCSTR)glewGetErrorString(error), "Glew error", MB_ICONERROR | MB_OK);
	//rid off the anoying 1280 error
	error = glGetError();
	if(error == 1280)
		std::cout << "Poped the anoying 1280 error code after initializing glew" << std::endl;

	const int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
	// initialize SDL image loading
	if (IMG_Init(img_flags) != img_flags)
		MessageBox(NULL, SDL_GetError(), "SDL error", MB_ICONERROR | MB_OK);

	//openGL alpha blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);	// Does not work with G-BUFFER!!!
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	pop_gl_errors("OpenGL Environment Initialization");

#pragma region SHADER INITIALIZATION
	compile_shaders();
#pragma endregion // SHADER INITIALIZATION
	//model buffers & vao
	create_quad_mesh_for_rendered_text(&text_vao, text_vbo);
	create_line_mesh_dynamic(&line_vao, line_vbo);


#if 0
	create_depth_buffers(&depth_FBO, &tex[Texture::t_depth_map], SHADOW_MAP_H, SHADOW_MAP_H);
#endif	 // SHADOW MAP INITIALIZATION

	//initialize text rendering
	if (TTF_Init() == -1)
		std::cerr << TTF_GetError() << std::endl;

	//load font
	font = TTF_OpenFont("../data/fonts/OpenSans-Regular.ttf", 16);

	//load ImGUI
#pragma region LOAD ImGUI
	//IMGUI_CHECKVERSION();	// NO LINKEA!!!
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui_ImplSdlGL3_Init(window);
	ImGui::StyleColorsDark();
#pragma endregion

	//check for GL errors
	pop_gl_errors(__FUNCTION__);
}
void Graphics::free()
{
	//free ImGUI
	ImGui_ImplSdlGL3_Shutdown();
	ImGui::DestroyContext();

	//free font texture
	TTF_CloseFont(font);
	//delete all textures
	for (int i = 0; i < Texture::texture_count; ++i)
		glDeleteTextures(1, &tex[i]);
	for (auto &t : texture_map)
		glDeleteTextures(1, &t.second);
	texture_map.clear();

	//delete model
	glDeleteBuffers(sizeof(text_vbo) / sizeof(GLuint), text_vbo);
	glDeleteVertexArrays(1, &text_vao);
	glDeleteBuffers(sizeof(line_vbo) / sizeof(GLuint), line_vbo);
	glDeleteVertexArrays(1, &line_vao);
	//delete shaders
	delete_shaders();
	//delete depth buffer
#if 0
	glDeleteFramebuffers(1, &depth_FBO);
#endif

	//free window & gl_context
	SDL_GL_DeleteContext(gl_context_id);
	SDL_DestroyWindow(window);
}

// NDC coordinates x{-1,1},y{-1,1}
mat3 Graphics::window_to_ndc() const
{
	// remember that glm matrices are column mayor, transpose what you know!
	return mat3(
		2.f / window_surface->w, 0, 0,
		0, -2.f / window_surface->h, 0,
		-1, 1, 1);
}

mat3 Graphics::ndc_to_window() const
{
	return mat3(
		window_surface->w / 2.f, 0, 0,
		0, window_surface->h / 2.f, 0,
		window_surface->w / 2.f , window_surface->h / 2.f , 1
	);
}

// true if fullscree, false if windowed(opposite to 0 and -1 from SDL_SetWindowFullscreen
bool Graphics::toggle_fullscreen()
{
	is_fullscreen = !is_fullscreen;
	if (is_fullscreen) {
		if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0)
			return is_fullscreen = false;
		// WINDOW SIZE IS UPDATED IN THE SDL_WINDOWEVENT_RESIZE
	}
	else
	{

		if (SDL_SetWindowFullscreen(window, 0) < 0)
			return is_fullscreen = true;
		resize_window(WINDOW_W, WINDOW_H);
	}

	//unlock the mouse
	SDL_SetWindowGrab(window, SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_FALSE);

	return is_fullscreen;
}

void Graphics::resize_window(int size_x, int size_y)
{
	SDL_SetWindowSize(window, size_x, size_y);
	//SDL_SetWindowPosition(window, pos_x, pos_y);
	window_surface->w = size_x;
	window_surface->h = size_y;
	glViewport(0, 0, size_x, size_y);
}

void Graphics::render_text(const std::string &text, f32 ndc_x, f32 ndc_y, f32 fontSize, Color color)
{
	if (text.empty())
		return;
	SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), SDL_Color{ 255,255,255, 0 });
	if (textSurface == NULL) {
		std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
	}
	else {
		//disable depth
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glUseProgram(shader_program[Shader::sh_text]);

		//create texture from surface pixels
		assert(tex[Texture::t_text] == GL_NONE);	// no memory leaks
		glGenTextures(1, &tex[Texture::t_text]);
		glUniform1i(getUniformTexLoc(), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex[Texture::t_text]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textSurface->w, textSurface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, textSurface->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//bind to opengl and render
		glBindVertexArray(text_vao);

		// invert scaleY as text image loads downwards
		mat4 model_mtx = glm::translate(vec3{ ndc_x, ndc_y, 0.f }) * glm::scale(vec3{ fontSize * textSurface->w, -fontSize * textSurface->h, 1.f });;
		glUniformMatrix4fv(getUniformMtxLoc(), 1, GL_FALSE, (const float*)&model_mtx[0]);
		color.set_uniform_RGBA(getUniformColorLoc());
		//draw model
		glDisable(GL_CULL_FACE);	// fix the Y flip
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glEnable(GL_CULL_FACE);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		//delete texture
		glDeleteTextures(1, &tex[Texture::t_text]);
		tex[Texture::t_text] = GL_NONE;
		//delete surface
		SDL_FreeSurface(textSurface);

		pop_gl_errors(__FUNCTION__);
	}
}

#pragma region LINE DRAWING
//render a circle made with sh_lines
void Graphics::draw_circle(const vec2& ndc_center, f32 radius, u32 segments, Color color)
{
	const f32 step = f32(2 * M_PI / segments);
	f32 angle = 0.f;
	for (u32 i = 0; i < segments - 1; i++) {
		vec2 p0 = vec2{ cos(angle), sin(angle) } *radius;
		angle += step;
		vec2 p1 = vec2{ cos(angle), sin(angle) } *radius;
		graphics.draw_line(ndc_center + p0, ndc_center + p1, color, color);
	}
	graphics.draw_line(ndc_center + vec2{ cos(angle), sin(angle) } *radius, ndc_center + vec2{ radius, 0 }, color, color);
}
/*
	Create a sh_line request but don't draw immediately
*/
void Graphics::draw_line(const vec2& ndc_start, const vec2& ndc_end, Color color_start, Color color_end)
{
	line_positions_to_render.emplace_back(ndc_start);
	line_positions_to_render.emplace_back(ndc_end);
	line_colors_to_render.emplace_back(color_start);
	line_colors_to_render.emplace_back(color_end);
}

/*
	create a single model and render sh_lines
*/
void Graphics::draw_lines()
{
	if (line_positions_to_render.empty())
		return;
	//prevent errors if buffer is smaller than sh_line count
	const size_t count = line_positions_to_render.size() > MAX_LINES ? MAX_LINES : line_positions_to_render.size();
	//vertex buffer object
	glBindVertexArray(line_vao);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(shader_program[Shader::sh_line2D]);

	//bind positions
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, MAX_LINES * sizeof(vec2) * 2, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(vec2), &line_positions_to_render[0]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// bind colors
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, MAX_LINES * sizeof(Color) * 2, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(Color), &line_colors_to_render[0]);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);


	glDrawArrays(GL_LINES, 0, count);

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);

	pop_gl_errors(__FUNCTION__);
}
void Graphics::draw_lines(const vec2* points, const Color* colors, const u32 point_count, GLuint draw_mode)
{
	if (point_count == 0)
		return;
	//vertex buffer object
	glBindVertexArray(line_vao);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(shader_program[Shader::sh_line2D]);

	//bind positions
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, MAX_LINES * sizeof(vec2) * 2, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, point_count * sizeof(vec2), points);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// bind colors
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, MAX_LINES * sizeof(Color) * 2, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, point_count * sizeof(Color), colors);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

	glDrawArrays(draw_mode, 0, point_count);

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);

	pop_gl_errors(__FUNCTION__);

}
void Graphics::clear_lines()
{
	line_positions_to_render.clear();
	line_colors_to_render.clear();
}
#pragma endregion

#pragma region SHADERS
void Graphics::delete_shaders() 
{
	for (int i = 0; i < Shader::shader_count; i++)
		glDeleteProgram(shader_program[i]);
}
void Graphics::compile_shaders()
{
#pragma region Initialize TEXT shader program
	//vertex shader
	GLuint vertex_shader = compile_shader("../data/shaders/text.vert");
	//fragment shader
	GLuint fragment_shader = compile_shader("../data/shaders/text.frag");
	//compile shader program & link
	shader_program[Shader::sh_text] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_text], vertex_shader);
	glAttachShader(shader_program[Shader::sh_text], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_text]);
	//set uniforms
	if (0 <= (uniform_mtx_loc = glGetUniformLocation(shader_program[Shader::sh_text], "M")))
		glBindAttribLocation(shader_program[Shader::sh_text], uniform_mtx_loc, "M");
	else
		MessageBox(NULL, std::string("\"M\" uniform name not found in shader.").c_str(), "Attribute binding error", MB_ICONERROR | MB_OK);
	if (0 <= (uniform_color_loc = glGetUniformLocation(shader_program[Shader::sh_text], "color")))
		glBindAttribLocation(shader_program[Shader::sh_text], uniform_color_loc, "color");
	else
		MessageBox(NULL, std::string("\"color\" uniform name not found in shader.").c_str(), "Attribute binding error", MB_ICONERROR | MB_OK);
	if (0 <= (uniform_tex_loc = glGetUniformLocation(shader_program[Shader::sh_text], "tex")))
		glBindAttribLocation(shader_program[Shader::sh_text], uniform_tex_loc, "tex");
	else
		MessageBox(NULL, std::string("\"tex\" uniform name not found in shader.").c_str(), "Attribute binding error", MB_ICONERROR | MB_OK);


	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_text]);
	pop_gl_errors("Text Shader Initialization");
#pragma endregion
#pragma region Initialize sh_line2D shader program
	//compile sh_line shader
	//GLuint geometry_shader = compile_shader("../resources/shaders/sh_lines.geo");
	vertex_shader = compile_shader("../data/shaders/line2D.vert");
	fragment_shader = compile_shader("../data/shaders/line.frag");

	//compile sh_line shader program
	shader_program[Shader::sh_line2D] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_line2D], vertex_shader);
	glAttachShader(shader_program[Shader::sh_line2D], fragment_shader);
	//glAttachShader(sh_line_shader_program, geometry_shader);
	glLinkProgram(shader_program[Shader::sh_line2D]);


	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_line2D]);
	pop_gl_errors("line2D Shader Initialization");
#pragma endregion
#pragma region Initialize sh_line3D shader program
	//compile sh_line shader
	//GLuint geometry_shader = compile_shader("../resources/shaders/sh_lines.geo");
	vertex_shader = compile_shader("../data/shaders/line3D.vert");
	fragment_shader = compile_shader("../data/shaders/line.frag");

	//compile sh_line shader program
	shader_program[Shader::sh_line3D] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_line3D], vertex_shader);
	glAttachShader(shader_program[Shader::sh_line3D], fragment_shader);
	//glAttachShader(sh_line_shader_program, geometry_shader);
	glLinkProgram(shader_program[Shader::sh_line3D]);


	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_line3D]);
	pop_gl_errors("line3D Shader Initialization");

#pragma endregion
#pragma region Initialize sh_basic shader program
	//vertex shader
	vertex_shader = compile_shader("../data/shaders/basic.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/basic.frag");
	//compile shader program & link
	shader_program[Shader::sh_basic] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_basic], vertex_shader);
	glAttachShader(shader_program[Shader::sh_basic], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_basic]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_basic]);
	pop_gl_errors("Basic Shader Initialization");

#pragma endregion
#pragma region SKYBOX
	//vertex shader
	vertex_shader = compile_shader("../data/shaders/skybox.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/cubemap.frag");
	//compile shader program & link
	shader_program[Shader::sh_skybox] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_skybox], vertex_shader);
	glAttachShader(shader_program[Shader::sh_skybox], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_skybox]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_skybox]);

	pop_gl_errors("sh_skybox Shader Initialization");

#pragma endregion
#pragma region Initialize LIGHT shader program
	//vertex shader
	vertex_shader = compile_shader("../data/shaders/light.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/light.frag");
	//compile shader program & link
	shader_program[Shader::sh_light] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_light], vertex_shader);
	glAttachShader(shader_program[Shader::sh_light], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_light]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_light]);

	pop_gl_errors("Light Shader Initialization");

#pragma endregion
#pragma region GEOMETRY SHADERS
	//vertex shader
	vertex_shader = compile_shader("../data/shaders/geometry_pass.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/geometry_pass.frag");
	//compile shader program & link
	shader_program[Shader::sh_geometry_pass] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_geometry_pass], vertex_shader);
	glAttachShader(shader_program[Shader::sh_geometry_pass], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_geometry_pass]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_geometry_pass]);

	pop_gl_errors("Geometry Buffer Shader Initialization");


	//vertex shader
	vertex_shader = compile_shader("../data/shaders/lighting_pass.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/lighting_pass.frag");
	//compile shader program & link
	shader_program[Shader::sh_lighting_pass] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_lighting_pass], vertex_shader);
	glAttachShader(shader_program[Shader::sh_lighting_pass], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_lighting_pass]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_lighting_pass]);
	// configure shader
	glUseProgram(shader_program[Shader::sh_lighting_pass]);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_lighting_pass], "gPosition"), 0);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_lighting_pass], "gNormalShininess"), 1);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_lighting_pass], "gAlbedoSpec"), 2);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_lighting_pass], "t_ambient_occlusion"), 3);

	pop_gl_errors("Deferred Shader Initialization");


	//vertex shader
	vertex_shader = compile_shader("../data/shaders/decal.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/decal.frag");
	//compile shader program & link
	shader_program[Shader::sh_decal] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_decal], vertex_shader);
	glAttachShader(shader_program[Shader::sh_decal], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_decal]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_decal]);
	// configure shader
	glUseProgram(shader_program[Shader::sh_decal]);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_decal], "t_diffuse"), 0);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_decal], "t_normal"), 1);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_decal], "t_depth"), 2);

	pop_gl_errors("Decal Shader Initialization");

	//vertex shader
	vertex_shader = compile_shader("../data/shaders/ssao_pass.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/ssao_pass.frag");
	//compile shader program & link
	shader_program[Shader::sh_ssao] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_ssao], vertex_shader);
	glAttachShader(shader_program[Shader::sh_ssao], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_ssao]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_ssao]);

	glUseProgram(shader_program[Shader::sh_ssao]);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_ssao], "t_position"), 0);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_ssao], "t_normal"), 1);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_ssao], "t_noise"), 2);

	// configure shader	TODO
	//glUseProgram(shader_program[Shader::sh_ambient_occlusion]);
	//glUniform1i(glGetUniformLocation(shader_program[Shader::sh_ambient_occlusion], "gPosition"), 0);
	//glUniform1i(glGetUniformLocation(shader_program[Shader::sh_ambient_occlusion], "gNormalShininess"), 1);
	//glUniform1i(glGetUniformLocation(shader_program[Shader::sh_ambient_occlusion], "gAlbedoSpec"), 2);

	pop_gl_errors("SSAO Shader Initialization");

	//vertex shader
	vertex_shader = compile_shader("../data/shaders/hbao_pass.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/hbao_pass.frag");
	//compile shader program & link
	shader_program[Shader::sh_hbao] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_hbao], vertex_shader);
	glAttachShader(shader_program[Shader::sh_hbao], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_hbao]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_hbao]);

	glUseProgram(shader_program[Shader::sh_hbao]);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_hbao], "t_position"), 0);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_hbao], "t_normal"), 1);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_hbao], "t_noise"), 2);

	pop_gl_errors("HBAO Shader Initialization");

#pragma endregion
#pragma region POST-PROCESSING
	//vertex shader (recycle)
	vertex_shader = compile_shader("../data/shaders/text.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/sobel.frag");
	//compile shader program & link
	shader_program[Shader::sh_sobel] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_sobel], vertex_shader);
	glAttachShader(shader_program[Shader::sh_sobel], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_sobel]);

	//glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_sobel]);
	glUseProgram(graphics.shader_program[sh_sobel]);
	glUniform1i(glGetUniformLocation(graphics.shader_program[sh_sobel], "depth_texture"), 0);

	pop_gl_errors("Sobel Shader Initialization");

	//vertex shader
	//vertex_shader = compile_shader("../data/shaders/text.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/antialiasing.frag");
	//compile shader program & link
	shader_program[Shader::sh_antialiasing] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_antialiasing], vertex_shader);
	glAttachShader(shader_program[Shader::sh_antialiasing], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_antialiasing]);

	//glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_antialiasing]);
	glUseProgram(graphics.shader_program[sh_antialiasing]);
	glUniform1i(glGetUniformLocation(graphics.shader_program[sh_antialiasing], "color_texture"), 0);
	glUniform1i(glGetUniformLocation(graphics.shader_program[sh_antialiasing], "edges_texture"), 1);

	pop_gl_errors("AntiAliasing Shader Initialization");

	//vertex shader
	//vertex_shader = compile_shader("../data/shaders/text.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/hdr.frag");
	//compile shader program & link
	shader_program[Shader::sh_hdr] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_hdr], vertex_shader);
	glAttachShader(shader_program[Shader::sh_hdr], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_hdr]);

	//glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_hdr]);
	glUseProgram(graphics.shader_program[sh_hdr]);
	glUniform1i(glGetUniformLocation(graphics.shader_program[sh_hdr], "hdr_texture"), 0);

	pop_gl_errors("HDR Shader Initialization");

	//vertex shader
	//vertex_shader = compile_shader("../data/shaders/text.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/blur.frag");
	//compile shader program & link
	shader_program[Shader::sh_blur] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_blur], vertex_shader);
	glAttachShader(shader_program[Shader::sh_blur], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_blur]);

	//glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_blur]);
	glUseProgram(graphics.shader_program[sh_blur]);
	glUniform1i(glGetUniformLocation(graphics.shader_program[sh_blur], "color_texture"), 0);

	pop_gl_errors("Blur Shader Initialization");

	//vertex shader
	//vertex_shader = compile_shader("../data/shaders/text.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/bloom.frag");
	//compile shader program & link
	shader_program[Shader::sh_bloom] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_bloom], vertex_shader);
	glAttachShader(shader_program[Shader::sh_bloom], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_bloom]);

	//glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_bloom]);
	glUseProgram(graphics.shader_program[sh_bloom]);
	glUniform1i(glGetUniformLocation(graphics.shader_program[sh_bloom], "color_texture"), 0);
	glUniform1i(glGetUniformLocation(graphics.shader_program[sh_bloom], "hdr_texture"), 1);


	pop_gl_errors("Bloom Shader Initialization");

#pragma region 2 pass blur
	fragment_shader = compile_shader("../data/shaders/gaussian_blur.frag");
	//compile shader program & link
	shader_program[Shader::sh_gaussian_blur] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_gaussian_blur], vertex_shader);
	glAttachShader(shader_program[Shader::sh_gaussian_blur], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_gaussian_blur]);

	//glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_gaussian_blur]);
	glUseProgram(graphics.shader_program[sh_gaussian_blur]);
	glUniform1i(glGetUniformLocation(graphics.shader_program[sh_gaussian_blur], "color_texture"), 0);


	pop_gl_errors("Gaussian Blur Shader Initialization");


	fragment_shader = compile_shader("../data/shaders/bilateral_blur.frag");
	//compile shader program & link
	shader_program[Shader::sh_bilateral_blur] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_bilateral_blur], vertex_shader);
	glAttachShader(shader_program[Shader::sh_bilateral_blur], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_bilateral_blur]);

	//glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_bilateral_blur]);
	glUseProgram(graphics.shader_program[sh_bilateral_blur]);
	glUniform1i(glGetUniformLocation(graphics.shader_program[sh_bilateral_blur], "color_texture"), 0);


	pop_gl_errors("Bilinear Blur Shader Initialization");

#pragma endregion
#pragma endregion
#pragma region DEBUG
	//vertex shader
	vertex_shader = compile_shader("../data/shaders/text.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/depth_color.frag");
	//compile shader program & link
	shader_program[Shader::sh_depth_color] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_depth_color], vertex_shader);
	glAttachShader(shader_program[Shader::sh_depth_color], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_depth_color]);

	//glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_depth_color]);
	glUseProgram(graphics.shader_program[sh_depth_color]);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_depth_color], "shadowMap"), 0);


	pop_gl_errors("sh_depth_color Shader Initialization");

	//vertex shader
	//vertex_shader = compile_shader("../data/shaders/text.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/red_color.frag");
	//compile shader program & link
	shader_program[Shader::sh_red_color] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_red_color], vertex_shader);
	glAttachShader(shader_program[Shader::sh_red_color], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_red_color]);

	//glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_red_color]);
	glUseProgram(graphics.shader_program[sh_red_color]);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_red_color], "shadowMap"), 0);


	pop_gl_errors("sh_red_color Shader Initialization");
	//vertex shader
	//vertex_shader = compile_shader("../data/shaders/text.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/alpha_color.frag");
	//compile shader program & link
	shader_program[Shader::sh_alpha_color] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_alpha_color], vertex_shader);
	glAttachShader(shader_program[Shader::sh_alpha_color], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_alpha_color]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_alpha_color]);
	glUseProgram(graphics.shader_program[sh_alpha_color]);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_alpha_color], "diffuse_texture"), 0);


	pop_gl_errors("sh_alpha_color Shader Initialization");

#pragma endregion
#pragma region TERRAIN
	//vertex shader
	vertex_shader = compile_shader("../data/shaders/terrain_perlin.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/terrain_perlin.frag");
	//compile shader program & link
	shader_program[Shader::sh_terrain_perlin] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_terrain_perlin], vertex_shader);
	glAttachShader(shader_program[Shader::sh_terrain_perlin], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_terrain_perlin]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_terrain_perlin]);

	pop_gl_errors("Terrain Perlin Shader Initialization");

	//vertex shader
	vertex_shader = compile_shader("../data/shaders/terrain_map.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/terrain_map.frag");
	//compile shader program & link
	shader_program[Shader::sh_terrain_map] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_terrain_map], vertex_shader);
	glAttachShader(shader_program[Shader::sh_terrain_map], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_terrain_map]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_terrain_map]);
	glUseProgram(graphics.shader_program[sh_terrain_map]);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_terrain_map], "t_height"), 3);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_terrain_map], "t_noise"), 4);

	pop_gl_errors("Terrain Map Shader Initialization");

	//vertex shader
	vertex_shader = compile_shader("../data/shaders/terrain_cubemap.vert");
	//fragment shader
	fragment_shader = compile_shader("../data/shaders/geometry_pass.frag");
	//compile shader program & link
	shader_program[Shader::sh_terrain_cubemap] = glCreateProgram();
	glAttachShader(shader_program[Shader::sh_terrain_cubemap], vertex_shader);
	glAttachShader(shader_program[Shader::sh_terrain_cubemap], fragment_shader);
	glLinkProgram(shader_program[Shader::sh_terrain_cubemap]);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	check_gl_program_link_errors(shader_program[Shader::sh_terrain_cubemap]);
	glUseProgram(graphics.shader_program[sh_terrain_cubemap]);
	glUniform1i(glGetUniformLocation(shader_program[Shader::sh_terrain_cubemap], "t_height_cubemap"), 3);


	pop_gl_errors("Terrain Cubemap Shader Initialization");


#pragma endregion
}
#pragma endregion
