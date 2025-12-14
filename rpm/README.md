### This is for rpm-ostree based system

```
# Create and enter toolbox
toolbox create && toolbox enter

# Add build dependencies
sudo dnf install vulkan-headers plasma-workspace-devel kf6-plasma-devel gstreamer1-libav \
lz4-devel mpv-libs-devel python3-websockets qt6-qtbase-private-devel libplasma-devel \
qt5-qtx11extras-devel qt6-qtwebchannel-devel qt6-qtwebsockets-devel cmake ninja rpmbuild extra-cmake-modules

# Clone repo
git clone https://github.com/catsout/wallpaper-engine-kde-plugin.git
cd wallpaper-engine-kde-plugin

# Install plugin package 
mkdir -p ~/.local/share/plasma/wallpapers/com.github.catsout.wallpaperEngineKde/
cp -R ./plugin/* ~/.local/share/plasma/wallpapers/com.github.catsout.wallpaperEngineKde/

mkdir -p ~/rpmbuild/SOURCES

# Rpmbuild in toolbox
rpmbuild --define="commit $(git rev-parse HEAD)" \
    --define="glslang_ver 11.8.0" \
    --undefine=_disable_source_fetch \
    -ba ./rpm/wek.spec

# Install package
cd ~/rpmbuild/RPMS/x86_64
rpm-ostree install --uninstall=wallpaper-engine-kde-plugin ./<select-rpm-to-install>.rpm
```
