#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include "pkg.h"
#include <iostream>
#include <algorithm>
#include "common.h"

std::string& wallpaper::fs::GetAssetsPath() {
    static std::string assetsPath = "assets";
    return assetsPath;
}

//private
typedef wallpaper::fs::file_node file_node;

auto                     GetFile(file_node& root, const std::string& path);
auto                     GetFile(const file_node& root, const std::string& path);
int                      readInt32(std::ifstream& f);
std::string              readSizedString(std::ifstream& f);
int                      GetPkgHeadSize(const std::string& pkgPath);

//return root.end() path is not in root
auto GetFile(file_node& root, const std::string& path)
{
    std::vector<std::string> names = wallpaper::SpliteString(path, "/");
    file_node* now = &root;
    auto finded = now->end();
    for(const auto& name:names)
    {
        finded = now->find(name);
        //it's must be found
        if(finded == now->end()) break;
        now = &(finded->second);
    }
    return finded == now->end()?root.end():finded;
}

auto GetFile(const file_node& root, const std::string& path)
{
    std::vector<std::string> names = wallpaper::SpliteString(path, "/");
    const file_node* now = &root;
    auto finded = now->end();
    for(const auto& name:names)
    {
        finded = now->find(name);
        //it's must be found
        if(finded == now->end()) break;
        now = &(finded->second);
    }
    return finded == now->end()?root.end():finded;
}


void PrintNode(const file_node& node, int depth, int now_depth = 0)
{
    if(now_depth == depth +1 && depth != -1)
        return;
    for(const auto& kv:node.subnode)
    {
        std::cout << std::string(now_depth,' ') << kv.first << std::endl;
        PrintNode(kv.second, depth, now_depth+1);
    }
}

int readInt32(std::ifstream& f)
{
    int32_t i;
    f.read(reinterpret_cast<char *>(&i),sizeof(i));
    return i;
}

std::string readSizedString(std::ifstream& f)
{
    uint len = readInt32(f);
    std::string result;
    result.resize(len);
    //memset(result,'\0',len+1);
    f.read(&result[0], len);
    return result;
}

int GetPkgHeadSize(const std::string& pkgPath)
{
    std::ifstream pkgFile;
    pkgFile.open(pkgPath);

    readSizedString(pkgFile);
    int entryCount = readInt32(pkgFile);
    for(int i=0 ; i<entryCount ; i++)
    {
        readSizedString(pkgFile); // path
        readInt32(pkgFile);       // offset
        readInt32(pkgFile);       // length
    }
    return pkgFile.tellg();
}




//public
// create dir along the path
// fail when get "" name or a file is already there
bool wallpaper::fs::CreateDir(file_node& root, const std::string& path)
{
    std::vector<std::string> names = SpliteString(path, "/");
    file_node* now = &root;
    auto found = now->end();
    for(const auto& name:names)
    {
        //give up when name is empty
        if(name.empty()) return false;
        found = now->find(name);
        //must exist
        if(found == now->end())
            (*now)[name] = file_node(); //create dir
        else
        {
            if( ! IsDir((*now)[name])) return false;
        }
        now = &(*now)[name];
    }
    return true;
}

void wallpaper::fs::PrintFileTree(const file_node& node, int depth)
{
    PrintNode(node,depth);
}

bool wallpaper::fs::NewFile(file_node& root, const std::string& path, const file_node& new_file)
{
    std::vector<std::string> names = SpliteString(path, "/");
    file_node* now = &root;
    auto found = now->end();
    std::string dirs =  wallpaper::ConectVecString(names, "/", 0, names.size()-2);
    if(dirs.empty())
    {
        root[names[0]] = new_file;
        return true;
    }
    else
    {
        if(! CreateDir(root, dirs)) return false;
        auto iter_lastdir = GetFile(root,dirs);
        if(iter_lastdir != root.end())
        {
            iter_lastdir->second[names[names.size()-1]] = new_file;
            return true;
        }
    }
    return false;
}

int wallpaper::fs::ReadPkgToNode(file_node& root, const std::string& pkgPath)
{
    std::ifstream pkgFile;
    pkgFile.open(pkgPath);
	if(!pkgFile.is_open())
		return -1;
    std::string ver = readSizedString(pkgFile);
	LOG_INFO("pkg version: " + ver);
    int headerSize = GetPkgHeadSize(pkgPath);
    int entryCount = readInt32(pkgFile);
    //Entry entry;
    std::string node_path;
    for(int i=0 ; i<entryCount ; i++)
    {
        file_node new_file;
        new_file.path = pkgPath;
        node_path = readSizedString(pkgFile);
        //std::cout << fullPath << std::endl;
        new_file.offset = readInt32(pkgFile) + headerSize;
        new_file.length = readInt32(pkgFile);
        NewFile(root, node_path, new_file);
    }
    return std::stoi(ver.c_str()+4);
}

void AddLocalDir(file_node& root, const std::string& pkgPath)
{
    //to do
}

bool wallpaper::fs::IsFileInNode(const file_node& root, const std::string& path)
{
    return GetFile(root, path) != root.end();
}

int wallpaper::fs::GetFileLentgh(const file_node& root, const std::string& path)
{
    auto iter = GetFile(root, path);
    return iter == root.end()?0:iter->second.length;
}


bool wallpaper::fs::IsFileExist(const std::string& filePath)
{
    std::ifstream infile(filePath);
    return infile.good();
}
//use tex path to return ifstream read to tex file
//use path to return string
//no way to identify the path is use for pkg , assets, or abselutely path
std::ifstream wallpaper::fs::GetFstream(const file_node& root, const std::string& path)
{
    std::ifstream f;
    auto file_in_pkg = GetFile(root, path);
    if(file_in_pkg != root.end())
    {
        f.open(file_in_pkg->second.path);
        f.seekg(file_in_pkg->second.offset);
    }
    else if(IsFileExist(GetAssetsPath()+"/"+path))
    {
        f.open(GetAssetsPath()+"/"+path);
    }
	if(!f.is_open()) {
		LOG_ERROR("ERROR::GetFstream File could not load " + path);
	}
    return f;
}

std::string wallpaper::fs::GetContent(const file_node& root, const std::string& path)
{
    std::ifstream f = GetFstream(root, path);
	if(!f.is_open()) return std::string();
    std::string result;
    if(IsFileInNode(root, path))
    {
        int length = GetFileLentgh(root, path);
        result.resize(length);
        f.read(&result[0], length);
    }
    else
    {
        std::stringstream ss;ss << f.rdbuf();
        result = ss.str();
    }
    return result;
}
/*
void WallpaperShader::loadWallpaperTex(std::ifstream& file)
{
    typedef int (*fun)(std::ifstream&);
    //texv texi
    std::string xv,xi,xb;
    xv.resize(9);
    file.read(&xv[0],9);
    xi.resize(9);
    file.read(&xi[0],9);

    //read header
    int format,flags,textureWidth,textureHeight,imageWidth,imageHeight,unkInt0;
    format = readInt32(file);
    flags = readInt32(file);
    textureWidth = readInt32(file);
    textureHeight = readInt32(file);
    imageWidth = readInt32(file);
    imageHeight = readInt32(file);
    unkInt0 = readInt32(file);

    int imageCount,mipmapCount,freeImageFormat;
    xb.resize(9);
    file.read(&xb[0],9);
    int xbVer = std::stoi(xb.c_str()+4);
    imageCount = readInt32(file);
    if(xbVer == 3)
        freeImageFormat = readInt32(file);
    else if(xbVer > 3||xbVer < 1)
        {std::cout << "ERROR::WallpaperShader::loadWallpaperTex not support texb version " << xbVer << std::endl;return;}

    mipmapCount = readInt32(file);
    if(mipmapCount > MAX_MIPMAP_COUNT)
        {std::cout << "ERROR::WallpaperShader::loadWallpaperTex too many mipmap" << std::endl;return;}

    for(int iImage = 0; iImage < imageCount; iImage++)
    {
        if(iImage == 1)
            {std::cout << "ERROR::WallpaperShader::loadWallpaperTex no multiple tex image support" << std::endl;return;}

        for(int iMipmap = 0; iMipmap < mipmapCount; iMipmap++)
        {
            int mWidth = readInt32(file);
            int mHeight = readInt32(file);
            bool IsLZ4Compressed = false;
            int DecompressedBytesCount = 0;
            if(xbVer > 1)
            {
                IsLZ4Compressed =readInt32(file) == 1;
                DecompressedBytesCount = readInt32(file);
            }
            int byteCount = readInt32(file);
            char* src,*dst,*result;
            src = new char[byteCount];
            file.read(src,byteCount);
            result = src;
            if(IsLZ4Compressed)
            {
                dst = new char[DecompressedBytesCount];
                int x = LZ4_decompress_safe(src,dst,byteCount,DecompressedBytesCount);
                if (x < DecompressedBytesCount)
                    {std::cout<< "ERROR::WallpaperShader::loadWallpaperTex lz4 decompress error" << std::endl;return;}
                result = dst;
            }
            if(xbVer == 3)
            {
                int n;
                result = (char*)stbi_load_from_memory((const unsigned char*)result,IsLZ4Compressed?DecompressedBytesCount:byteCount,&mWidth,&mHeight,&n,4);
                std::cout << "stb " << mWidth  << "x" << mHeight << " " << n << std::endl;
            }
            Uniform::loadImage2DToGL(iMipmap,getTexGLFormat(format),mWidth,mHeight,result);
            delete [] src;
            if(IsLZ4Compressed) delete [] dst;
            if(xbVer == 3) stbi_image_free(result);
        }
    }

}
*/

