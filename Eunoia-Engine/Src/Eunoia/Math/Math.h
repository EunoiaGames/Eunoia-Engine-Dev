#pragma once

#include "../Common.h"
#include "GeneralMath.h"
#include <cmath>

namespace Eunoia
{

	EU_REFLECT()
	struct v2
	{
		v2(r32 x, r32 y) :
			x(x), y(y) {}

		v2() :
			x(0.0f), y(0.0f) {}

		inline v2 Min(const v2& vec) const { return v2(EU_MIN(x, vec.x), EU_MIN(y, vec.y)); }
		inline v2 Max(const v2& vec) const { return v2(EU_MAX(x, vec.x), EU_MAX(y, vec.y)); }

		inline v2 operator*(r32 scaler) const { return { x * scaler, y * scaler }; }
		inline v2 operator*(const v2& vec) const { return { x * vec.x, y * vec.y }; }
		inline v2 operator+(const v2& vec) const { return { x + vec.x, y + vec.y }; }
		inline v2 operator-(const v2& vec) const { return { x - vec.x, y - vec.y }; }

		inline v2& operator+=(const v2& vec) { x += vec.x; y += vec.y; return *this; }
		inline v2& operator-=(const v2& vec) { x -= vec.x; y -= vec.y; return *this; }
		inline v2& operator*=(r32 scaler) { x *= scaler; y *= scaler; return *this; }
		inline v2& operator*=(const v2& vec) { x *= vec.x; y *= vec.y; return *this; }

		EU_PROPERTY() r32 x;
		EU_PROPERTY() r32 y;
	};

	struct quat;
	EU_REFLECT()
	struct v3
	{
		v3(r32 x, r32 y, r32 z) :
			x(x), y(y), z(z) {}

		v3(const v2& xy, r32 z) :
			x(xy.x), y(xy.y), z(z)
		{}

		v3() :
			x(0.0f), y(0.0f), z(0.0f)
		{}

		inline v2 xy() const { return { x, y }; }
		inline v2 yz() const { return { y, z }; }
		inline v2 xz() const { return { x, z }; }

		inline r32 Dot(const v3& vec) const { return x * vec.x + y * vec.y + z * vec.z; }
		inline r32 Length() const { return sqrt(Dot(*this)); }
		inline v3 Cross(const v3& vec) const { return v3(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x); }

		inline v3 Lerp(const v3& vec, r32 t) const { return v3(EU_LERP(x, vec.x, t), EU_LERP(y, vec.y, t), EU_LERP(z, vec.z, t)); }

		inline v3 Min(const v3& vec) const { return v3(EU_MIN(x, vec.x), EU_MIN(y, vec.y), EU_MIN(z, vec.z)); }
		inline v3 Max(const v3& vec) const { return v3(EU_MAX(x, vec.x), EU_MAX(y, vec.y), EU_MAX(z, vec.z)); }
		inline v3 Clamp(const v3& min, const v3& max) { return v3(EU_CLAMP(min.x, max.x, x), EU_CLAMP(min.y, max.y, y), EU_CLAMP(min.z, max.z, z)); }

		EU_API inline quat ToQuaternionRotation() const;

		inline v3 Normalized() const
		{
			r32 len = Length();
			return v3(x / len, y / len, z / len);
		}

		inline v3 Rotate(const v3& axis, r32 deg) const
		{
			r32 rad = EU_DEG_TO_RAD(deg);
			r32 sinAngle = sinf(-rad);
			r32 cosAngle = cosf(-rad);
			return Cross(axis * sinAngle) + (*this * cosAngle) + (axis * Dot(axis * (1.0f - cosAngle)));
		}

		EU_API v3 Rotate(const quat& rot) const;

		inline v3 operator*(r32 scaler)	   const { return v3(x * scaler, y * scaler, z * scaler); }
		inline v3 operator*(const v3& vec) const { return v3(x * vec.x, y * vec.y, z * vec.z); }
		inline v3 operator+(const v3& vec) const { return v3(x + vec.x, y + vec.y, z + vec.z); }
		inline v3 operator-(const v3& vec) const { return v3(x - vec.x, y - vec.y, z - vec.z); }

		inline v3& operator+=(const v3& vec) { x += vec.x; y += vec.y; z += vec.z; return *this; }
		inline v3& operator-=(const v3& vec) { x -= vec.x; y -= vec.y; z -= vec.z; return *this; }
		inline v3& operator*=(r32 scaler) { x *= scaler; y *= scaler; z *= scaler; return *this; }
		inline v3& operator*=(const v3& vec) { x *= vec.x; y *= vec.y; z *= vec.z; return *this; }


		inline bool operator==(const v3& vec) const { return x == vec.x && y == vec.y && z == vec.z; }
		inline bool operator!=(const v3& vec) const { return !operator==(vec); }

		EU_PROPERTY() r32 x;
		EU_PROPERTY() r32 y;
		EU_PROPERTY() r32 z;
	};

	EU_REFLECT()
	struct v4
	{
		v4(r32 x, r32 y, r32 z, r32 w) :
			x(x), y(y), z(z), w(w) {}

		v4(const v3& xyz, r32 w) :
			x(xyz.x), y(xyz.y), z(xyz.z), w(w)
		{}

		v4(const v2& xy, r32 z, r32 w) :
			x(xy.x), y(xy.y), z(z), w(w)
		{}

		v4(r32 xyzw = 0.0f) :
			x(xyzw), y(xyzw), z(xyzw), w(xyzw)
		{}

		inline v2 xy() const { return v2(x, y); }
		inline v3 xyz() const { return v3(x, y, z); }

		EU_PROPERTY() r32 x;
		EU_PROPERTY() r32 y;
		EU_PROPERTY() r32 z;
		EU_PROPERTY() r32 w;
	};

	EU_REFLECT()
	struct m3
	{
		m3()
		{
			for (u32 i = 0; i < 3; i++)
				for (u32 j = 0; j < 3; j++)
					values[i][j] = 0.0f;
		}

		m3(r32 m00, r32 m01, r32 m02,
			r32 m10, r32 m11, r32 m12,
			r32 m20, r32 m21, r32 m22)
		{
			values[0][0] = m00; values[0][1] = m01; values[0][2] = m02;
			values[1][0] = m10; values[1][1] = m11; values[1][2] = m12;
			values[2][0] = m20; values[2][1] = m21; values[2][2] = m22;
		}

		m3(r32 diagnol)
		{
			for (u32 i = 0; i < 3; i++)
				for (u32 j = 0; j < 3; j++)
					if (i == j)
						values[i][j] = diagnol;
					else
						values[i][j] = 0.0f;
		}

		inline m3 Transpose() const { return Transpose(*this); }

		inline const r32* operator[](u32 index) const { return values[index]; }
		inline r32* operator[](u32 index) { return values[index]; }

		inline m3 operator*(const m3& mat) const
		{
			m3 m;
			for (u32 i = 0; i < 3; i++)
			{
				for (u32 j = 0; j < 3; j++)
				{
					m[i][j] = values[i][0] * mat[0][j] +
						values[i][1] * mat[1][j] +
						values[i][2] * mat[2][j];
				}
			}
			return m;
		}

		inline v3 operator*(const v3& vec) const
		{
			return v3(values[0][0] * vec.x + values[0][1] * vec.y + values[0][2] * vec.z,
				values[1][0] * vec.x + values[1][1] * vec.y + values[1][2] * vec.x,
				values[2][0] * vec.x + values[2][1] * vec.y + values[2][2] * vec.x);
		}

		inline v2 operator*(const v2& vec) const
		{
			return v2(values[0][0] * vec.x + values[0][1] * vec.y + values[0][2] * 1.0f,
				values[1][0] * vec.x + values[1][1] * vec.y + values[1][2] * 1.0f);
		}

		inline static m3 CreateIdentity()
		{
			m3 result;
			for (u32 i = 0; i < 3; i++)
				for (u32 j = 0; j < 3; j++)
					if (i == j)
						result[i][j] = 1.0f;
					else
						result[i][j] = 0.0f;
			return result;
		}

		inline static m3 Transpose(const m3& mat)
		{
			m3 result;
			for (u32 i = 0; i < 3; i++)
				for (u32 j = 0; j < 3; j++)
					result[i][j] = mat[j][i];
			return result;
		}

		inline static m3 CreateTranslation(const v2& translation)
		{
			return m3(1.0f, 0.0f, translation.x,
				0.0f, 1.0f, translation.y,
				0.0f, 0.0f, 1.0f);
		}

		inline static m3 CreateScale(const v2& scale)
		{
			return m3(scale.x, 0.0f, 0.0f,
				0.0f, scale.y, 0.0f,
				0.0f, 0.0f, 1.0f);
		}

		inline static m3 CreateRotation(r32 deg)
		{
			r32 rad = EU_DEG_TO_RAD(deg);
			r32 cosRad = cosf(rad);
			r32 sinRad = sinf(rad);

			return m3(cosRad, -sinRad, 0.0f,
				sinRad, cosRad, 0.0f,
				0.0f, 0.0f, 1.0f);
		}

		inline static m3 CreateTransformation(const v2& translation, const v2& scale, r32 deg)
		{
			return m3::CreateTranslation(translation) * (m3::CreateRotation(deg) * m3::CreateScale(scale));
		}

		EU_PROPERTY()
		r32 values[4][4];
	};

	struct quat;
	EU_REFLECT()
	struct m4
	{
		m4()
		{
			for (u32 i = 0; i < 4; i++)
				for (u32 j = 0; j < 4; j++)
					values[i][j] = 0.0f;
		}

		m4(r32 m00, r32 m01, r32 m02, r32 m03,
			r32 m10, r32 m11, r32 m12, r32 m13,
			r32 m20, r32 m21, r32 m22, r32 m23,
			r32 m30, r32 m31, r32 m32, r32 m33)
		{
			values[0][0] = m00; values[0][1] = m01; values[0][2] = m02; values[0][3] = m03;
			values[1][0] = m10; values[1][1] = m11; values[1][2] = m12; values[1][3] = m13;
			values[2][0] = m20; values[2][1] = m21; values[2][2] = m22; values[2][3] = m23;
			values[3][0] = m30; values[3][1] = m31; values[3][2] = m32; values[3][3] = m33;
		}

		m4(r32 diagnol)
		{
			for (u32 i = 0; i < 4; i++)
				for (u32 j = 0; j < 4; j++)
					if (i == j)
						values[i][j] = diagnol;
					else
						values[i][j] = 0.0f;
		}

		inline m4 Transpose() const { return Transpose(*this); }

		inline v3 ExtractPosition() const 
		{
			return v3(values[0][3], values[1][3], values[2][3]);
		}

		inline v3 ExtractScale() const
		{
			v3 c1(values[0][0], values[1][0], values[2][0]);
			v3 c2(values[0][1], values[1][1], values[2][1]);
			v3 c3(values[0][2], values[1][2], values[2][2]);

			return v3(c1.Length(), c2.Length(), c3.Length());
		}

		inline const r32* operator[](u32 index) const { return values[index]; }
		inline r32* operator[](u32 index) { return values[index]; }

		inline m4 operator*(const m4& mat) const
		{
			m4 m;
			for (u32 i = 0; i < 4; i++)
			{
				for (u32 j = 0; j < 4; j++)
				{
					m[i][j] = values[i][0] * mat[0][j] +
						values[i][1] * mat[1][j] +
						values[i][2] * mat[2][j] +
						values[i][3] * mat[3][j];
				}
			}
			return m;
		}

		inline v4 operator*(const v4& vec) const
		{
			return v4(values[0][0] * vec.x + values[0][1] * vec.y + values[0][2] * vec.z + values[0][3] * vec.w,
				values[1][0] * vec.x + values[1][1] * vec.y + values[1][3] * vec.z + values[1][3] * vec.w,
				values[2][0] * vec.x + values[2][1] * vec.y + values[2][3] * vec.z + values[2][3] * vec.w,
				values[3][0] * vec.x + values[3][1] * vec.y + values[3][3] * vec.z + values[3][3] * vec.w);
		}

		inline v3 operator*(const v3& vec) const
		{
			return v3(values[0][0] * vec.x + values[0][1] * vec.y + values[0][2] * vec.z + values[0][3] * 1.0f,
				values[1][0] * vec.x + values[1][1] * vec.y + values[1][3] * vec.x + values[1][3] * 1.0f,
				values[2][0] * vec.x + values[2][1] * vec.y + values[2][3] * vec.x + values[2][3] * 1.0f);
		}

		inline static m4 CreateIdentity()
		{
			m4 result;
			for (u32 i = 0; i < 4; i++)
				for (u32 j = 0; j < 4; j++)
					if (i == j)
						result[i][j] = 1.0f;
					else
						result[i][j] = 0.0f;
			return result;
		}

		inline static m4 CreateOrthographic(r32 left, r32 right, r32 bottom, r32 top, r32 n, r32 f)
		{
			return m4(2.0f / (right - left), 0.0f,					0.0f,			 -(right + left) / (right - left),
					  0.0f,					 2.0f / (top - bottom), 0.0f,			 -(top + bottom) / (top - bottom),
					  0.0f,					 0.0f,					2.0f / (f - n), -(f + n) / (f - n),
					  0.0f,					 0.0f,					0.0f,			 1.0f);

			//return m4(2.0f / (right - left), 0.0f, 0.0f, -(right + left) / (right - left),
			//		0.0f, 2.0f / (bottom - top), 0.0f, -(bottom + top) / (bottom - top),
			//	0.0f, 0.0f, 1.0f / (n - f), n / (n - f),
			//	0.0f, 0.0f, 0.0f, 1.0f);
		}

		inline static m4 CreatePerspective(r32 width, r32 height, r32 fov, r32 znear, r32 zfar)
		{
			r32 ar = width / height;
			r32 tanHalfFov = tanf(EU_DEG_TO_RAD(fov) / 2.0f);
			r32 zrange = znear - zfar;

			return m4(1.0f / (ar * tanHalfFov), 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f / tanHalfFov, 0.0f, 0.0f,
				0.0f, 0.0f, (-znear - zfar) / zrange, 2 * zfar * znear / zrange,
				0.0f, 0.0f, 1.0f, 0.0f);
		}

		inline static m4 CreateTranslation(const v3& translation)
		{
			return m4(1.0f, 0.0f, 0.0f, translation.x,
				0.0f, 1.0f, 0.0f, translation.y,
				0.0f, 0.0f, 1.0f, translation.z,
				0.0f, 0.0f, 0.0f, 1.0f);
		}

		inline static m4 CreateScale(const v3& scale)
		{
			return m4(scale.x, 0.0f, 0.0f, 0.0f,
					  0.0f, scale.y, 0.0f, 0.0f,
					  0.0f, 0.0f, scale.z, 0.0f,
					  0.0f, 0.0f, 0.0f, 1.0f);
		}

		inline static m4 CreateRotationX(r32 deg)
		{
			r32 angle = EU_DEG_TO_RAD(deg);
			r32 cosAngle = cosf(angle);
			r32 sinAngle = sinf(angle);

			return m4(1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, cosAngle, -sinAngle, 0.0f,
				0.0f, sinAngle, cosAngle, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
		}

		inline static m4 CreateRotationY(r32 deg)
		{
			r32 angle = EU_DEG_TO_RAD(deg);
			r32 cosAngle = cosf(angle);
			r32 sinAngle = sinf(angle);

			return m4(cosAngle, 0.0f, sinAngle, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				-sinAngle, 0.0f, cosAngle, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
		}

		inline static m4 CreateRotationZ(r32 deg)
		{
			r32 angle = EU_DEG_TO_RAD(deg);
			r32 cosAngle = cosf(angle);
			r32 sinAngle = sinf(angle);

			return m4(cosAngle, -sinAngle, 0.0f, 0.0f,
				sinAngle, cosAngle, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
		}

		EU_API static m4 CreateTransformation(const v3& translation, const v3& scale, const quat& rot);

		inline static m4 CreateRotation(const v3& axis) { return CreateRotationX(axis.x) * (CreateRotationY(axis.y) * CreateRotationZ(axis.z)); }

		inline static m4 CreateRotation(const v3& forward, const v3& up, const v3& right)
		{
			return m4(right.x, right.y, right.z, 0.0f,
				up.x, up.y, up.z, 0.0f,
				forward.x, forward.y, forward.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
		}

		inline static m4 CreateRotation(const v3& forward, const v3& up)
		{
			v3 right = up.Cross(forward);
			v3 newUp = forward.Cross(right);

			return CreateRotation(forward, newUp, right);
		}

		EU_API static m4 CreateView(const v3& cameraPos, const quat& cameraRot);

		inline static m4 Transpose(const m4& mat)
		{
			m4 result;
			for (u32 i = 0; i < 4; i++)
				for (u32 j = 0; j < 4; j++)
					result[i][j] = mat[j][i];
			return result;
		}

		EU_PROPERTY()
		r32 values[4][4];
	};

	EU_REFLECT()
	struct quat
	{
		EU_PROPERTY() r32 x;
		EU_PROPERTY() r32 y;
		EU_PROPERTY() r32 z;
		EU_PROPERTY() r32 w;

		inline quat(r32 x, r32 y, r32 z, r32 w) :
			x(x), y(y), z(z), w(w)
		{}

		inline quat() :
			x(0.0f), y(0.0f), z(0.0f), w(1.0f)
		{}

		inline quat(const v3& axis, r32 deg)
		{
			r32 halfAngle = EU_DEG_TO_RAD(deg) / 2.0f;
			r32 sinHalfAngle = sinf(halfAngle);
			r32 cosHalfAngle = cosf(halfAngle);

			x = axis.x * sinHalfAngle;
			y = axis.y * sinHalfAngle;
			z = axis.z * sinHalfAngle;
			w = cosHalfAngle;
		}

		inline quat(const m4& rotation)
		{
			w = sqrtf(1.0f + rotation[0][0] + rotation[1][1] + rotation[2][2]) / 2.0f;
			r32 w4 = w * 4.0f;
			x = (rotation[2][1] - rotation[1][2]) / w4;
			y = (rotation[0][2] - rotation[2][0]) / w4;
			z = (rotation[1][0] - rotation[0][1]) / w4;
		}

		inline quat Conjugate() const { return quat(-x, -y, -z, w); }
		inline r32 Dot(const quat& q) const { return x * q.x + y * q.y + z * q.z + w * q.w; }
		inline r32 Length() const { return sqrtf(Dot(*this)); }

		inline quat Normalized() const
		{
			r32 len = Length();
			return quat(x / len, y / len, z / len, w / len);
		}

		inline quat Lerp(const quat& q, r32 t) const { return *this * t + q * (1.0f - t); }

		inline quat Slerp(quat q, r32 t) const
		{
			r32 dot = Dot(q);
			if (dot < 0)
				q = q * -1.0f;

			r32 theta = acosf(dot);

			r32 mult1, mult2;
			if (theta > 0.000001)
			{
				mult1 = sinf((1.0f - t) * theta) / sinf(theta);
				mult2 = sinf(t * theta) / sinf(theta);
			}
			else
			{
				mult1 = 1.0f - t;
				mult2 = t;
			}

			return operator*(mult1) + q * mult2;
		}

		inline v3 ToEulerRotation() const
		{
			//X Rotation
			r32 sinRollCosPitch = 2.0f * (w * x + y * z);
			r32 cosRollCosPitch = 1.0f - 2.0f * (x * x + y * y);
			r32 roll = atan2f(sinRollCosPitch, cosRollCosPitch);
			//Y Rotation
			r32 sinPitch = 2.0f * (w * y - z * x);
			r32 pitch = 0.0f;
			if (EU_ABS(sinPitch) > 1.0f)
				pitch = copysignf(EU_HALF_PI, sinPitch);
			else
				pitch = asinf(sinPitch);
			//Z Rotation
			r32 sinYawCosPitch = 2.0f * (w * z + x * y);
			r32 cosYawCosPitch = 1.0f - 2.0f * (y * y + z * z);
			r32 yaw = atan2f(sinYawCosPitch, cosYawCosPitch);

			return v3(roll, pitch, yaw);
		}

		inline m4 CreateRotationMatrix() const
		{
			v3 forward(2.0f * (x * z - w * y), 2.0f * (y * z + w * x), 1.0f - 2.0f * (x * x + y * y));
			v3 up(2.0f * (x * y + w * z), 1.0f - 2.0f * (x * x + z * z), 2.0f * (y * z - w * x));
			v3 right(1.0f - 2.0f * (y * y + z * z), 2.0f * (x * y - w * z), 2.0f * (x * z + w * y));

			return m4::CreateRotation(forward, up, right);
		}

		inline void Set(r32 x, r32 y, r32 z, r32 w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		inline void Set(const quat& q)
		{
			x = q.x;
			y = q.y;
			z = q.z;
			w = q.w;
		}

		inline v3 GetForward() const { return v3(0, 0, 1).Rotate(*this); }
		inline v3 GetBack() const { return v3(0, 0, -1).Rotate(*this); }
		inline v3 GetUp() const { return v3(0, 1, 0).Rotate(*this); }
		inline v3 GetDown() const { return v3(0, -1, 0).Rotate(*this); }
		inline v3 GetRight() const { return v3(1, 0, 0).Rotate(*this); }
		inline v3 GetLeft() const { return v3(-1, 0, 0).Rotate(*this); }

		inline quat operator+(const quat& q) const { return quat(x + q.x, y + q.y, z + q.z, w + q.w); }
		inline quat operator*(r32 scaler) const { return quat(x * scaler, y * scaler, z * scaler, w * scaler); }

		inline quat operator*(const quat& q) const
		{
			return quat(x * q.w + w * q.x + y * q.z - z * q.y,
				y * q.w + w * q.y + z * q.x - x * q.z,
				z * q.w + w * q.z + x * q.y - y * q.x,
				w * q.w - x * q.x - y * q.y - z * q.z);
		}

		inline quat operator*(const v3& vec) const
		{
			return quat(w * vec.x + y * vec.z - z * vec.y,
				w * vec.y + z * vec.x - x * vec.z,
				w * vec.z + x * vec.y - y * vec.x,
				-x * vec.x - y * vec.y - z * vec.z);
		}

		inline bool operator==(const quat& q) const { return x == q.x && y == q.y && z == q.z && w == q.w; }
		inline bool operator!=(const quat& q) const { return !operator==(q); }
	};

	EU_REFLECT()
	struct Transform2D
	{
		Transform2D(const v2& pos, const v2& scale, r32 rot) :
			pos(pos),
			scale(scale),
			rot(rot)
		{}

		Transform2D() :
			pos(0.0f, 0.0f),
			scale(1.0f, 1.0f),
			rot(0.0f)
		{}

		inline Transform2D operator*(const Transform2D& t) const
		{
			Transform2D res(pos + t.pos, scale * t.scale, fmodf(rot + t.rot, 360.0f));
			return res;
		}

		inline Transform2D& operator*=(const Transform2D& t)
		{
			pos += t.pos;
			scale *= t.scale;
			rot = fmodf(rot + t.rot, 360.0f);
			return *this;
		}

		inline Transform2D& Translate(const v2& translation) { pos += translation; return *this; }
		inline Transform2D& Translate(const v2& dir, r32 amount) { pos += (dir * amount); return *this; }
		inline Transform2D& Scale(const v2& scale) { this->scale *= scale; return *this; }
		inline Transform2D& Scale(r32 scale) { this->scale *= scale; return *this; }
		inline Transform2D& Rotate(r32 rot) { this->rot = fmodf(this->rot + rot, 360.0f); }

		inline m3 CreateTransformMatrix() const { return m3::CreateTransformation(pos, scale, rot); }

		EU_PROPERTY() v2 pos;
		EU_PROPERTY() v2 scale;
		EU_PROPERTY() r32 rot;
	};

	EU_REFLECT()
	struct Transform3D
	{
		Transform3D(const v3& pos, const v3& scale, const quat& rot) :
			pos(pos),
			scale(scale),
			rot(rot)
		{}

		Transform3D() :
			pos(0.0f, 0.0f, 0.0f),
			scale(1.0f, 1.0f, 1.0f),
			rot(0.0f, 0.0f, 0.0f, 1.0f)
		{}

		inline Transform3D operator*(const Transform3D& t) const
		{
			Transform3D res(pos + t.pos, scale * t.scale, (t.rot * rot).Normalized());
			return res;
		}

		inline Transform3D& operator*=(const Transform3D& t)
		{
			pos += t.pos;
			scale *= t.scale;
			rot = (t.rot * rot).Normalized();
			return *this;
		}

		inline Transform3D& Translate(const v3& translation) { pos += translation; return *this; }
		inline Transform3D& Translate(const v3& dir, r32 amount) { pos += (dir * amount); return *this; }
		inline Transform3D& Scale(const v3& scale) { this->scale *= scale; return *this; }
		inline Transform3D& Scale(r32 scale) { this->scale *= scale; return *this; }
		inline Transform3D& Rotate(const quat& rot) { this->rot = (rot * this->rot).Normalized(); return *this; }
		inline Transform3D& Rotate(const v3& axis, r32 deg) { return Rotate(quat(axis, deg)); }

		inline m4 CreateTransformMatrix() const { return m4::CreateTransformation(pos, scale, rot); }

		EU_PROPERTY() v3 pos;
		EU_PROPERTY() v3 scale;
		EU_PROPERTY() quat rot;
	};
}