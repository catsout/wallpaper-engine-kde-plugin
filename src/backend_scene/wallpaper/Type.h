#pragma once
#include <string>

namespace wallpaper 
{
// From Repkg github repo
enum class ImageType {
	UNKNOWN = -1,   /// Unknown format (returned value only, never use it as input value)
	BMP = 0,	/// Windows or OS/2 Bitmap File (*.BMP)
	ICO = 1,	/// Windows Icon (*.ICO)
	JPEG = 2,   /// Independent JPEG Group (*.JPG, *.JIF, *.JPEG, *.JPE)
	JNG = 3,	/// JPEG Network Graphics (*.JNG)
	KOALA = 4,  /// Commodore 64 Koala format (*.KOA)
	LBM = 5,	/// Amiga IFF (*.IFF, *.LBM)
	MNG = 6,	/// Multiple Network Graphics (*.MNG)
	PBM = 7,	/// Portable Bitmap (ASCII) (*.PBM)
	PBMRAW = 8, /// Portable Bitmap (BINARY) (*.PBM)
	PCD = 9,	/// Kodak PhotoCD (*.PCD)
	PCX = 10,	/// Zsoft Paintbrush PCX bitmap format (*.PCX)
	PGM = 11,	/// Portable Graymap (ASCII) (*.PGM)
	PGMRAW = 12,/// Portable Graymap (BINARY) (*.PGM)
	PNG = 13,	/// Portable Network Graphics (*.PNG)
	PPM = 14,	/// Portable Pixelmap (ASCII) (*.PPM)
	PPMRAW = 15,/// Portable Pixelmap (BINARY) (*.PPM)
	RAS = 16,	/// Sun Rasterfile (*.RAS)
	TARGA = 17, /// truevision Targa files (*.TGA, *.TARGA)
	TIFF = 18,  /// Tagged Image File Format (*.TIF, *.TIFF)
	WBMP = 19,  /// Wireless Bitmap (*.WBMP)
	PSD = 20,	/// Adobe Photoshop (*.PSD)
	CUT = 21,	/// Dr. Halo (*.CUT)
	XBM = 22,	/// X11 Bitmap Format (*.XBM)
	XPM = 23,	/// X11 Pixmap Format (*.XPM)
	DDS = 24,	/// DirectDraw Surface (*.DDS)
	GIF = 25,	/// Graphics Interchange Format (*.GIF)
	HDR = 26,	/// High Dynamic Range (*.HDR)
	FAXG3 = 27,	/// Raw Fax format CCITT G3 (*.G3)
	SGI = 28,	/// Silicon Graphics SGI image format (*.SGI)
	EXR = 29,	/// OpenEXR format (*.EXR)
	J2K = 30,	/// JPEG-2000 format (*.J2K, *.J2C)
	JP2 = 31,	/// JPEG-2000 format (*.JP2)
	PFM = 32,	/// portable floatmap (*.pfm)
	PICT = 33,	/// Macintosh PICT (*.PICT)
	RAW = 34,	/// RAW camera image (*.*)
};
std::string ToString(const ImageType&);

enum class TextureFormat {
	BC1,  // DXT1
	BC2,  // DXT3
	BC3,  // DXT5
	RGB8,
	RGBA8,
	RG8,
	R8
};
std::string ToString(const TextureFormat&);

enum class BlendMode {
	Disable,
	Translucent,
	Additive,
	Normal
};

enum class ShaderType {
	VERTEX,
	GEOMETRY,
	FRAGMENT
};

enum class TextureType {
	IMG_2D,
};

enum class MeshPrimitive {
	POINT,
	TRIANGLE
};

enum class FillMode {
	STRETCH,
	ASPECTFIT,	
	ASPECTCROP
};

enum class TextureWrap {
	CLAMP_TO_EDGE,
	REPEAT
};

enum class TextureFilter {
	LINEAR,
	NEAREST
};

struct TextureSample {
	TextureWrap wrapS {TextureWrap::REPEAT};
	TextureWrap wrapT {TextureWrap::REPEAT};
	TextureFilter magFilter {TextureFilter::NEAREST};
	TextureFilter minFilter {TextureFilter::NEAREST};
};

enum class VertexType {
	FLOAT1,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	UINT1,
	UINT2,
	UINT3,
	UINT4
};

}
