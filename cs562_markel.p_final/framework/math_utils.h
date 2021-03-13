/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2019 Markel Pisano's Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of Markel Pisano Berrojalbiz is prohibited.
File Name: math_utils.h
Purpose: define math types and utils
Language: c++
Platform: windows 10
Project: cs300_markel.p_0
Author: Markel Pisano Berrojalbiz
Creation date: 5/16/2019
----------------------------------------------------------------------------------------------------------*/
#pragma warning (disable : 4201)	// disable stupid "nameless struct" warning!

#ifndef MATH_UTILS_H
#define MATH_UTILS_H


#include <glm.hpp>
#include "gtc/matrix_transform.hpp"
#include "gtx/matrix_transform_2d.hpp"
#include "gtc/random.hpp"
#include "gtx/vector_angle.hpp"

#include <utility>	// std::pair


using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat2 = glm::mat2;
using mat3 = glm::mat3;
using mat4 = glm::mat4;

using ivec2 = glm::ivec2;

using f32 = float;
using f64 = double;
using u32 = unsigned int;
using u64 = long long unsigned int;
using u16 = unsigned short;
using u8 = unsigned char;

#ifndef M_PI
#define M_PI    3.14159265358979323846264338327950288   /**< pi */
#endif
#define DEG_TO_RAD (M_PI / 180.0)
#define RAD_TO_DEG (180.0 / M_PI)
#define EPSILON 0.000001f

#define isZero(x) ((x < EPSILON) && (x > -EPSILON))
#define isEqual(x,y) (isZero(abs(x - y)))

__forceinline bool point_to_AABB(const vec2& p, const vec2& pos, const vec2& sc)
{
	return p.x > pos.x - sc.x && p.x < pos.x + sc.x
		&&
		p.y > pos.y - sc.y && p.y < pos.y + sc.y;
}

// AABB defined by center(p0) + half size(sh0)
inline bool AABB_to_AABB(const vec2& p0, const vec2& sh0, const vec2& p1, const vec2& sh1) {
	return point_to_AABB(p0, p1, sh0 + sh1);
}

inline f32 angle_atan2(const vec2 &a, const vec2 &b) {
	f32 dot = glm::dot(a, b);
	f32 det = a.x * b.y - a.y * b.x;
	return atan2(det, dot);
}
inline f32 angle_atan2(const vec3 &a, const vec3 &b) {
	f32 dot = glm::dot(a, b);
	f32 det = a.x * b.y + b.x * a.z + a.y * b.z - b.y * a.z - b.x * a.y - b.z * a.x;
	return atan2(det, dot);
}

inline bool intersect_line_to_line(const vec2 &p0, const vec2 &d0, const vec2 &p1, const vec2 &d1, vec2 *intersection_point = nullptr) {
	//define intersection as A.a + t * diffA
	f32 det0 = d0.x * d1.y - d0.y * d1.x;
	if (det0 != 0.f) {
		f32 t = ((p0.x - p1.x) * d1.y - (p0.y - p1.y) * d1.x) / det0;
		if (intersection_point)
			*intersection_point = { p0 - d0 * t };
		return true;
	}
	return false;
}
inline bool intersect_line_to_line(const vec2 &p0, const vec2 &d0, const vec2 &p1, const vec2 &d1, f32 &t) {
	//define intersection as A.a + t * diffA
	f32 det0 = d0.x * d1.y - d0.y * d1.x;
	if (det0 != 0.f) {
		t = ((p0.x - p1.x) * d1.y - (p0.y - p1.y) * d1.x) / det0;
		return true;
	}
	return false;
}
inline bool intersect_segment_to_segment(const vec2 &s0, const vec2 &e0, const vec2 &s1, const vec2 &e1, vec2 *intersection_point = nullptr) {
	vec2 dif0 = s0 - e0;
	vec2 dif1 = s1 - e1;
	f32 det0 = dif0.x * dif1.y - dif0.y * dif1.x;
	if (det0 != 0.0f) {
		f32 difx = s0.x - s1.x;
		f32 dify = s0.y - s1.y;
		f32 s = (difx * dif0.y - dify * dif0.x) / det0;
		f32 t = (difx * dif1.y - dify * dif1.x) / det0;
		if (intersection_point)
			*intersection_point = s0 - dif0 * t;
		if (t >= 0.0f && t <= 1.0f && s >= 0.0f && s <= 1.0f)
			return true;
	}
	return false;
}
inline bool point_inside_triangle(const vec2 &a, const vec2 &b, const vec2 &c, const vec2 &p) {
	mat3 m;
	m[0] = { a, 1.f };
	m[1] = { b, 1.f };
	m[2] = { c, 1.f };
	f32 ABC = abs(glm::determinant(m) / 2.f);
	m[0] = { p, 1.f };
	f32 PBC = abs(glm::determinant(m) / 2.f);
	m[1] = { a, 1.f };
	f32 PAC = abs(glm::determinant(m) / 2.f);
	m[2] = { b, 1.f };
	f32 PAB = abs(glm::determinant(m) / 2.f);
	return isZero(ABC - PBC - PAC - PAB);
}
// return circle passing through ABC defined by center + radius2. If radius negative, no circumcircle posible
inline std::pair<vec2, f32> get_circumcircle(const vec2 &a, const vec2 &b, const vec2 &c) {
	const vec2 ab = b - a;
	const vec2 ac = c - a;
	vec2 intersection;
	if (intersect_line_to_line(a + ab / 2.f, { ab.y, -ab.x }, a + ac / 2.f, { ac.y, -ac.x }, &intersection)) {
		return { intersection, glm::length2(intersection - a) };
	}
	return { intersection, -1.f };
}

// return sphere passing through ABC defined by center + radius2. If radius negative, no circumsphere posible
inline std::pair<vec3, f32> get_circumsphere(const vec3 &a, const vec3 &b, const vec3 &c) {
	const vec3 ac = c - a;
	const vec3 ab = b - a;
	const vec3 abXac = glm::cross(ab, ac);
	const f32 len2 = glm::length2(abXac);
	if (isZero(len2)) {
		return { {}, -1.f };	// circumsphere too big!
	}
	// this is the vector from "a" to the circumsphere center
	vec3 to_center = (glm::cross(abXac, ab) * glm::length2(ac) + glm::cross(ac, abXac) * glm::length2(ab))
		/ (2.f * len2);
	vec3 center = a + to_center;
	return {center, glm::length2(to_center)};
}

inline bool same_clock_dir(const vec3 &d01, const vec3 &d02, const vec3 &norm) {
	const vec3 test = glm::cross(d01, d02);
	const f32 dot_prod = glm::dot(test, norm);
	if (dot_prod < 0)
		return false;	// different direction
	return true;	//same direction
}

inline bool intersect_line_triangle(const vec3 &p0, const vec3 &p1, const vec3 &p2, const vec3 &line_s, const vec3 &line_dir, vec3 *intersection = nullptr) {
	vec3 d01 = p1 - p0;
	vec3 d02 = p2 - p0;
	vec3 norm = glm::cross(d01, d02);
	f32 dot_prod = glm::dot(norm, line_dir);
	if (dot_prod < 0) {
		vec3 d = line_s - p0;
		f32 t = -glm::dot(norm, d) / dot_prod;

		if (t < 0)
			return false;	// no intersection
		vec3 p_int = line_s + line_dir * t;
		if (intersection)
			*intersection = p_int;
		if (same_clock_dir(d01, p_int - p0, norm))
			if (same_clock_dir(p2 - p1, p_int - p1, norm))
				if (same_clock_dir(-d02, p_int - p2, norm))
					return true;
	}
	return false;
}
inline bool intersect_ray_to_plane(const vec3& p, const vec3& dir, const vec4& plane, vec3 *point = nullptr) {
	assert(isEqual(glm::length2(dir), 1.f));
	vec3 n = plane;
	f32 dot_vn = glm::dot(dir, n);
	if (dot_vn == 0.f)	// parallel?
		return false;
	f32 t = -(glm::dot(p, n) + plane.w) / dot_vn;
	if (point)
		*point = p + dir * t;
	if (t < 0.f)	// behind
		return false;
	return true;
}
inline vec4 plane_equation(const vec3& p0, const vec3& p1, const vec3& p2) {
	vec3 v0 = p0 - p1;
	vec3 v1 = p2 - p1;
	vec3 n = glm::cross(v1, v0);
	n = glm::normalize(n);

	return vec4{n, -glm::dot(p0, n)};
}
inline vec4 plane_equation(vec3 point, vec3 normal) {
	assert(isEqual(glm::length2(normal), 1.f));
	
	return vec4{normal, -glm::dot(point, normal)};
}

// return square distance from p to line p0p1, optional closest point in line to p
inline f32 distance2_point_to_line(const vec3 &p, const vec3 &p0, const vec3 &p1, vec3 *line_p = nullptr) {
	vec3 v = p - p0;
	vec3 s = p1 - p0;
	f32 len2 = glm::length2(s);
	f32 dot = glm::dot(v, s) / len2;
	vec3 disp = s * dot;
	if (line_p)
		*line_p = p0 + disp;
	v -= disp;
	return glm::length2(v);
}
// return distance from p to line p0p1, optional closest point in line to p
inline f32 distance_point_to_line(const vec3 &p, const vec3 &p0, const vec3 &p1, vec3 *line_p = nullptr) {
	return sqrt(distance2_point_to_line(p, p0, p1, line_p));
}

// check if two spheres intersect and return plane as [Ax + By + Cz = D]
inline bool intersect_sphere_to_sphere(const vec3 &c0, f32 r0, const vec3 &c1, f32 r1, vec4 *intersection_plane = nullptr)
{
	vec3 diff = c1 - c0;
	f32 dist2 = glm::length2(diff);
	f32 R = r0 + r1;
	if (dist2 > R * R) {
		if (intersection_plane) {
			//calculate intersection plane
			intersection_plane->x = 2.f * (c1.x - c0.x);
			intersection_plane->y = 2.f * (c1.y - c0.y);
			intersection_plane->z = 2.f * (c1.z - c0.z);
			intersection_plane->w =
				c0.x * c0.x - c1.x * c1.x
				+ c0.y * c0.y - c1.y * c1.y
				+ c0.z * c0.z - c1.z * c1.z
				- r0 * r0 + r1 * r1;
		}
		return true;
	}

	return false;
}

inline bool is_point_inside_triangle(const vec3 &p0, const vec3 &p1, const vec3 &p2, const vec3 &p)
{
	vec3 u = p1 - p0;
	vec3 v = p2 - p0;
	vec3 w = p - p0;

	f32 uu = glm::dot(u, u);
	f32 uv = glm::dot(u, v);
	f32 vv = glm::dot(v, v);
	f32 wu = glm::dot(w, u);
	f32 wv = glm::dot(w, v);
	f32 d = uv * uv - uu * vv;
	f32 invD = 1.f / d;
	f32 s = (uv * wv - vv * wu) * invD;
	if (s < 0.f || s > 1.f)
		return false;
	f32 t = (uv * wu - uu * wv) * invD;
	if (t < 0.f || t > 1.f)
		return false;
	return true;
}

// return number of intersections
inline u8 intersect_segment_to_sphere(const vec3 &start, const vec3 &end, const vec3 &center, f32 radius
	, f32 *first_intersection = nullptr, f32 *second_intersection = nullptr)
{
	f32 a, b, c, i;
	vec3 dif = end - start;
	a = glm::length2(dif);
	b = 2.f * glm::dot(dif, start - center);
	c = glm::length2(center) + glm::length2(start) - 2.f * glm::dot(center, start) - radius * radius;
	i = b * b - 4.f * a * c;
	if (i < 0.f)
		return 0;
	if (i == 0.f) {
		if (first_intersection)
			*first_intersection = -b / (2.f * a);
		return 1;
	}
	else {
		if (first_intersection)
			*first_intersection = (-b + sqrt(i)) / 2.f * a;
		if(second_intersection)
			*second_intersection = (-b - sqrt(i)) / 2.f * a;
	}
	return 2;
}

inline u8 intersect_point_to_moving_sphere(const vec3 &p, const vec3 &center, f32 radius, const vec3 &vel
								, f32 *first_intersection = nullptr, f32 *second_intersection = nullptr) {
	// convert this problem to segment vs sphere
	return intersect_segment_to_sphere(p, p - vel, center, radius, first_intersection, second_intersection);
}

u8 intersect_segment_to_moving_sphere(const vec3 &p0, const vec3 &p1, const vec3 &center, f32 radius, const vec3 &vel
	, f32 &dist_travel, vec3 *reaction);
bool intersect_triangle_to_moving_sphere(const vec3 &p0, const vec3 &p1, const vec3 &p2
	, const vec3 &center, f32 radius, vec3 &vel, f32 &dist_travel, vec3 * _reaction);

bool intersect_triangle_to_sphere(const vec3 &p0, const vec3 &p1, const vec3 &p2, const vec3 &c, f32 r);

#endif // MATH_UTILS_H