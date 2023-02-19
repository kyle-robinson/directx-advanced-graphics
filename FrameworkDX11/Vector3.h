#pragma once
#include<iostream>
#include<math.h>
#include<assert.h>
using namespace std;

/// <summary>
/// created from https://github.com/ThomasMillard123/FGAGC
/// </summary>
class Vector3
{
public:
	Vector3();
	Vector3(float x, float y, float z);
	Vector3(const Vector3& Vec);

	Vector3 operator+(const Vector3& vec);   //addition
	Vector3& operator+=(const Vector3& vec);//assign new result to vector
	Vector3 operator-(const Vector3& vec); //substraction
	Vector3& operator-=(const Vector3& vec);//assign new result to vector
	Vector3 operator*(float value);    //multiplication
	Vector3& operator*=(float value);  //assign new result to vector.
	Vector3 operator/(float value);    //division
	Vector3& operator/=(float value);  //assign new result to vector
	Vector3& operator=(const Vector3& vec);



	float dot_product(const Vector3& vec); //scalar dot_product
	Vector3 cross_product(const Vector3& vec); //cross_product
	Vector3 normalization();   //normalized vector

	 //Scalar operations
	float square(); //gives square of the vector
	float distance(const Vector3& vec); //distance between two vectors
	float magnitude();  //magnitude of the vector

	float x;
	float y;
	float z;

	bool operator !=(const Vector3& vec);

};

