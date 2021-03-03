#pragma once
#include <memory>
#include <string>

#include <iostream>
#include "GLWrapper.h"

namespace wallpaper
{
namespace ImageType {
	// From Repkg github repo
	enum Type {
        /// Unknown format (returned value only, never use it as input value)
        UNKNOWN = -1,
		
        /// Windows or OS/2 Bitmap File (*.BMP)
        BMP = 0,
		
        /// Windows Icon (*.ICO)
        ICO = 1,

        /// Independent JPEG Group (*.JPG, *.JIF, *.JPEG, *.JPE)
        JPEG = 2,

        /// JPEG Network Graphics (*.JNG)
        JNG = 3,

        /// Commodore 64 Koala format (*.KOA)
        KOALA = 4,

        /// Amiga IFF (*.IFF, *.LBM)
        LBM = 5,

        /// Amiga IFF (*.IFF, *.LBM)
        IFF = 5,

        /// Multiple Network Graphics (*.MNG)
        MNG = 6,

        /// Portable Bitmap (ASCII) (*.PBM)
        PBM = 7,

        /// Portable Bitmap (BINARY) (*.PBM)
        PBMRAW = 8,

        /// Kodak PhotoCD (*.PCD)
        PCD = 9,

        /// Zsoft Paintbrush PCX bitmap format (*.PCX)
        PCX = 10,

        /// Portable Graymap (ASCII) (*.PGM)
        PGM = 11,

        /// Portable Graymap (BINARY) (*.PGM)
        PGMRAW = 12,

        /// Portable Network Graphics (*.PNG)
        PNG = 13,

        /// Portable Pixelmap (ASCII) (*.PPM)
        PPM = 14,

        /// Portable Pixelmap (BINARY) (*.PPM)
        PPMRAW = 15,

        /// Sun Rasterfile (*.RAS)
        RAS = 16,

        /// truevision Targa files (*.TGA, *.TARGA)
        TARGA = 17,

        /// Tagged Image File Format (*.TIF, *.TIFF)
        TIFF = 18,

        /// Wireless Bitmap (*.WBMP)
        WBMP = 19,

        /// Adobe Photoshop (*.PSD)
        PSD = 20,

        /// Dr. Halo (*.CUT)
        CUT = 21,

        /// X11 Bitmap Format (*.XBM)
        XBM = 22,

        /// X11 Pixmap Format (*.XPM)
        XPM = 23,

        /// DirectDraw Surface (*.DDS)
        DDS = 24,

        /// Graphics Interchange Format (*.GIF)
        GIF = 25,

        /// High Dynamic Range (*.HDR)
        HDR = 26,

        /// Raw Fax format CCITT G3 (*.G3)
        FAXG3 = 27,

        /// Silicon Graphics SGI image format (*.SGI)
        SGI = 28,

        /// OpenEXR format (*.EXR)
        EXR = 29,

        /// JPEG-2000 format (*.J2K, *.J2C)
        J2K = 30,

        /// JPEG-2000 format (*.JP2)
        JP2 = 31,

        /// Portable FloatMap (*.PFM)
        PFM = 32,

        /// Macintosh PICT (*.PICT)
        PICT = 33,

        /// RAW camera image (*.*)
        RAW = 34,
	};
	std::string to_string(Type);
}

class Image
{
public:
	Image() {};
	Image(const char* file_data, int size, ImageType::Type type);

	template<class Deleter>
		Image(int width, int height, gl::TextureFormat::Format format, ImageType::Type type, char* rawdata, size_t size, Deleter deleter):m_width(width),m_height(height),m_format(format),m_type(type),m_size(size),m_data(rawdata, deleter) {
	}


	int Width() const {return m_width;}
	int Height() const {return m_height;}
	auto Format() const {return m_format;}
	auto Type() const {return m_type;}

	const char* RawData() const {return m_data.get();};
	size_t Size() const {return m_size;}
private:
	int m_width;
	int m_height;
	gl::TextureFormat::Format m_format;
	ImageType::Type m_type;
	std::shared_ptr<char> m_data;
	size_t m_size;
};
}
