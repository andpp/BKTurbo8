// pch.cpp: файл исходного кода, соответствующий предварительно скомпилированному заголовочному файлу

#include "pch.h"

// При использовании предварительно скомпилированных заголовочных файлов необходим следующий файл исходного кода для выполнения сборки.

//std::wstring string_to_wstring(std::string &str)
//{
//    std::wstring wstr;
//    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv(str, wstr);
//    return wstr;
//}

// convert UTF-8 string to wstring
std::wstring utf8_to_wstring (const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> res;
    return res.from_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8 (const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> res;
    return res.to_bytes(wstr);
}

void wsplitpath_s(const std::string &str,  std::string &f_path, std::string &f_name, std::string &f_ext)
{

    size_t pos = str.find_last_of("\\/");

    if (std::string::npos == pos) {
        f_path = "";
        f_name = str;
    } else {
        f_path = str.substr(0, pos);
        f_name = str.substr(pos+1);
    }

    // Extension
    std::string::size_type idx;

    idx = f_name.rfind('.');
    if(idx != std::string::npos)
    {
        f_ext = f_name.substr(idx+1);
        f_name = f_name.substr(0,idx);
    }
    else
    {
        f_ext = "";
    }
}

std::wstring trimLeft(const std::wstring &str, const wchar_t trim_char)
{
    std::wstring res = str;

    while (!res.empty() && (res.front() == trim_char))
    {
        res.erase(res.begin());
    }

    return res;
}

std::string trimLeft(const std::string &str, const char trim_char)
{
    std::string res = str;

    while (!res.empty() && (res.front() == trim_char))
    {
        res.erase(res.begin());
    }

    return res;
}

std::wstring trimRight(const std::wstring &str, const wchar_t trim_char)
{
    std::wstring res = str;

    while (!res.empty() && (res.back() == trim_char))
    {
        res.pop_back();
    }

    return res;
}

std::string trimRight(const std::string &str, const char trim_char)
{
    std::string res = str;

    while (!res.empty() && (res.back() == trim_char))
    {
        res.pop_back();
    }

    return res;
}


std::wstring trim(const std::wstring &str, const wchar_t trim_char)
{
    std::wstring res = trimLeft(str, trim_char);
    return trimRight(res, trim_char);
}

std::string trim(const std::string &str, const char trim_char)
{
    std::string res = trimLeft(str, trim_char);
    return trimRight(res, trim_char);
}


std::wstring strToLower(const std::wstring &str)
{
    std::wstring res;

    if (!str.empty())
    {
//		_locale_t loc = _create_locale(LC_ALL, "Russian");

//		for (wchar_t n : str)
//		{
//			res.push_back(_tolower_l(n, loc));
//		}

//		_free_locale(loc);
        res = str;
        std::transform(
          res.begin(), res.end(),
          res.begin(),
          towlower);
    }

    return res;
}

std::string strToLower(const std::string &str)
{
    std::string res;

    if (!str.empty())
    {
//		_locale_t loc = _create_locale(LC_ALL, "Russian");

//        for (char n : str)
//        {
//            res.push_back(tolower(n));
//        }

//		_free_locale(loc);
        res = str;
        std::transform(
          res.begin(), res.end(),
          res.begin(),
          tolower);
    }

    return res;
}

std::wstring strToUpper(const std::wstring &str)
{
    std::wstring res;

    if (!str.empty())
    {
//		_locale_t loc = _create_locale(LC_ALL, "Russian");

//		for (wchar_t n : str)
//		{
//			res.push_back(_tolower_l(n, loc));
//		}

//		_free_locale(loc);
        res = str;
        std::transform(
          res.begin(), res.end(),
          res.begin(),
          towupper);
    }

    return res;
}

std::string strToUpper(const std::string &str)
{
    std::string res;

    if (!str.empty())
    {
//		_locale_t loc = _create_locale(LC_ALL, "Russian");

//        for (char n : str)
//        {
//            res.push_back(tolower(n));
//        }

//		_free_locale(loc);
        res = str;
        std::transform(
          res.begin(), res.end(),
          res.begin(),
          toupper);
    }

    return res;
}
