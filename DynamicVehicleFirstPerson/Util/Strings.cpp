#include "Strings.hpp"

#include <Windows.h>
#include <algorithm>

std::vector<std::string> StrUtil::Split(const std::string& s, char delim) {
    std::vector<std::string> elems;
    Split(s, delim, std::back_inserter(elems));
    return elems;
}

std::string StrUtil::ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::string StrUtil::Utf8Encode(const std::wstring& wstr) {
    if (wstr.empty())
        return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring StrUtil::Utf8Decode(const std::string& str) {
    if (str.empty())
        return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
