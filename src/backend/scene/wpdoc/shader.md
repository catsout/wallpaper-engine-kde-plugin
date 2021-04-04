## Shaders

Shaders are written in GLSL version 120 with a few minor exceptions. They are then translated into HLSL dynamically and compiled as shader model 3.0. The attributes or uniforms you can use are either globally predefined or you can expose them into the editor (or even the user, for e.g. tintable colors). Refer to the assets/shaders/readme.txt file in the application to get an up to date list of available uniforms and conventions.  

### Uniforms json

The following JSON options exist:

    material (STRING): Set "material": "MY MATERIAL NAME" to define the name shown in the editor.
    type (STRING): Set "type": "color" for vec3 uniforms to enable the color picker. Otherwise leave it out to use a slider or text boxes to enter values.
    default (VARYING): Set "default": "1 0 0" to define a default color (red) or "default": 1.0 to define a default float.
    range (ARRAY of FLOAT): Set "range": [-1.0, 1.0] to overwrite the slider range. It will use 0 to 1 if you omit it.