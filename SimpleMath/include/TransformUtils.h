#pragma once
#include "SimpleMath.h"

typedef DirectX::SimpleMath::Vector4  float4;
typedef DirectX::SimpleMath::Matrix   matrix;
typedef DirectX::SimpleMath::Matrix   float4x4;
typedef DirectX::SimpleMath::Vector3  float3;
typedef DirectX::SimpleMath::Vector2  float2;
typedef DirectX::SimpleMath::Color    color;
typedef uint32_t uint;

typedef DirectX::SimpleMath::Vector4 Vector4;
typedef DirectX::SimpleMath::Matrix  Matrix ;
typedef DirectX::SimpleMath::Vector3 Vector3;
typedef DirectX::SimpleMath::Vector2 Vector2;
typedef DirectX::SimpleMath::Color   Color;
typedef uint32_t uint;


struct RenderData{
	float  time;
	matrix view;
	matrix projection;
	float4 viewPosition;
};


class Transform {
	matrix transform;

public:

	Transform();
	Transform(const matrix& transform);
	Transform(float3 position, float3 roitation, float3 scale);
	matrix GetTransform() const;
	matrix GetInverseTransform() const;
	Transform operator*(const Transform& rhs);
};


class UITransform {
	float2 shift;
	float2 wh;
public:
	UITransform();
	UITransform(float left, float top, float width, float height);
};

 