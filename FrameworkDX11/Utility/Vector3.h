#pragma once
#ifndef VECTOR3_H
#define VECTOR3_H

class Vector3
{
public:
	Vector3();
	Vector3( float x, float y, float z );
	Vector3( const Vector3& Vec );

	Vector3 operator + ( const Vector3& vec );
	Vector3& operator += ( const Vector3& vec );
	Vector3 operator - ( const Vector3& vec );
	Vector3& operator -= ( const Vector3& vec );
	Vector3 operator * ( float value );
	Vector3& operator *= ( float value );
	Vector3 operator / ( float value );
	Vector3& operator /= ( float value );
	Vector3& operator = ( const Vector3& vec );
	bool operator !=( const Vector3& vec );

	float dot_product( const Vector3& vec );
	Vector3 cross_product( const Vector3& vec );
	Vector3 normalization();

	float square();
	float distance( const Vector3& vec );
	float magnitude();

	float x, y, z;
};

#endif