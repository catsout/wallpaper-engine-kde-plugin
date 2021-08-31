#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "Interface/IGraphicManager.h"
#include "TextureResource.h"
#include "Utils/Hash.h"
#include "Type.h"

namespace wallpaper
{
namespace fg {

enum class TexUsage {
	COLOR,
	DEPTH
};

struct TextureKey {
	uint16_t width;
	uint16_t height;
	TexUsage usage;
	TextureFormat format;
	TextureSample sample;

	static std::size_t HashValue(const TextureKey& k) {
		std::size_t seed {0};
		utils::hash_combine(seed, k.width);
		utils::hash_combine(seed, k.height);
		utils::hash_combine_fast(seed, (int)k.usage);
		utils::hash_combine_fast(seed, (int)k.format);

		utils::hash_combine_fast(seed, (int)k.sample.wrapS);
		utils::hash_combine_fast(seed, (int)k.sample.wrapT);
		utils::hash_combine_fast(seed, (int)k.sample.magFilter);
		return seed;
	}
};

class TextureCache {
public:
	TextureCache() {
		m_unused.reserve(512);
	};
	~TextureCache() = default;
	TextureCache(const TextureCache&) = delete;
	TextureCache& operator=(const TextureCache&) = delete;
	TextureCache(TextureCache&& o):m_inused(std::move(o.m_inused)),
									m_unused(std::move(o.m_unused)) {}

	TextureCache& operator=(TextureCache&& o){
		m_inused = (std::move(o.m_inused));
		m_unused = (std::move(o.m_unused));
		return *this;
	}

	typedef std::size_t TexHash;

	HwTexHandle Query(const TextureResource::Desc& desc, IGraphicManager& gm) {
		TexHash hash = TextureKey::HashValue(TextureKey{
			.width = desc.width,
			.height = desc.height,
			.format = desc.format,
			.sample = desc.sample
		});

		{
			HwTexHandle h = ChangeToInUse(hash);
			if(!HwTexHandle::IsInvalied(h)) {
				//gm.ClearTexture(h, {0,0,0,1.0f});  heigh gpu usage
				return h;
			}
		}

		IGraphicManager::TextureDesc gmdesc {
			.width = desc.width,
			.height = desc.height,
			.type = TextureType::IMG_2D,
			.format = desc.format,
			.sample = desc.sample
		};
		HwTexHandle h = gm.CreateTexture(gmdesc);
		m_inused.insert({h, hash});
		return h;
	};

	HwTexHandle Query(const Image& img, IGraphicManager& gm) {
		TexHash hash = TextureKey::HashValue(TextureKey{
			.width = (uint16_t)img.width,
			.height = (uint16_t)img.height,
			.format = img.format
		});
		// not use unused as image
		HwTexHandle h = gm.CreateTexture(img);
		m_inused.insert({h, hash});
		assert(h.idx != 65537);
		return h;
	}

	bool Release(HwTexHandle& inh) {
		do {
			HwTexHandle h = inh;
			inh = HwTexHandle();
			if(m_inused.count(h) == 0) break;

			TexHash hash = m_inused.at(h);
			m_inused.erase(h);
			m_unused.push_back({hash, h});
			return true;
		} while(false);
		return false;
	};

	void ClearUnused(IGraphicManager& gm) {
		for(const auto& el:m_unused) {
			gm.DestroyTexture(el.second);
		}
		m_unused.clear();
	}
private:
	HwTexHandle ChangeToInUse(TexHash hash) {
		HwTexHandle h;
		for(auto it=m_unused.begin();it<m_unused.end();it++) {
			if(hash == it->first) {
				h = it->second;
				m_unused.erase(it);
				m_inused.insert({h, hash});
				return h;
			}
		}
		return h;
	}
	struct HwTexHandleHasher {
		std::size_t operator()(const HwTexHandle& k) const { return k.idx; }
	};
	std::unordered_map<HwTexHandle,TexHash,HwTexHandleHasher> m_inused;
	std::vector<std::pair<TexHash,HwTexHandle>> m_unused;
};

}
}
