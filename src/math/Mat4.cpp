#include "Mat4.hpp"

Mat4::Mat4() : Mat4(0.0f) {}

Mat4::Mat4(float diagonal)
{
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			m[row][column] = (row == column) ? diagonal : 0.0f;
		}
	}
}

const float* Mat4::toColumnMajor() const
{
    static float columnMajor[16];

    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            columnMajor[column * 4 + row] = m[row][column];
        }
    }

    return columnMajor;
}

float* Mat4::operator[](int index)
{
    return m[index];
}

const float* Mat4::operator[](int index) const
{
    return m[index];
}

Mat4 Mat4::identity()
{
	return Mat4(1.0f);
}

Mat4 Mat4::translate(const Vec3& position)
{
	Mat4 result(1.0f);
	result[0][3] = position.x;
	result[1][3] = position.y;
	result[2][3] = position.z;
	return result;
}

Mat4 Mat4::scale(const Vec3& scale)
{
	Mat4 result(1.0f);
	result[0][0] = scale.x;
	result[1][1] = scale.y;
	result[2][2] = scale.z;
	return result;
}

Mat4 Mat4::rotate(float angle, const Vec3& axis)
{
    Vec3 a = axis.normalize();

    const float x = a.x;
    const float y = a.y;
    const float z = a.z;

    const float c = std::cos(angle);
    const float s = std::sin(angle);
    const float t = 1.0f - c;

    Mat4 result;

    result[0][0] = t * x * x + c;
    result[0][1] = t * x * y - s * z;
    result[0][2] = t * x * z + s * y;
    result[0][3] = 0.0f;

    result[1][0] = t * x * y + s * z;
    result[1][1] = t * y * y + c;
    result[1][2] = t * y * z - s * x;
    result[1][3] = 0.0f;

    result[2][0] = t * x * z - s * y;
    result[2][1] = t * y * z + s * x;
    result[2][2] = t * z * z + c;
    result[2][3] = 0.0f;

    result[3][0] = 0.0f;
    result[3][1] = 0.0f;
    result[3][2] = 0.0f;
    result[3][3] = 1.0f;

    return result;
}

Mat4 Mat4::rotationX(float angle)
{
	Mat4 result = Mat4::identity();
	const float c = std::cos(angle);
	const float s = std::sin(angle);

	result[1][1] = c;
	result[1][2] = -s;
	result[2][1] = s;
	result[2][2] = c;
	return result;
}

Mat4 Mat4::rotationY(float angle)
{
	Mat4 result = Mat4::identity();
	const float c = std::cos(angle);
	const float s = std::sin(angle);

	result[0][0] = c;
	result[0][2] = s;
	result[2][0] = -s;
	result[2][2] = c;
	return result;
}

Mat4 Mat4::rotationZ(float angle)
{
	Mat4 result = Mat4::identity();
	const float c = std::cos(angle);
	const float s = std::sin(angle);

	result[0][0] = c;
	result[0][1] = -s;
	result[1][0] = s;
	result[1][1] = c;
	return result;
}

Mat4 Mat4::model(
    const Vec3& trans,
    float angle,
    const Vec3& axis,
    const Vec3& scale)
{
    Mat4 T = Mat4::translate(trans);
    Mat4 R = Mat4::rotate(angle, axis);
    Mat4 S = Mat4::scale(scale);

    return T * R * S;
}

Mat4 Mat4::perspective(float fov, float aspect, float nearPlane, float farPlane)
{
	Mat4 result;
	const float tanHalfFov = std::tan(fov * 0.5f);

	result[0][0] = 1.0f / (aspect * tanHalfFov);
	result[1][1] = 1.0f / tanHalfFov;
	result[2][2] = (farPlane + nearPlane) / (nearPlane - farPlane);
	result[2][3] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
	result[3][2] = -1.0f;
	return result;
}

// Mat4 Mat4::orthographic(
// 	float left,
// 	float right,
// 	float bottom,
// 	float top,
// 	float nearPlane,
// 	float farPlane
// )
// {
// 	Mat4 result = Mat4::identity();

// 	result[0][0] = 2.0f / (right - left);
// 	result[1][1] = 2.0f / (top - bottom);
// 	result[2][2] = -2.0f / (farPlane - nearPlane);

// 	result[0][3] = -(right + left) / (right - left);
// 	result[1][3] = -(top + bottom) / (top - bottom);
// 	result[2][3] = -(farPlane + nearPlane) / (farPlane - nearPlane);

// 	return result;
// }

Mat4 Mat4::lookAt(const Vec3& position, const Vec3& target, const Vec3& worldUp)
{
    const Vec3 cameraDirection = (position - target).normalize();
    const Vec3 cameraRight = worldUp.cross(cameraDirection).normalize();
    const Vec3 cameraUp = cameraDirection.cross(cameraRight);

    Mat4 result = Mat4::identity();

    result[0][0] = cameraRight.x;
    result[0][1] = cameraRight.y;
    result[0][2] = cameraRight.z;
    result[0][3] = -cameraRight.dot(position);

    result[1][0] = cameraUp.x;
    result[1][1] = cameraUp.y;
    result[1][2] = cameraUp.z;
    result[1][3] = -cameraUp.dot(position);

    result[2][0] = cameraDirection.x;
    result[2][1] = cameraDirection.y;
    result[2][2] = cameraDirection.z;
    result[2][3] = -cameraDirection.dot(position);

    return result;
}

// Mat4 Mat4::transpose() const
// {
// 	Mat4 result;
// 	for (int row = 0; row < 4; ++row) {
// 		for (int col = 0; col < 4; ++col) {
// 			result[row][col] = m[col][row];
// 		}
// 	}
// 	return result;
// }

// Mat4 Mat4::inverse() const
// {
// 	float aug[4][8];

// 	for (int row = 0; row < 4; ++row) {
// 		for (int col = 0; col < 4; ++col) {
// 			aug[row][col] = m[row][col];
// 			aug[row][col + 4] = (row == col) ? 1.0f : 0.0f;
// 		}
// 	}

// 	for (int pivot = 0; pivot < 4; ++pivot) {
// 		int bestRow = pivot;
// 		for (int row = pivot + 1; row < 4; ++row) {
// 			if (std::fabs(aug[row][pivot]) > std::fabs(aug[bestRow][pivot])) {
// 				bestRow = row;
// 			}
// 		}

// 		if (std::fabs(aug[bestRow][pivot]) < 1e-8f) {
// 			return Mat4::identity();
// 		}

// 		if (bestRow != pivot) {
// 			for (int col = 0; col < 8; ++col) {
// 				const float tmp = aug[pivot][col];
// 				aug[pivot][col] = aug[bestRow][col];
// 				aug[bestRow][col] = tmp;
// 			}
// 		}

// 		const float pivotValue = aug[pivot][pivot];
// 		for (int col = 0; col < 8; ++col) {
// 			aug[pivot][col] /= pivotValue;
// 		}

// 		for (int row = 0; row < 4; ++row) {
// 			if (row == pivot) {
// 				continue;
// 			}
// 			const float factor = aug[row][pivot];
// 			for (int col = 0; col < 8; ++col) {
// 				aug[row][col] -= factor * aug[pivot][col];
// 			}
// 		}
// 	}

// 	Mat4 result;
// 	for (int row = 0; row < 4; ++row) {
// 		for (int col = 0; col < 4; ++col) {
// 			result[row][col] = aug[row][col + 4];
// 		}
// 	}

// 	return result;
// }

Mat4 Mat4::operator*(const Mat4& other) const
{
	Mat4 result;
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			float sum = 0.0f;
			for (int k = 0; k < 4; ++k) {
				sum += m[row][k] * other.m[k][column];
			}
			result[row][column] = sum;
		}
	}
	return result;
}

Vec4 Mat4::operator*(const Vec4& v) const
{
	return Vec4(
		m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
		m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
		m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
		m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
	);
}
