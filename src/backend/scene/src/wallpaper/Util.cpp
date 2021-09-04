#include "Util.h"

namespace wp = wallpaper;

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

std::string wp::FileSuffix(const std::string& file)
{
	auto dotp = file.find_last_of(".");
	if (dotp != std::string::npos)
		return file.substr(dotp+1);	
	return std::string();
}

