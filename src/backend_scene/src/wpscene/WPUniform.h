#pragma once
#include "WPJson.hpp"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>
#include <cstdint>

namespace wallpaper
{
namespace wpscene
{

struct WPUniformTex {
    struct Component {
        std::string label;
        std::string combo;
    };
    std::string material; // A unique key to allow overwriting and identifying the texture easily,
                          // just needs to be a unique name.
    std::string
        label; // The name for the texture that will be shown in the editor if it's not hidden.
    std::string
        default_; // The default texture to load if the user hasn't specified a different one.
    // bool hidden {false}; // Hides the texture in the editor.
    std::string
        mode; // Sets the texture up to be painted in grayscale with a single channel to read from.
    std::string combo; // Defines a combo that will be set to 1 if the user has painted something or
                       // selected a texture and set to 0 if no texture is bound.
    std::array<float, 4> paintdefaultcolor {
        0, 0, 0, 1.0f
    }; // Defines what color should be applied when the user begins to paint a new texture.

    std::vector<Component>               components;
    bool                                 requireany { false };
    std::unordered_map<std::string, int> require;

    bool FromJson(const nlohmann::json& json) {
        GET_JSON_NAME_VALUE_NOWARN(json, "material", material);
        GET_JSON_NAME_VALUE_NOWARN(json, "label", label);
        GET_JSON_NAME_VALUE_NOWARN(json, "default", default_);

        GET_JSON_NAME_VALUE_NOWARN(json, "mode", mode);
        GET_JSON_NAME_VALUE_NOWARN(json, "combo", combo);
        if (json.contains("components")) {
            for (const auto& el : json.at("components")) {
                Component c;
                GET_JSON_NAME_VALUE(el, "label", c.label);
                GET_JSON_NAME_VALUE_NOWARN(el, "combo", c.combo);
                components.push_back(c);
            }
        }
        GET_JSON_NAME_VALUE_NOWARN(json, "requireany", requireany);
        if (json.contains("require") && json.at("require").is_object()) {
            for (const auto& el : json.at("require").items()) {
                int value { false };
                GET_JSON_VALUE(el.value(), value);
                require[el.key()] = value;
            }
        }
        return true;
    }
};
} // namespace wpscene
} // namespace wallpaper
