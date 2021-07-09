#pragma once
#include <cstdint>
#include <vector>
#include <iostream>

namespace wallpaper
{
struct SpriteFrame {
	uint32_t imageId;
	float frametime;
	float x;
	float y;
	float width;
	float height;

	float rate; // real h / w

	float unk0;
	float unk1;
};

class SpriteAnimation {
public:
	const auto& GetAnimateFrame(double newtime) {
		if((m_remainTime -= newtime) < 0.0f) {
			SwitchToNext();
			const auto& frame = m_frames.at(m_curFrame);
			m_remainTime = frame.frametime;
		}
		const auto& frame = m_frames.at(m_curFrame);
		return frame;
	}
	const auto& GetCurFrame() const {
		return m_frames.at(m_curFrame);
	}
	void AppendFrame(const SpriteFrame& frame) {
		m_frames.push_back(frame);
	}

	uint32_t numFrames() const { return m_frames.size(); }
private:
	void SwitchToNext() {
		if(m_curFrame >= m_frames.size() - 1)
			m_curFrame = 0;
		else m_curFrame++;
	}
	int32_t m_curFrame;
	double m_remainTime;
	std::vector<SpriteFrame> m_frames;
};
}
