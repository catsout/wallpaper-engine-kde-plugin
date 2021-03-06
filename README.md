## Wallpaper Engine for Kde
A wallpaper plugin integrating [wallpaper engine](https://store.steampowered.com/app/431960/Wallpaper_Engine) into kde wallpaper setting.  
It's simple and small.  

### Known issue
- Web type doesn't support mouse focus.  
- Some case scene wallpaper may crash your kde.  
  Remove `WallpaperFilePath` line in `~/.config/plasma-org.kde.plasma.desktop-appletsrc` and restart kde to fix.  

### Scene support status
Scene wallpaper is supportted by direct opengl(3.1).  
It's almost usable.  
#### performance
I'm not vary familiar with graphic programming, so don't expect high performance.  
- 4k wallpaper with complex effects need at least GTX1050
- 80Mb+ wallpaper may require 1GB+ VRAM at now
#### not work
- Particle System   
- Scene script  
- Camera shake and zoom  
- Global bloom effect  
- Perspective renderable  
- ColorBlendMode  

### Note
- Support **scene(2d)**,**video**,**web** types
- Compile needed for scene type and mpv video backend, also ok for using without compiling.
- Only test on x11.
- Need Qt >= 5.13, for loop video support without black screen.
- The code is just work, the performance may be slow.  

You need to choose your steam library directory. Like `~/.local/share/Steam`  

### How to use:
1. Use steam+proton or wine+steam
2. Buy and install wallpaper engine
3. Subscribe some wallpapers  
4. Select steam library dir(like .local/share/Steam) in plugin
5. Enjoy


### Install
```sh
git clone https://github.com/catsout/wallpaper-engine-kde-plugin.git
plasmapkg2 -i wallpaper-engine-kde-plugin/plugin
```
### Uninstall
```sh
plasmapkg2 -r wallpaper-engine-kde-plugin/plugin
```

### Compile c++ part
```sh
mkdir build && cd build
cmake ..
make
sudo make install
```

### Better performance for intel card
May not greatly improve.  
#### hwdecode
Qt using GStreamer for video backend.  
You need make your system gstreamer have vaapi plugin installed.  
on debian using `apt install gstreamer1.0-vaapi`.   

#### egl
put `export QT_XCB_GL_INTEGRATION=xcb_egl` to where the xorg-sever will read.  
on debian `~/.xsessionrc` is ok.  

##### kwin display wrong
egl may break kwin.  
use `export QT_XCB_GL_INTEGRATION=xcb_glx kwin --replace`.

#### use mpv 
need compile c++ plugin.

### Credits

- [Smart Video Wallpaper](https://store.kde.org/p/1316299/)     
- [RePKG](https://github.com/notscuffed/repkg)                  
- [linux-wallpaperengine](https://github.com/Almamu/linux-wallpaperengine)                                                                                                                                                                                                                                             

### Preview
![](https://cdn.pling.com/img/a/6/b/6/5a487e675a82563f53d1121eadc3a8d9a0161f8aecfc3a45ae8a413dc0c78baa6518.jpg)
