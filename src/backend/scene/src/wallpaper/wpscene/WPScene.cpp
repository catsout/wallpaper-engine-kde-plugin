#include "WPScene.h"
#include "common.h"

using namespace wallpaper::wpscene;

bool Orthogonalprojection::FromJson(const nlohmann::json& json) {
    if(json.is_null()) return false;
	if(json.contains("auto")) {
		GET_JSON_NAME_VALUE(json, "auto", auto_);
	}
	else {
		GET_JSON_NAME_VALUE(json, "width", width);
		GET_JSON_NAME_VALUE(json, "height", height);
	}
    return true;
}

bool WPSceneCamera::FromJson(const nlohmann::json& json) {
    GET_JSON_NAME_VALUE(json, "center", center);
    GET_JSON_NAME_VALUE(json, "eye", eye);
    GET_JSON_NAME_VALUE(json, "up", up);
    return true;
}

bool WPSceneGeneral::FromJson(const nlohmann::json& json) {
	GET_JSON_NAME_VALUE(json, "clearcolor", clearcolor);
	GET_JSON_NAME_VALUE(json, "cameraparallax", cameraparallax);
	GET_JSON_NAME_VALUE(json, "cameraparallaxamount", cameraparallaxamount);
	GET_JSON_NAME_VALUE(json, "cameraparallaxdelay", cameraparallaxdelay);
	GET_JSON_NAME_VALUE(json, "cameraparallaxmouseinfluence", cameraparallaxmouseinfluence);
	GET_JSON_NAME_VALUE(json, "zoom", zoom);
    if(json.contains("orthogonalprojection")) {
        const auto& ortho = json.at("orthogonalprojection");
        if(ortho.is_null())
            isOrtho = false;
        else {
            isOrtho = true;
            orthogonalprojection.FromJson(ortho);
        }
    }
    return true;
}

bool WPScene::FromJson(const nlohmann::json& json) {
    if(json.contains("camera")) {
        camera.FromJson(json.at("camera"));
    } else {
        LOG_ERROR("scene no camera");
        return false;
    }
    if(json.contains("general")) {
        general.FromJson(json.at("general"));
    } else {
        LOG_ERROR("scene no genera data");
        return false;
    }
    return true; 
}
