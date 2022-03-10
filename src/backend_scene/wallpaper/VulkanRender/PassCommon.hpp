#pragma once
#include "Vulkan/Instance.hpp"
#include "Type.h"

namespace wallpaper
{
namespace vulkan
{
inline void SetBlend(BlendMode bm, vk::PipelineColorBlendAttachmentState& state) {
	state.setBlendEnable(true);
	state.setColorBlendOp(vk::BlendOp::eAdd)
		.setAlphaBlendOp(vk::BlendOp::eAdd);
	switch (bm)
	{
	case BlendMode::Disable:
		state.setBlendEnable(false);
		break;
	case BlendMode::Normal:
		state.setSrcColorBlendFactor(vk::BlendFactor::eOne)
			.setDstColorBlendFactor(vk::BlendFactor::eZero)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
		break;
	case BlendMode::Translucent:
		state.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
		//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BlendMode::Additive:
		state.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOne)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstAlphaBlendFactor(vk::BlendFactor::eOne);
		break;
	}
}
}
}