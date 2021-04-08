## Wallpaper Engine for Kde
A wallpaper plugin integrating [wallpaper engine](https://store.steampowered.com/app/431960/Wallpaper_Engine) into kde wallpaper setting.  
It's simple and small.  

### Known issue
- Some case scene wallpaper may crash your kde.  
  Remove `WallpaperFilePath` line in `~/.config/plasma-org.kde.plasma.desktop-appletsrc` and restart kde to fix.  

### Note
- Support **scene(2d)**,**video**,**web** types
- Scene,mpv support require plugin lib, which need to be compiled   
The lib will be autodetect after install

You need to choose your steam library directory(where *wallpaper engine* installed).  
Like `~/.local/share/Steam`  

### Kde plugin
#### Dependencies
qt-labs-folderlistmodel  
qml-module-qtwebchannel
#### Install
```sh
# Install
git clone https://github.com/catsout/wallpaper-engine-kde-plugin.git
plasmapkg2 -i wallpaper-engine-kde-plugin/plugin

# Update
plasmapkg2 -u wallpaper-engine-kde-plugin/plugin

# Uninstall
plasmapkg2 -r wallpaper-engine-kde-plugin/plugin
```
Need restart plasma after update  
Try: `killall plasmashell && kstart5 plasmashell`

### Plugin lib
#### Dependencies
Debian:  
```sh
sudo apt install liblz4-dev qtbase5-private-dev qtbase5-dev qtdeclarative5-dev libqt5x11extras5-dev libmpv-dev  qt5-default 
```
#### Build and install
```sh
cd wallpaper-engine-kde-plugin
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

### How to use:
1. Use steam+proton or wine+steam
2. Buy and install wallpaper engine
3. Subscribe some wallpapers  
4. Select steam library dir(like .local/share/Steam) in plugin
5. Enjoy

### Scene support status
Scene wallpaper is supportted by direct opengl(3.2).  
It's almost usable.  
#### performance
I'm not vary familiar with graphic programming, so don't expect high performance.  
- Most likely 10~20% usage higher than *wallpaper engine* on windows.  
#### not work
- Particle System   
- Scene script  
- Text layer  
- Pure gif scene
- Audio play
- Camera shake and zoom  
- Global bloom effect  
- Perspective renderable  
- ~~ColorBlendMode~~  

### Web support status
Basic web apis are supportted, but the audio api dose not send data at now.  
#### no webgl
WebEngineView in plasmashell can't init opengl.  
Some wallpaper using webgl may not work, and performance may be bad.   

### HWdecode for video
#### QtMultimedia
Default video backend of this plugin.  
It's using GStreamer to play video.  
- GStreamer plugin installed
like `gstreamer1.0-vaapi` in debain
- `GST_VAAPI_ALL_DRIVERS=1`
this needed for vaapi  
put it to where the xserver read.  

#### Mpv
Need to compile plugin lib.  
The config is set to `hwdec=auto`, and not configurable at now.  

### Credits

- [Smart Video Wallpaper](https://store.kde.org/p/1316299/)     
- [RePKG](https://github.com/notscuffed/repkg)                  
- [linux-wallpaperengine](https://github.com/Almamu/linux-wallpaperengine)                                                                                                                                                                                                                                             

### Preview
![](https://cdn.pling.com/img/9/e/6/1/e77349a442e38812aa443a34de2f1488b4611d1f190b7432b081bd18fb6b2e0371eb.jpg)
![](https://cdn.pling.com/img/0/e/e/9/23b2aefba63630c7eb723afc202cdaaa2809d32d8a2ddca03b9fec8f82de62d721cd.jpg)
