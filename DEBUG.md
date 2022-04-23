### How to get plasmashell log
- `journalctl /usr/bin/plasmashell -f`
if you get empty, try command below   
- `plasmashell --replace`
You need to re-login to run plasmashell normally after this command.

Please choose the log after you open the wallpaper plugin setting.

### How to debug scene wallpaper
build [standalone viewer](https://github.com/catsout/wallpaper-engine-kde-plugin#standalone) with `-DCMAKE_BUILD_TYPE=Debug`  
install `vulkan-validation-layers`  
run `./sceneviewer --valid-layer <steamapps>/common/wallpaper_engine/assets <steamapps>/workshop/content/431960/<workshop_id>/scene.pkg`  
