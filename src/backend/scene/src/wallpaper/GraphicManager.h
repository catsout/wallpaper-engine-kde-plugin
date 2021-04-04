#pragma once
#include "Scene.h"

namespace wallpaper
{

class GraphicManager {
public:
	GraphicManager() {}
	virtual ~GraphicManager() {}
	virtual bool Initialize() { return true; }
	virtual bool Initialize(void *get_proc_address(const char*)) { return true; }
	virtual void Destroy() {}
	virtual void Draw() {}
	virtual void InitializeScene(Scene*) {}
};
}
