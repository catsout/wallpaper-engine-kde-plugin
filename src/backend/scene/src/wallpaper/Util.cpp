#include "Util.h"

namespace wp = wallpaper;

wp::LineStr wp::GetLineWith(const std::string& src, const std::string& find_str, long pos)
{
	LineStr line;
	std::string::size_type find = src.find(find_str, pos);
	line.pos = find;
	if(find != std::string::npos)
	{
		std::string::size_type end = src.find('\n', find + find_str.size());
		end = (end==std::string::npos)?src.size():end;
		line.len = end - find;
		line.value = src.substr(line.pos, line.len);
	}
	return line;
}

bool wp::DeleteLine(std::string& src, LineStr& line)
{
	if(line.pos != std::string::npos)
	{
		src.erase(line.pos, line.len);
		return true;
	}
	return false;
}


//splite string, may return vector contains empty string
std::vector<std::string> wp::SpliteString(std::string str, std::string spliter)
{
    std::vector<std::string> result;
    int pos = 0;
    while((pos = str.find_first_of(spliter)) != std::string::npos)
    {
        result.push_back(str.substr(0, pos));
        str = str.substr(pos+1);
    }
    result.push_back(str);
    return result;
}

std::string wp::ConectVecString(const std::vector<std::string>& strs,const std::string& conector, int first, int last)
{
    std::string result("");
    if(first == last)
        return strs[last];
    else if(first > last)
        return "";
    result += strs[first] + conector + ConectVecString(strs, conector, first+1, last);
    return result;
}

int wp::readInt32(std::ifstream& f)
{
    int32_t i;
    f.read(reinterpret_cast<char *>(&i),sizeof(i));
    return i;
}


float wp::ReadFloat(std::ifstream& f) {
    float i;
    f.read(reinterpret_cast<char *>(&i),sizeof(i));
    return i;
}

std::string wp::readSizedString(std::ifstream& f)
{
    uint len = readInt32(f);
    std::string result;
    result.resize(len);
    f.read(&result[0],len);
    return result;
}

std::string wp::FileSuffix(const std::string& file)
{
	auto dotp = file.find_last_of(".");
	if (dotp != std::string::npos)
		return file.substr(dotp+1);	
	return std::string();
}

