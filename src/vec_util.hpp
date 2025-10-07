#pragma once

#include <godot_cpp/variant/variant.hpp>


using namespace godot;

inline Vector4 ctor_vec3_f(Vector3 const &xyz, real_t const w = 0.0) {
	return Vector4(xyz.x, xyz.y, xyz.z, w);
}
