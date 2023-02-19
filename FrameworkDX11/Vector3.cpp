#include "Vector3.h"

Vector3::Vector3()
{
	x = 0;
	y = 0;
	z = 0;
}

Vector3::Vector3(float x, float y, float z):x(x),y(y),z(z)
{
}

Vector3::Vector3(const Vector3& Vec)
{
	x = Vec.x;
	y = Vec.y;
	z = Vec.z;
}

Vector3 Vector3::operator+(const Vector3& vec)
{
	return Vector3(x+vec.x,y+vec.y,z+vec.z);
}

Vector3& Vector3::operator+=(const Vector3& vec)
{

	//Returns ‘this’ pointer (i.e. self-reference summing the values for 
	//each component with the corresponding component in the added vector

	x += vec.x;
	y += vec.y;
	z += vec.z;
	return *this;

}

Vector3 Vector3::operator-(const Vector3& vec)
{
	return Vector3(x - vec.x, y - vec.y, z - vec.z);
}

Vector3& Vector3::operator-=(const Vector3& vec)
{
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	return *this;
}

Vector3 Vector3::operator*(float value)
{
	return Vector3(x * value, y * value, z * value);
}

Vector3& Vector3::operator*=(float value)
{
	x *= value;
	y *= value;
	z *= value;
	return *this;
}

Vector3 Vector3::operator/(float value)
{
	assert(value != 0);
	return Vector3(x / value, y / value, z / value);
}

Vector3& Vector3::operator/=(float value)
{
	assert(value != 0);
	x /= value;
	y /= value;
	z /= value;
	return *this;
}

Vector3& Vector3::operator=(const Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;

	return *this;
}


float Vector3::dot_product(const Vector3& vec)
{

	return x*vec.x+y*vec.y+z*vec.z;
}

Vector3 Vector3::cross_product(const Vector3& vec)
{
	float ni=y*vec.z-z*vec.y;
	float nj= z * vec.x - x * vec.z;
	float nk= x * vec.y - y * vec.x;

	return Vector3(ni,nj,nk);
}

Vector3 Vector3::normalization()
{
	Vector3 vecReturn;
	vecReturn.x = x / magnitude();
	vecReturn.y = y / magnitude();
	vecReturn.z = z / magnitude();

	if (x == 0) {
		vecReturn.x = 0;
	}
	if (y == 0) {
		vecReturn.y = 0;
	}
	if (z == 0) {
		vecReturn.z = 0;
	}
	return vecReturn;
}

float Vector3::square()
{
	return x*x+y*y+z*z;
}

float Vector3::distance(const Vector3& vec)
{
	float X = x - vec.x;
	float Y = y - vec.y;
	float Z = z - vec.z;
	return sqrt(((X*X)+(Y*Y)+(Z*Z)));
}

float Vector3::magnitude()
{
	return sqrt(x * x + y * y + z * z);
}

bool Vector3::operator!=(const Vector3& vec)
{
	if (x == vec.x && y == vec.y && z == vec.z) {
		return true;
	}
	return false;
}
