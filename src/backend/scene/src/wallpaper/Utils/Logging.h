#ifndef WALLPAPER_LOG
#define WALLPAPER_LOG

#define __SHORT_FILE__ __FILE__
#if 1
#undef __SHORT_FILE__
#define __SHORT_FILE__ ({constexpr const char*const p = past_last_slash(__FILE__); p;})
static constexpr const char* const past_last_slash(const char* const path, const int pos=0, const int last_slash=0) {
	if(path[pos] == '\0') return &path[last_slash];
	if(path[pos] == '/') return past_last_slash(path, pos+1, pos+1);
	else return past_last_slash(path, pos+1, last_slash);
}
#endif 

enum { 
	LOGLEVEL_INFO = 0,
	LOGLEVEL_ERROR
};

#define LOG_INFO(...) WallpaperLog(LOGLEVEL_INFO, __SHORT_FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) WallpaperLog(LOGLEVEL_ERROR, __SHORT_FILE__, __LINE__, __VA_ARGS__)

void WallpaperLog(int level, const char *file, int line, const char *fmt, ...);

#endif

