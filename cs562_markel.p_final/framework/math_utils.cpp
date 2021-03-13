#include "math_utils.h"

u8 intersect_segment_to_moving_sphere(const vec3 &p0, const vec3 &p1, const vec3 &center, f32 radius, const vec3 &vel
	, f32 &dist_travel, vec3 *reaction) {
	vec4 plane = plane_equation(p0, p1, p1 - vel);
	f32 d = glm::dot(vec3{ plane }, center) + plane.w;
	if (d > radius || d < -radius)
		return 0;
	const f32 rr = radius * radius;
	f32 r = sqrt(rr - d * d);
	vec3 pt0 = center - vec3{ plane } *d;	// projection of Center into the plane
	vec3 onLine;
	f32 h = distance_point_to_line(pt0, p0, p1, &onLine);
	vec3 v = onLine - pt0;
	v = glm::normalize(v);
	vec3 pt1 = v * r + pt0;	// point on the sphere that will maybe collide with the edge
	int a0 = 0, a1 = 1;
	f32 pl_x = fabsf(plane.x);
	f32 pl_y = fabsf(plane.y);
	f32 pl_z = fabsf(plane.z);
	if (pl_x > pl_y && pl_x > pl_z) {
		a0 = 1;
		a1 = 2;
	}
	else if (pl_y > pl_z) {
		a0 = 0;
		a1 = 2;
	}
	vec3 vv = pt1 + vel;
	f32 t;
	bool res = intersect_line_to_line(vec2{ pt1[a0], pt1[a1] }, vec2{ vv[a0], vv[a1] }, vec2{ p0[a0], p0[a1] }, vec2{ p1[a0], p1[a1] }, t);
	if (!res || t < 0)
		return 0;
	vec3 inter = pt1 + vel * t;
	vec3 r1 = p0 - inter;
	vec3 r2 = p1 - inter;
	if (glm::dot(r1, r2) > 0.f)
		return 0;
	if (t > dist_travel)
		return 0;
	dist_travel = t;
	if (reaction)
		*reaction = center - pt1;
	return 2;	// two collisions (asume los vertices ya checkeados y no colisionados)
}

bool intersect_triangle_to_moving_sphere(const vec3 &p0, const vec3 &p1, const vec3 &p2
	, const vec3 &center, f32 radius, vec3 &vel, f32 &dist_travel, vec3 * _reaction)
{
	// code from http://flipcode.com/archives/Moving_Sphere_VS_Triangle_Collision.shtml
	vec3 nvel = glm::normalize(vel);
	vec3 p0p1 = p1 - p0;
	vec3 p0p2 = p2 - p0;
	vec3 tri_normal = glm::normalize(glm::cross(p0p1, p0p2));
	f32 n_dot_v = glm::dot(tri_normal, nvel);
	if (n_dot_v > -EPSILON)
		return false;
	f32 minDist = FLT_MAX;
	int col = -1;
	dist_travel = FLT_MAX;
	vec4 plane = plane_equation(p0, tri_normal);

	// pass1: sphere VS plane
	f32 h = glm::dot(vec3{ plane }, center) + plane.w;
	if (h < -radius)
		return false;
	if (h > radius) {
		h -= radius;
		if (n_dot_v != 0.f) {
			f32 t = -h / n_dot_v;
			vec3 onPlane = center + nvel * t;
			if (is_point_inside_triangle(p0, p1, p2, onPlane)) {
				if (t < dist_travel) {
					dist_travel = t;
					if (_reaction)
						*_reaction = tri_normal;
					col = 0;
				}
			}
		}
	}
	// pass2: sphere VS triangle vertices
#pragma region vertex test
	// p0
	f32 inter1 = FLT_MAX, inter2 = FLT_MAX;
	u8 col_num = intersect_point_to_moving_sphere(p0, center, radius, nvel, &inter1, &inter2);
	if (col_num != 0) {
		f32 t = inter1;
		if (inter2 < t)
			t = inter2;
		if (t >= 0.f) {
			if (t < dist_travel) {
				dist_travel = t;
				vec3 onSphere = p0 + nvel * t;
				if (_reaction)
					*_reaction = center - onSphere;
				col = 1;
			}
		}

	}
	// p1
	inter1 = FLT_MAX; inter2 = FLT_MAX;
	col_num = intersect_point_to_moving_sphere(p1, center, radius, nvel, &inter1, &inter2);
	if (col_num != 0) {
		f32 t = inter1;
		if (inter2 < t)
			t = inter2;
		if (t >= 0.f) {
			if (t < dist_travel) {
				dist_travel = t;
				vec3 onSphere = p1 + nvel * t;
				if (_reaction)
					*_reaction = center - onSphere;
				col = 1;
			}
		}
	}
	// p2
	inter1 = FLT_MAX; inter2 = FLT_MAX;
	col_num = intersect_point_to_moving_sphere(p2, center, radius, nvel, &inter1, &inter2);
	if (col_num != 0) {
		f32 t = inter1;
		if (inter2 < t)
			t = inter2;
		if (t >= 0.f) {
			if (t < dist_travel) {
				dist_travel = t;
				vec3 onSphere = p2 + nvel * t;
				if (_reaction)
					*_reaction = center - onSphere;
				col = 1;
			}
		}
	}
#pragma endregion
	// pass3: sphere VS triangle edges
#pragma region edge test
	// edge p0p1
	col = intersect_segment_to_moving_sphere(p0, p1, center, radius, nvel, dist_travel, _reaction);
	col = intersect_segment_to_moving_sphere(p1, p2, center, radius, nvel, dist_travel, _reaction);
	col = intersect_segment_to_moving_sphere(p2, p0, center, radius, nvel, dist_travel, _reaction);

#pragma endregion
	if (_reaction && col != -1)
		*_reaction = glm::normalize(*_reaction);

	return col >= 0;
}


bool intersect_triangle_to_sphere(const vec3 &p0, const vec3 &p1, const vec3 &p2, const vec3 &c, f32 r)
{
	// translate problem so sphere is centered at origin
	vec3 A = p0 - c;
	vec3 B = p1 - c;
	vec3 C = p2 - c;
	f32 rr = r * r;
	// compute a vector normal to triangle plane (V)
	vec3 V = glm::cross(B - A, C - A);
	// compute distance "d" of sphere center to triangle plane
	f32 d = glm::dot(A, V);
	f32 e = glm::dot(V, V);
	bool separated = d * d > rr * e;
	if (separated)
		return false;

	// vertex testing

	// compute distance between sphere center and vertex A
	f32 aa = glm::dot(A, A);
	f32 ab = glm::dot(A, B);
	f32 ac = glm::dot(A, C);
	f32 bb = glm::dot(B, B);
	f32 bc = glm::dot(B, C);
	f32 cc = glm::dot(C, C);
	// the plane through A with normal N separates sphere iff: 
	// (1) A lies outside the sphere, and 
	// (2) if B and C lie on the opposite side of the plane w.r.t the sphere center
	bool separated_a = (aa > rr) && (ab > aa) && (ac > aa);
	bool separated_b = (bb > rr) && (ab > bb) && (bc > bb);
	bool separated_c = (cc > rr) && (ac > cc) && (bc > cc);
	separated = separated || separated_a || separated_b || separated_c;

	// edge testing

	// edge AB
	vec3 E = B - A;
	d = ab - aa;
	e = glm::length2(E);
	vec3 Q = A * e - d * E;
	vec3 QE = C * e - Q;
	separated_a = (glm::length2(Q) > rr * e * e) && (glm::dot(Q, QE) > 0.f);

	// edge BC
	E = C - B;
	d = bc - bb;
	e = glm::length2(E);
	Q = B * e - d * E;
	QE = A * e - Q;
	separated_b = (glm::length2(Q) > rr * e * e) && (glm::dot(Q, QE) > 0.f);

	// edge CA
	E = A - C;
	d = ac - cc;
	e = glm::length2(E);
	Q = C * e - d * E;
	QE = B * e - Q;
	separated_c = (glm::length2(Q) > rr * e * e) && (glm::dot(Q, QE) > 0.f);

	separated = separated || (separated_a || separated_b || separated_c);

	return !separated;	// return TRUE if not separated, aka intersected
}
