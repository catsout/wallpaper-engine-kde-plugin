#ifndef WALLPAPER_LOG
#define WALLPAPER_LOG


enum { 
	LOGLEVEL_INFO = 0,
	LOGLEVEL_ERROR
};

#define LOG_INFO(...) WallpaperLog(LOGLEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) WallpaperLog(LOGLEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

void WallpaperLog(int level, const char *file, int line, const char *fmt, ...);

#endif

