TEMPLATE = lib
TARGET = wallpaperEngine
QT += qml quick core gui-private x11extras
INCLUDEPATH += .
CONFIG += qt plugin link_pkgconfig 
QT_CONFIG -= no-pkg-config

PKGCONFIG += mpv

uri = wallpaper.engineforkde

HEADERS += viewModel.h \
           wallpaperProject.h \
           backend/mpv/mpv.h \
           backend/mpv/qthelper.hpp
SOURCES += plugin.cpp viewModel.cpp wallpaperProject.cpp backend/mpv/mpv.cpp

install_path = $$[QT_INSTALL_QML]/wallpaper/engineforkde
PLUGINFILES = $$PWD/qmldir
pluginfiles_install.files = $$PLUGINFILES
pluginfiles_install.path = $$install_path
target.path = $$install_path 
INSTALLS += target pluginfiles_install
