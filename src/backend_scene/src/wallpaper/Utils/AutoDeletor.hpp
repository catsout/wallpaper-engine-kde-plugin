#pragma once

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

#define AUTO_DELETER(name, del_func) AutoDeleter del_##name(del_func);
}