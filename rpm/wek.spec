Name: wallpaper-engine-kde-plugin-qt6

%global shortcommit %(c=%{commit}; echo ${c:0:7})

Version: %{shortcommit}
Release: 1%{?dist}
Summary: A kde wallpaper plugin integrating wallpaper engine

Group: Development/System 
License: GPLv2
URL: https://github.com/catsout/wallpaper-engine-kde-plugin
Source0: https://github.com/catsout/wallpaper-engine-kde-plugin/archive/%{commit}.tar.gz
Source1: https://github.com/KhronosGroup/glslang/archive/refs/tags/%{glslang_ver}.tar.gz

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: mpv-libs-devel vulkan-headers plasma-workspace-devel kf6-plasma-devel lz4-devel qt6-qtbase-private-devel qt5-qtx11extras-devel
Requires: plasma-workspace gstreamer1-libav mpv-libs lz4 python3-websockets qt6-qtwebchannel-devel qt6-qtwebsockets-devel

%description

%prep
%setup -q -n wallpaper-engine-kde-plugin-%{commit}
%setup -T -D -a 1 -n wallpaper-engine-kde-plugin-%{commit}
rm -r src/backend_scene
git clone --recurse-submodules https://github.com/catsout/wallpaper-scene-renderer.git src/backend_scene

%global _enable_debug_package 0
%global debug_package %{nil}

%build
mkdir -p build && cd build
cmake .. -DUSE_PLASMAPKG=ON
%make_build

%install
rm -rf $RPM_BUILD_ROOT
cd build
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_libdir}/*

%changelog 
