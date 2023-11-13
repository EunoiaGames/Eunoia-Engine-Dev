#include "Math.h"

namespace Eunoia {

	m4 m4::CreateTransformation(const v3& translation, const v3& scale, const quat& rot)
	{
		return CreateTranslation(translation) * (rot.CreateRotationMatrix() * CreateScale(scale));
	}

	m4 m4::CreateView(const v3& cameraPos, const quat& cameraRot)
	{
		return cameraRot.Conjugate().CreateRotationMatrix() * m4::CreateTranslation(cameraPos * -1.0f);
	}

	inline quat v3::ToQuaternionRotation() const
	{
		r32 cosYaw = cos(z * 0.5);
		r32 sinYaw = sin(z * 0.5);
		r32 cosPitch = cos(y * 0.5);
		r32 sinPitch = sin(y * 0.5);
		r32 cosRoll = cos(x * 0.5);
		r32 sinRoll = sin(x * 0.5);

		return quat(sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw,
					cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,
					cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw,
					cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw);
	}

	v3 v3::Rotate(const quat& rot) const
	{
		quat conj = rot.Conjugate();
		quat w = rot * (*this) * conj;
		return v3(w.x, w.y, w.z);
	}

}