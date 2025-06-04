#pragma once

struct Vector2 {
	float x;
	float y;

	Vector2(float x = 0,float y = 0) : x(x), y(y){}

	Vector2 operator+=(const Vector2& v) { this->x += v.x; this->y += v.y; return *this; }
};

struct Vector3 {
	float x;
	float y;
	float z;

	Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

	// 🔹 **ベクトルの加算代入演算子を追加**
	Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
	// 🔹 **ベクトルの減算代入演算子を追加**
	Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	// 🔹 スカラー乗算代入演算子を追加
	Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
	// 🔹 **単項マイナス演算子を追加**
	Vector3 operator-() const { return Vector3(-x, -y, -z); }
};

// 既存の演算子オーバーロード
inline Vector3 operator+(const Vector3& v1, const Vector3& v2) { return Vector3{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z }; }
inline Vector3 operator-(const Vector3& v1, const Vector3& v2) { return Vector3{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z }; }
inline Vector3 operator*(const Vector3& v1, float s) { return Vector3{ v1.x * s, v1.y * s, v1.z * s }; }


struct Vector4 
{
	float x;
	float y;
	float z;
	float w;

   Vector4(float x = 0, float y = 0, float z = 0,float w = 0) : x(x), y(y), z(z), w(w) {}
};

inline Vector4 operator+(const Vector4& v1, const Vector4& v2) { return Vector4{ v1.x + v2.x,v1.y + v2.y,v1.z + v2.z,v1.w + v2.w }; }
inline Vector4 operator*(const Vector4& v1, float s) { return Vector4{ v1.x * s,v1.y * s,v1.z * s,v1.w * s }; }