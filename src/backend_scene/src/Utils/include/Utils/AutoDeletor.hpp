#pragma once


#define AUTO_DELETER(name, del_func) wallpaper::AutoDeleter del_##name(del_func);

namespace wallpaper
{
template<typename TDeleter>
struct AutoDeleter {
	AutoDeleter(TDeleter&& del):m_del(std::move(del)) {}
	~AutoDeleter() {
		m_del();
	}
	TDeleter m_del;
};
}