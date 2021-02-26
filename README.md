## Wallpaper Engine for Kde
A wallpaper plgin integrating [wallpaper engine](https://store.steampowered.com/app/431960/Wallpaper_Engine) into kde wallpaper setting.  
It's simple and small.  

### Warning for scene support
Scene support is unstable, may easily crash your kde.   
Backup your `~/.config/plasma-org.kde.plasma.desktop-appletsrc` file, before using scene wallpaper.  
And restore it when scene wallpaper break your kde.  

### Known issue
- With web type, desktop right click not work.  
  Need hold leftclick on the desktop to get `Configure Desktop and Wallpaper` 

### Note
- Support **scene(2d)**,**video**,**web** types
- Compile needed for scene type and mpv video backend, also ok for using without compiling.
- Only test on x11.
- Need Qt >= 5.13, for loop video support without black screen.
- The code is just work, the performance may be slow.  

You need to choose your steam library directory. Like `~/.local/share/Steam`  

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


![](https://cdn.pling.com/img/e/8/d/f/b0c358d344d8e9132cb09f08c7d83d02563f959cbc754ad7446b2ac6e017d61d0ede.png)
