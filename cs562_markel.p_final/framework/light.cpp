#include "light.h"
#include "camera.h"

void light_scissor_optimization(const PointLight& light, f32 min_attenuation, const Camera& camera, bool show_scissor_test)
{
	f32 discriminant = pow(min_attenuation * light.attenuation.y, 2)
		- 4 * min_attenuation * light.attenuation.z * (min_attenuation * light.attenuation.x - 1);
	assert(discriminant >= 0.f);
	f32 r1 = (-(min_attenuation * light.attenuation.y) + sqrt(discriminant)) / (2 * min_attenuation * light.attenuation.z);
	//if camera inside radius, affect the whole viewport
	if (glm::length2(camera.pos - light.position) > r1 * r1)
	{
		// discard lights behind camera
		f32 side = glm::dot(glm::normalize(camera.target - camera.pos), light.position - camera.pos);
		if (side < 0)
			return;

		// do scissor test
		const mat4 cam_mtx = camera.get_view();
		// convert R1 to camera space
		const vec3 cam_light_pos = cam_mtx * vec4{ light.position, 1.f };
		const vec3 cam_r_pos = cam_mtx * vec4{ vec3{ r1, 0, 0 } +light.position, 1.f };
		//R in view space
		r1 = glm::length(cam_r_pos - cam_light_pos);
		// contruct bounding box
		vec3 bounding_rect[2] = {
			cam_light_pos + vec3{ r1, r1, 0 },	/* right, top */
			cam_light_pos + vec3{ -r1, -r1, 0 }	/* left, bot */
		};
		// debug lines!
		vec2 bounding_rect_ndc[2] = {};
		// to ndc
		const mat4 proj_mtx = camera.get_proj();
#pragma omp parallel for
		for (u32 i = 0; i < sizeof(bounding_rect) / sizeof(vec3); i++) {
			vec4 p = proj_mtx * vec4{ bounding_rect[i], 1.f };
			// ndc
			bounding_rect[i] = vec3{ p } / p.w;
			bounding_rect_ndc[i] = bounding_rect[i];
			// screen space
			bounding_rect[i] = graphics.ndc_to_window() * bounding_rect[i];
		}

		//glViewport(bounding_rect[1].x, bounding_rect[1].y,
		//	bounding_rect[0].x - bounding_rect[1].x, bounding_rect[0].y - bounding_rect[1].y);
		GLsizei width =  GLsizei(bounding_rect[0].x - bounding_rect[1].x);
		GLsizei height = GLsizei(bounding_rect[0].y - bounding_rect[1].y);
#if 1
		// clamp edges
		vec2 left_bot = { glm::max(0.f, bounding_rect[1].x), glm::max(0.f, bounding_rect[1].y) };
		vec2 right_top = { glm::min(WINDOW_W, glm::max(0, width)), glm::min(WINDOW_H, glm::max(0, height)) };

		//glScissor(glm::max(0.f, bounding_rect[1].x), glm::max(0.f, bounding_rect[1].y),
		//	glm::min(WINDOW_W, abs(width)), glm::min(WINDOW_H, abs(height)));

		glScissor(	(GLsizei)left_bot.x,	(GLsizei)left_bot.y,
					(GLsizei)right_top.x ,	(GLsizei)right_top.y);

		if (show_scissor_test) {
			graphics.draw_line(bounding_rect_ndc[0], vec2{ bounding_rect_ndc[0].x, bounding_rect_ndc[1].y });	// right
			graphics.draw_line(bounding_rect_ndc[1], vec2{ bounding_rect_ndc[0].x, bounding_rect_ndc[1].y });	// bot
			graphics.draw_line(bounding_rect_ndc[1], vec2{ bounding_rect_ndc[1].x, bounding_rect_ndc[0].y });	// left
			graphics.draw_line(bounding_rect_ndc[0], vec2{ bounding_rect_ndc[1].x, bounding_rect_ndc[0].y });	// top
		}
#else
		// check if in frustrum
		vec2 win_half_size = vec2{ graphics.window_surface->w ,graphics.window_surface->h } / 2.f;
		vec2 bounding_rect_half_size = vec2{ width, height } / 2.f;
		if (width > 0 && height > 0 && AABB_to_AABB(win_half_size, win_half_size, vec2{ bounding_rect[1] } +bounding_rect_half_size, bounding_rect_half_size)) {
			glScissor(bounding_rect[1].x, bounding_rect[1].y,
				width, height);

			if (show_scissor_test) {
				graphics.draw_line(bounding_rect_ndc[0], vec2{ bounding_rect_ndc[0].x, bounding_rect_ndc[1].y });
				graphics.draw_line(bounding_rect_ndc[1], vec2{ bounding_rect_ndc[0].x, bounding_rect_ndc[1].y });
				graphics.draw_line(bounding_rect_ndc[1], vec2{ bounding_rect_ndc[1].x, bounding_rect_ndc[0].y });
				graphics.draw_line(bounding_rect_ndc[0], vec2{ bounding_rect_ndc[1].x, bounding_rect_ndc[0].y });
			}

			if (pop_gl_errors("Light Scissor"))
				printf("bad very bad");

		}
#endif
	}
	else {
		glScissor(0, 0, WINDOW_W, WINDOW_H);
	}
	pop_gl_errors(__FUNCTION__);
}

void AmbientLight::edit(const char * node_name)
{
	if (ImGui::TreeNode(node_name)) {
		vec3 tmp(f32(diffuse.r) / 255, f32(diffuse.g) / 255, f32(diffuse.b) / 255);
		ImGui::ColorEdit3("Diffuse", &tmp[0]);
		diffuse = Color{ vec4{ tmp, 1.f } };

		ImGui::TreePop();
	}
}

void DirectionalLight::edit(const char * node_name)
{
	if (ImGui::TreeNode(node_name)) {
		vec3 tmp(f32(diffuse.r) / 255, f32(diffuse.g) / 255, f32(diffuse.b) / 255);
		ImGui::ColorEdit3("Diffuse", &tmp[0]);
		diffuse = Color{ vec4{ tmp, 1.f } };
		tmp = vec3(f32(specular.r) / 255, f32(specular.g) / 255, f32(specular.b) / 255);
		ImGui::ColorEdit3("Specular", &tmp[0]);
		specular = Color{ vec4{ tmp, 1.f } };

		ImGui::DragFloat3("Direction", &direction[0], 0.01f, -10.f, 10.f);
		direction = glm::normalize(direction);

		ImGui::TreePop();
	}
}

void PointLight::edit(const char * node_name)
{
	if (ImGui::TreeNode(node_name)) {
		vec3 tmp(f32(diffuse.r) / 255, f32(diffuse.g) / 255, f32(diffuse.b) / 255);
		ImGui::ColorEdit3("Diffuse", &tmp[0]);
		diffuse = Color{ vec4{ tmp, 1.f } };
		tmp = vec3(f32(specular.r) / 255, f32(specular.g) / 255, f32(specular.b) / 255);
		ImGui::ColorEdit3("Specular", &tmp[0]);
		specular = Color{ vec4{ tmp, 1.f } };

		ImGui::DragFloat3("Position", &position[0], 0.01f, std::numeric_limits<f32>::min(), std::numeric_limits<f32>::max());
		ImGui::DragFloat3("Attenuation", &attenuation[0], 0.01f, 0.f, 10.f);

		ImGui::TreePop();
	}
}

void SpotLight::edit(const char * node_name)
{
	if (ImGui::TreeNode(node_name)) {
		vec3 tmp(f32(diffuse.r) / 255, f32(diffuse.g) / 255, f32(diffuse.b) / 255);
		ImGui::ColorEdit3("Diffuse", &tmp[0]);
		diffuse = Color{ vec4{ tmp, 1.f } };
		tmp = vec3(f32(specular.r) / 255, f32(specular.g) / 255, f32(specular.b) / 255);
		ImGui::ColorEdit3("Specular", &tmp[0]);
		specular = Color{ vec4{ tmp, 1.f } };

		ImGui::DragFloat3("Position", &position[0], 0.01f, std::numeric_limits<f32>::min(), std::numeric_limits<f32>::max());
		ImGui::DragFloat3("Direction", &direction[0], 0.01f, -10.f, 10.f);
		direction = glm::normalize(direction);
		ImGui::DragFloat3("Attenuation", &attenuation[0], 0.01f, 0.f, 10.f);
		tmp = vec3(angle0, angle1, falloff);
		ImGui::DragFloat2("In/Out/Falloff", &tmp[0], 0.01f, 0.f, 360.f);
		angle0 = tmp.x; angle1 = tmp.y; falloff = tmp.z;

		ImGui::TreePop();
	}
}