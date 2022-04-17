# Wallpaper Engine for Kde
A wallpaper plugin integrating [wallpaper engine](https://store.steampowered.com/app/431960/Wallpaper_Engine) into kde wallpaper setting.  

## Note
- Known issues:
  - Some scene wallpapers may **crash** your KDE.  
    Remove `WallpaperFilePath` line in `~/.config/plasma-org.kde.plasma.desktop-appletsrc` and restart KDE to fix.  
  - Mouse long press (to enter panel edit mode) is broken on desktop.
  - Screen Locking is not supported, please not use this plugin in screen locking.  
- Support **scene(2d)**,**video**,**web** wallpaper types
- Requires *Wallpaper Engine* installed on steam
- Requires Python 3.3+
- Requires CMake 3.16+
- Requires Qt 5.13+ for playing video(no mpv), or mpv instead  
- Requires Vulkan 1.1+, Opengl [External Memory Object](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_external_objects.txt) extension

## Install
#### Dependencies
Debian:  
```sh
sudo apt install build-essential libvulkan-dev plasma-workspace-dev gstreamer1.0-libav \
liblz4-dev libmpv-dev python3-websockets qtbase5-private-dev \
libqt5x11extras5-dev \
qml-module-qtwebchannel qml-module-qtwebsockets
```  

Fedora:  
```sh
# Please add "RPM Fusion" repo first
sudo dnf install vulkan-headers plasma-workspace-devel kf5-plasma-devel gstreamer1-libav \
lz4-devel mpv-libs-devel python3-websockets qt5-qtbase-private-devel \
qt5-qtx11extras-devel qt5-qtwebchannel-devel qt5-qtwebsockets-devel
```

Arch:  
```sh
sudo pacman -S extra-cmake-modules plasma-framework gst-libav \
base-devel mpv python-websockets qt5-declarative qt5-websockets qt5-webchannel vulkan-headers 
```

Void:  
```sh
sudo xbps-install -S extra-cmake-modules plasma-framework \
gst-libav base-devel mpv python3-websockets qt5-declarative qt5-websockets \
qt5-webchannel plasma-workspace-devel mpv-devel liblz4-devel Vulkan-Headers
```

Fedora Kinoite:  
see [install via rpm-ostree](rpm)

#### Build and Install
```sh
# Download source
git clone https://github.com/catsout/wallpaper-engine-kde-plugin.git
cd wallpaper-engine-kde-plugin

# Download submodule (glslang)
git submodule update --init

# Configure
# 'USE_PLASMAPKG=ON': using plasmapkg2 tool to install plugin
mkdir build && cd build
cmake .. -DUSE_PLASMAPKG=ON

# Build
make

# Install package (ignore if USE_PLASMAPKG=OFF for system-wide installation)
make install_pkg
# install lib
sudo make install
```
#### Uninstall
1. remove files that list in `wallpaper-engine-kde-plugin/build/install_manifest.txt`
2. `plasmapkg2 -r ~/.local/share/plasma/wallpapers/com.github.casout.wallpaperEngineKde`

## Usage
1. *Wallpaper Engine* installed on Steam
2. Subscribe to some wallpapers on the Workshop
3. Select the *steamlibrary* folder on the Wallpapers tab of this plugin
	- The *steamlibrary* which contains the *steamapps* folder
	- *Wallpaper Engine* needs to be installed in this *steamlibrary*

### Restart KDE
You need to restart KDE(re-login) after **reinstalling the plugin**  
Please not use `kstart5 plasmashell`, which will cause freezing  
Re-login is ok  

## Support Status
### Scene:
Scene wallpapers are supported by vulkan 1.1  
Requires *Wallpaper Engine* installed for assets(shaders,pictures...)
#### open-source libraries
[argparse](https://github.com/p-ranav/argparse) - Command line argument parser  
[stb](https://github.com/nothings/stb) - Image loading  
[miniaudio](https://github.com/mackron/miniaudio) - Audio loading and playback  
[nlohmann/json](https://github.com/nlohmann/json) - Json parsing  
[Eigen](https://eigen.tuxfamily.org/index.php) - Math operations  
[glad](https://github.com/Dav1dde/glad) - Opengl loader  
[glslang](https://github.com/KhronosGroup/glslang) - Glsl to Spv  
[Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp) - Vulkan C++ API  
[VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - Vulkan memory allocation library  
#### implemented
- [x] Layer
	- [x] Image
	- [x] Composition / Fullscreen
	- [ ] Text
- [x] Effect
	- [x] Mouse position
	- [x] Parallax
	- [x] ColorBlendMode
	- [x] PBR light
	- [ ] Global bloom
- [x] Camera
	- [x] Zoom
	- [ ] Shake
	- [ ] Fade / Path
- [x] Audio
	- [x] Loop
	- [ ] Random
	- [ ] Visualization
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
- [x] Puppet warp
- [ ] 3D model
- [ ] Timeline animations
- [ ] Scenescript  

### Web
Basic web apis are supported, but the audio api does not send data for now.  
#### no webgl
WebEngineView in plasmashell can't init opengl.  
Some wallpaper using webgl may not work, and performance may be bad.   

### Video HWdecode  
#### QtMultimedia
The default video backend of this plugin.  
It's using GStreamer to play video.  
[hwdecode](https://wiki.archlinux.org/title/GStreamer#Hardware_video_acceleration) for GStreamer

#### Mpv
Need to compile the plugin lib.  
The config is set to `hwdec=auto`, and is not configurable for now.  

## About integrating into other desktop environments
There is no general way. If there is a way to have good support for most desktop environments, why not we just require wallpaper engine itself to support linux. Some similar apps like [lively](https://github.com/rocksdanister/lively) and [ScreenPlay](https://store.steampowered.com/app/672870/ScreenPlay) can benefit from that, but that way doesn't exist. Actually the integration and implement are separated, for all integration ways, the implement is shared. So if there is a general way, we can move to it easily.  

The major work of this plugin is the scene wallpaper [renderer](src/backend_scene/wallpaper). If you want to integrate this into other desktop environments, here are some [examples](src/backend_scene/standalone_view). Currently this renderer is rendering under vulkan and sharing to opengl texture which will be read by qml(plasmashell) in kde. You can integrate this renderer into anything that can show vulkan or opengl textures.  

## Acknowledgments
- [RePKG](https://github.com/notscuffed/repkg)
- [RenderDoc](https://renderdoc.org/)
- [NVIDIA Nsight Graphics](https://developer.nvidia.com/nsight-graphics)
- [learnopengl.com](https://learnopengl.com/)
- [SaschaWillems/Vulkan](https://github.com/SaschaWillems/Vulkan)
- All the open-source libraries mentioned above

## Preview
![](https://cdn.pling.com/img/0/e/e/9/23b2aefba63630c7eb723afc202cdaaa2809d32d8a2ddca03b9fec8f82de62d721cd.jpg)
![](https://images.pling.com/img/00/00/60/17/66/1475528/screenshot-20220402-1822442.png)
![](https://images.pling.com/img/00/00/60/17/66/1475528/screenshot-20220401-1121383.png)
