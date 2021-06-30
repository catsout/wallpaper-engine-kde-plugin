#pragma once
#include <string>
#include <map>

namespace wallpaper
{
namespace fs
{
std::string& GetAssetsPath();

struct file_node
{
    file_node():offset(0u),length(0u),path(),is_compressed(false) {};
    file_node(uint offset, uint length, const std::string& path, bool is_compressed=false)
        :offset(offset),length(length),path(path),is_compressed(is_compressed) {};
    uint offset;
    uint length;
    std::string path;
    bool is_compressed;
    std::map<std::string,file_node> subnode;
    file_node& operator[](const std::string& name)
    {
        return subnode[name];
    }

    auto find(const std::string& name)
    {
        return subnode.find(name);
    }

    auto find(const std::string& name) const
    {
        const std::map<std::string,file_node>& const_subnode = subnode;
        return const_subnode.find(name);
    }

    auto end() const
    {
        return subnode.end();
    }

    auto end()
    {
        return subnode.end();
    }


    auto begin() const
    {
        return subnode.begin();
    }

    auto begin()
    {
        return subnode.begin();
    }
};



inline bool IsDir(const file_node& file)
{
    return (file.length == 0u && file.offset == 0u && file.path.empty());
}

//std::map<std::string,file_node>::iterator GetFile(file_node& root, const std::string& path);
bool CreateDir(file_node& root, const std::string& path);
void PrintFileTree(const file_node& node, int depth);
bool NewFile(file_node& root, const std::string& path, const file_node& new_file);
int ReadPkgToNode(file_node& root, const std::string&pkgPath);
bool IsFileInNode(const file_node& root, const std::string& path);
bool IsFileExistWithAssets(const file_node& root, const std::string& path);
bool IsFileExist(const std::string& filePath);
int GetFileLentgh(const file_node& root, const std::string& path);
std::ifstream GetFstream(const file_node& root, const std::string& path);
std::string GetContent(const file_node& root, const std::string& path);
}
}
