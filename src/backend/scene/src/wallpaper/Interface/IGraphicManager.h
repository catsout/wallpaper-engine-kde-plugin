#pragma once
#include "Handle.h"
#include "Scene/Scene.h"
#include "Image.h"
#include "Type.h"

namespace wallpaper
{

struct TextureHandle { uint16_t idx {Uninitialed}; };
struct RenderTargetHandle { uint16_t idx {Uninitialed}; };

constexpr uint8_t MaxAttachmentNum {8};

struct HwTexHandle { uint32_t idx {0};
	bool operator==(const HwTexHandle& r) const {
		return idx == r.idx;
	}
	static bool IsInvalied(const HwTexHandle& h) { return h.idx == 0; }
};
struct HwRenderTargetHandle { uint32_t idx {0}; 
	bool operator==(const HwTexHandle& r) const {
		return idx == r.idx;
	}
	static bool IsInvalied(const HwRenderTargetHandle& h) { return h.idx == 0; }
};
struct HwShaderHandle { uint32_t idx {0}; };

class IGraphicManager {
public:
	IGraphicManager() = default;
	virtual ~IGraphicManager() = default;
	IGraphicManager(const IGraphicManager&) = delete;
	IGraphicManager(IGraphicManager&&) = delete;

	virtual bool Initialize() { return true; }
	virtual bool Initialize(void *get_proc_address(const char*)) { return true; }
	virtual void Destroy() {}
	virtual void Draw() {}
	virtual void InitializeScene(Scene*) {}

	struct TextureDesc {
		uint16_t width {2};
		uint16_t height {2};
    	uint16_t numMips {0};
		TextureType type {TextureType::IMG_2D};
		TextureFormat format {TextureFormat::RGBA8};
		TextureSample sample;
	};
	virtual HwTexHandle CreateTexture(TextureDesc) { return {}; }
	virtual HwTexHandle CreateTexture(const Image&) { return {}; }
	struct RenderTargetDesc {
		uint16_t width {1};
		uint16_t height {1};
		std::array<HwTexHandle, MaxAttachmentNum> attachs;
	};
	virtual void ClearTexture(HwTexHandle thandle, std::array<float, 4> clearcolors) {};
	virtual HwRenderTargetHandle CreateRenderTarget(RenderTargetDesc) { return {}; }
	virtual void DestroyTexture(HwTexHandle) {}
	virtual void DestroyRenderTarget(HwRenderTargetHandle) {}


	virtual void SetFlip(bool xflip, bool yflip) {};
};
}
