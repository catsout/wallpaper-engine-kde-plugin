## README
A wallpaper plgin integrating wallpaper engine in kde wallpaper setting.
It's simple and small.

### note:
- Only **video** and **web** are supported.
- in web type, desktop right click not work
- Qt >= 5.13

You need to choose your steam workshop directory. Like `Steam/steamapps/workshop`

### install
```sh
tar -cf wp-kde.tar WallpaperEngineForKde
kpackagetool5 -t Plasma/Wallpaper -i wp-kde.tar
```
### uninstall
```sh
kpackagetool5 -r WallpaperEngineForKde
```

### Credits

- [Smart Video Wallpaper](https://store.kde.org/p/1316299/)                                                                                                                                                                                                                                                                          

