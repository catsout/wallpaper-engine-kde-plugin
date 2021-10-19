# Wallpaper Engine for Kde
A wallpaper plugin integrating [wallpaper engine](https://store.steampowered.com/app/431960/Wallpaper_Engine) into kde wallpaper setting.  

## Note
- Known issue
  - Some scene wallpapers may **crash** your kde.  
    Remove `WallpaperFilePath` line in `~/.config/plasma-org.kde.plasma.desktop-appletsrc` and restart kde to fix.  
  - Mouse long press(to enter panel edit mode) is broken on desktop  
- Support **scene(2d)**,**video**,**web** types
- Require *wallpapaer engine* installed by steam
- Require qt 5.13+ for playing video(no mpv), or use mpv instead  
- This plugin has(not required) a [lib](#plugin-lib) to be compiled, if you want any feature below  
  - scene wallpaper  
  - mpv video backend  
  - mouse input  

## Contents
- [Install](#install)
- [Usage](#usage)
- [Support Status](#support-status)
- [Acknowledgments](#acknowledgments)
- [Preview](#preview)

## Install
### Kde plugin
#### Dependencies
qt-labs-folderlistmodel  
qml-module-qtwebchannel
#### Install kde plugin
```sh
# Install
git clone https://github.com/catsout/wallpaper-engine-kde-plugin.git
plasmapkg2 -i wallpaper-engine-kde-plugin/plugin

# Update
plasmapkg2 -u wallpaper-engine-kde-plugin/plugin

# Uninstall
plasmapkg2 -r wallpaper-engine-kde-plugin/plugin
```

### Plugin lib
#### Dependencies
Debian:  
```sh
sudo apt install liblz4-dev qtbase5-private-dev qtbase5-dev qtdeclarative5-dev libqt5x11extras5-dev libmpv-dev  
```  

Arch:
```sh
sudo pacman -S base-devel mpv qt5-declarative
```
#### Install plugin lib
```sh
cd wallpaper-engine-kde-plugin
mkdir build && cd build
cmake ..
make
sudo make install
```

## Usage
1. Use steam+proton or wine+steam
2. Buy and install wallpaper engine(not run)
3. Subscribe some wallpapers on [workshop](https://steamcommunity.com/app/431960/workshop/)(or the we app, if you can run it)  
4. Let steam download wallpapers
5. Install this plugin, and select steam library dir(where *wallpaper engine* installed) in plugin  
e.g `.local/share/Steam`
6. Enjoy  

### Restart Kde
Need to restart kde(re-login) after **updating plugin** or **reinstalling plugin lib**  
Or try: `kquitapp5 plasmashell && kstart5 plasmashell`  

## Support Status
### Scene
Scene wallpaper is supported by direct opengl(3.2).  
Require *wallpapaer engine* installed for assets(shaders,pictures...)
#### implemented
- [x] Layer
	- [x] image
	- [x] composition / fullscreen
	- [ ] text
- [x] Effect
	- [x] Mouse position
	- [x] Parallax
	- [x] ColorBlendMode
	- [x] PBR light
	- [ ] Global bloom
- [x] Camera
	- [x] zoom
	- [ ] shake
	- [ ] fade / path
- [x] Audio
	- [x] loop
	- [ ] random
	- [ ] visualization
- [x] Particle System
	- [x] Renderers
		- [ ] Rope
	- [x] Emitters
		- [ ] Duration 
	- [x] Initializers
	- [x] Operators
		- [ ] Control Point Force
		- [ ] Vortex
	- [ ] Control Points
	- [ ] Children
- [ ] 3D model
- [ ] Timeline animations
- [ ] Puppet warp
- [ ] Scenescript  

### Web
Basic web apis are supported, but the audio api dose not send data at now.  
#### no webgl
WebEngineView in plasmashell can't init opengl.  
Some wallpaper using webgl may not work, and performance may be bad.   

### Video HWdecode  
#### QtMultimedia
Default video backend of this plugin.  
It's using GStreamer to play video.  
[hwdecode](https://wiki.archlinux.org/title/GStreamer#Hardware_video_acceleration) for GStreamer

#### Mpv
Need to compile plugin lib.  
The config is set to `hwdec=auto`, and not configurable at now.  

### Acknowledgments
- [RePKG](https://github.com/notscuffed/repkg)
- [RenderDoc](https://renderdoc.org/)
- [NVIDIA Nsight Graphics](https://developer.nvidia.com/nsight-graphics)

### Preview
![](https://cdn.pling.com/img/f/b/9/f/63f1672d628422f92fd189fe55f60ee8c9f911a691d0745eeaf51d2c6fae6763b8f8.jpg)
![](https://cdn.pling.com/img/d/7/9/f/c28d236408e66ba3cbca5173fb0bf4362b9df45e6e1c485deb6d9f7b4fe6adf93a2b.jpg)
![](https://cdn.pling.com/img/0/e/e/9/23b2aefba63630c7eb723afc202cdaaa2809d32d8a2ddca03b9fec8f82de62d721cd.jpg)
