## Texture Compile Options
- **Format** RGBA8888 is an uncompressed texture with 4 channels. The VRAM occupied equals *width * height * 4* bytes. The compiled texture is saved with LZ4 compression and a bit smaller on disk. DXT5 is a compressed format using *width * height* bytes of VRAM. Both have *N* versions for normal maps, these swizzle the G and A channels which allows more precision when using it along with DXT5.
- **Point filter** disables bilinear texture filtering. Use it for pixel art.
- **No mip maps** disables mip mapping [\[1\]](http://en.wikipedia.org/wiki/Mipmap) for this texture. Less VRAM and disk space will be occupied, but the texture will flicker when it occupies less screen space than its resolution has. Recommended to be disabled for large background images, but don't disable it on 3D models.
- **Clamp UVs** disables texture repetition when textures are sampled outside of normalized (0 - 1) coordinates and will instead return the color of the nearest pixel still on the actual texture.


## Special Texture
- _rt_imageLayerComposite_id_a/b
- _rt_FullFrameBuffer
- _rt_FullCompoBuffer1_fullscreen_id
- _rt_FullCompoBuffer1_id_id