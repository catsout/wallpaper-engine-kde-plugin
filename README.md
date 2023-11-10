# Wallpaper Engine for Kde
A wallpaper plugin integrating [wallpaper engine](https://store.steampowered.com/app/431960/Wallpaper_Engine) into kde wallpaper setting.  

## Note
- Known issues:
  - Some scene wallpapers may **crash** your KDE.  
    Remove `WallpaperSource` line in `~/.config/plasma-org.kde.plasma.desktop-appletsrc` and restart KDE to fix.  
  - Mouse long press (to enter panel edit mode) is broken on desktop.
  - Screen Locking is not supported, please not use this plugin in screen locking.  
- Support **scene(2d)**,**video**,**web** wallpaper types
- Requires *Wallpaper Engine* installed on steam
- Requires C++20(gcc 10+)
- Requires Python 3.5+
- Requires Qt 5.13+ for playing video(no mpv), or mpv instead  
- Requires Vulkan 1.1+, Opengl [External Memory Object](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_external_objects.txt) extension
- Requires [vulkan driver](https://wiki.archlinux.org/title/Vulkan#Installation) installed  
If you are using amd, please choose RADV driver.  

## Install

Gentoo:
```sh
sudo emerge -av eselect-repository
sudo eselect repository enable gig
sudo emerge -av kde-misc/wallpaper-engine-kde-plugin
```

#### Dependencies
Debian:  
```sh
sudo apt install build-essential libvulkan-dev plasma-workspace-dev gstreamer1.0-libav \
liblz4-dev libmpv-dev python3-websockets qtbase5-private-dev \
libqt5x11extras5-dev \
qml-module-qtwebchannel qml-module-qtwebsockets cmake
```  

Fedora:  
```sh
# Please add "RPM Fusion" repo first
sudo dnf install vulkan-headers plasma-workspace-devel kf5-plasma-devel gstreamer1-libav \
lz4-devel mpv-libs-devel python3-websockets qt5-qtbase-private-devel \
qt5-qtx11extras-devel qt5-qtwebchannel-devel qt5-qtwebsockets-devel cmake
```

Arch:  
```sh
sudo pacman -S extra-cmake-modules plasma-framework gst-libav \
base-devel mpv python-websockets qt5-declarative qt5-websockets qt5-webchannel vulkan-headers cmake
```

Void:  
```sh
sudo xbps-install -S extra-cmake-modules plasma-framework \
gst-libav base-devel mpv python3-websockets qt5-declarative qt5-websockets \
qt5-webchannel plasma-workspace-devel mpv-devel liblz4-devel Vulkan-Headers cmake
```

openSUSE:
```sh
# You'll need to add the Packman repository first
sudo zypper in vulkan-devel plasma-framework-devel plasma5-workspace-devel \
libqt5-qtwebsockets-devel mpv-devel python310-websockets \
libqt5-qtx11extras-devel liblz4-devel gstreamer-plugins-libav \
libqt5-qtbase-private-headers-devel cmake
```

Fedora Kinoite:  
see [install via rpm-ostree](rpm)

#### Note for kde store
Still need to run commands below to get scene and mpv work.  
Every time you receive update in discover, you should run these commands to update.  

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
make -j$nproc

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
		- This is usually `~/.local/share/Steam` by default
	- *Wallpaper Engine* needs to be installed in this *steamlibrary*

### Restart KDE
You need to restart plasmashell after **reinstalling the plugin**  
`systemctl --user restart plasma-plasmashell.service`  
If you are using old kde, you can't run command above, please re-login.  

## Support Status
### Scene:
Scene wallpapers are supported by vulkan 1.1  
Requires *Wallpaper Engine* installed for assets(shaders,pictures...)

#### standalone
Only for testing and debug  
Requires glfw  
```
# git clone and init submodule
cd src/backend_scene/standalone_view
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_QML=ON
make -j$nproc

./sceneviewer --help
```

#### open-source libraries
[argparse](https://github.com/p-ranav/argparse) - Command line argument parser  
[stb](https://github.com/nothings/stb) - Image loading  
[vog/sha1](https://github.com/vog/sha1) - SHA-1  
[effolkronium/random](https://github.com/effolkronium/random) - Random wrapper for modern C++   
[miniaudio](https://github.com/mackron/miniaudio) - Audio loading and playback  
[nlohmann/json](https://github.com/nlohmann/json) - Json parsing  
[Eigen](https://eigen.tuxfamily.org/index.php) - Math operations  
[glad](https://github.com/Dav1dde/glad) - Opengl loader  
[glslang](https://github.com/KhronosGroup/glslang) - Glsl to Spv  
[SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect) - C/C++ reflection API for SPIR-V bytecode  
[VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - Vulkan memory allocation library  
#### supported
- [x] Layer
	- [x] Image
	- [x] Composition / Fullscreen
	- [ ] Text
- [x] Effect
    - [x] Basic
	- [x] Mouse position with delay
	- [x] Parallax
    - [x] Depth Parallax
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
	- [x] Emitters
		- [ ] Duration 
	- [x] Initializers
	- [x] Operators
	- [x] Control Points
        - [ ] Mouse Follow
	- [x] Children
    - [ ] Audio Response
- [x] Puppet warp
- [ ] 3D model
- [ ] Timeline animations
- [ ] Scenescript  
- [ ] User Properties

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

The major work of this plugin is the scene wallpaper [renderer](src/backend_scene/src). If you want to integrate this into other desktop environments, here are some [examples](src/backend_scene/standalone_view). Currently this renderer is rendering under vulkan and sharing to opengl texture which will be read by qml(plasmashell) in kde. You can integrate this renderer into anything that can show vulkan or opengl textures.  

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
