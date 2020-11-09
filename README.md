## README
A wallpaper plgin integrating *wallpaper engine* into kde wallpaper setting.  
It's simple and small.  

### Note:
- Only **video** and **web** are supported.
- Only test on x11.
- Need Qt >= 5.13, for loop video support without black screen
- With web type, desktop right click not work

You need to choose your steam workshop directory. Like `Steam/steamapps/workshop`  

### Better performance for intel card
May not greatly improve  
#### hwdecode
Qt using GStreamer for video backend.  
You need make your system gstreamer have vaapi plugin installed.  
on debian using `apt install gstreamer1.0-vaapi`  

#### egl
put `export QT_XCB_GL_INTEGRATION=xcb_egl` to where the xorg-sever will read.  
on debian `~/.xsessionrc` is ok.  

##### kwin display wrong
egl may break kwin.  
use `export QT_XCB_GL_INTEGRATION=xcb_glx kwin --replace`

#### use mpv

### Install
```sh
tar -cf wp-kde.tar WallpaperEngineForKde
kpackagetool5 -t Plasma/Wallpaper -i wp-kde.tar
```
### Uninstall
```sh
kpackagetool5 -r WallpaperEngineForKde
```

### Credits

- [Smart Video Wallpaper](https://store.kde.org/p/1316299/)                                                                                                                                                                                                                                                                          

