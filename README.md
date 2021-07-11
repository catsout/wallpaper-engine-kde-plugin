## Wallpaper Engine for Kde
A wallpaper plugin integrating [wallpaper engine](https://store.steampowered.com/app/431960/Wallpaper_Engine) into kde wallpaper setting.  
It's simple and small.  

### Known issue
- Some scene wallpapers may crash your kde.  
  Remove `WallpaperFilePath` line in `~/.config/plasma-org.kde.plasma.desktop-appletsrc` and restart kde to fix.  
- Mouse long press(to enter panel edit mode) is broken on desktop  
### Note
- Support **scene(2d)**,**video**,**web** types
- Scene,mpv support require plugin lib, which need to be compiled   
The lib will be autodetected after installed

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
Need restart plasma(re-login) after update  
Or try: `kquitapp5 plasmashell && kstart5 plasmashell`  

### Plugin lib
#### Dependencies
Debian:  
```sh
sudo apt install liblz4-dev qtbase5-private-dev qtbase5-dev qtdeclarative5-dev libqt5x11extras5-dev libmpv-dev  qt5-default 
```  

Arch:
```sh
sudo pacman -S base-devel mpv qt5-declarative
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
2. Buy and install wallpaper engine(not run)
3. Subscribe some wallpapers  
4. Select steam library dir(like .local/share/Steam) in plugin
5. Enjoy

### Scene support status
Scene wallpaper is supported by direct opengl(3.2).  
It's almost usable.  
#### not work
- 3D model
- Timeline animations
- Puppet warp
- Scene script  
- Text layer  
- Audio play
- PBR light
- Camera shake,fade,~~zoom,parallax~~
- Global bloom effect  
- ~~Perspective renderable~~  
- ~~Particle System~~  
- ~~Gif scene~~
- ~~ColorBlendMode~~  

### Web support status
Basic web apis are supported, but the audio api dose not send data at now.  
#### no webgl
WebEngineView in plasmashell can't init opengl.  
Some wallpaper using webgl may not work, and performance may be bad.   

### HWdecode for video
#### QtMultimedia
Default video backend of this plugin.  
It's using GStreamer to play video.  
To enable hwdecode:  
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
![](https://cdn.pling.com/img/f/b/9/f/63f1672d628422f92fd189fe55f60ee8c9f911a691d0745eeaf51d2c6fae6763b8f8.jpg)
![](https://cdn.pling.com/img/d/7/9/f/c28d236408e66ba3cbca5173fb0bf4362b9df45e6e1c485deb6d9f7b4fe6adf93a2b.jpg)
![](https://cdn.pling.com/img/0/e/e/9/23b2aefba63630c7eb723afc202cdaaa2809d32d8a2ddca03b9fec8f82de62d721cd.jpg)
