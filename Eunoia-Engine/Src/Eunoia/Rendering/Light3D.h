#pragma once

#include "../Math/Math.h"

namespace Eunoia {

	EU_REFLECT()
	enum Light3DType
	{
		LIGHT3D_DIRECTIONAL,
		LIGHT3D_POINT,
		LIGHT3D_SPOTLIGHT,

		NUM_LIGHT3D_TYPES
	};

	EU_REFLECT()
	struct Attenuation
	{
		Attenuation() :
			constant(0.0f), linear(0.0f), quadratic(1.0f)
		{}

		Attenuation(r32 constant, r32 linear, r32 quadratic) :
			constant(constant),
			linear(linear),
			quadratic(quadratic)
		{}

		EU_PROPERTY() r32 constant;
		EU_PROPERTY() r32 linear;
		EU_PROPERTY() r32 quadratic;
	};

	EU_REFLECT()
	struct ShadowInfo
	{
		ShadowInfo()
		{}

		m4 projection;
		b32 castShadow;
		quat directionalRot;
	};

	EU_REFLECT()
	struct Light3D
	{
		Light3D(Light3DType type, const v3& pos, const v3& direction, const v4& colorAndIntensity, const Attenuation& attenuation, const ShadowInfo& shadowInfo) :
			type(type),
			pos(pos),
			direction(direction),
			colorAndIntensity(colorAndIntensity),
			attenuation(attenuation),
			shadowInfo(shadowInfo)
		{}

		Light3D()
		{}

		Light3DType type;
		v3 pos;
		v3 direction;
		v4 colorAndIntensity;
		Attenuation attenuation;
		ShadowInfo shadowInfo;
	};

}