#pragma once
#include "Vectors.h"
#include "Matrixs.h"

struct Quaternion
{
	float x;
	float y;
	float z;
	float w;
};

struct Transform
{
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct QuaternionTransform
{
	Vector3 scale;
	Quaternion rotate;
	Vector3 translate;
};

static Quaternion operator*(const Quaternion& q1, const Quaternion& q2) {
	return {
		q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
		q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
	};
}