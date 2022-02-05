#include "Transform.h"

Transform::Transform(matrix transform): transform(transform) {}

Transform::Transform() : transform(matrix::CreateScale(float3{1.0,1.0,1.0})) {}

Transform::Transform(float3 position, float3 roitation, float3 scale):
	transform(
		matrix::CreateScale(scale)*
		matrix::CreateFromYawPitchRoll(roitation.x, roitation.y, roitation.z)*
		matrix::CreateTranslation(position)
	)
{
}

matrix Transform::GetTransform() const {
	return transform.Transpose();
}

matrix Transform::GetInverseTransform()  const {
	return transform.Invert().Transpose();
}

Transform Transform::operator*(const Transform& rhs) {
	return Transform(transform*rhs.transform);
}
