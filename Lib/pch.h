// pch.h: это предварительно скомпилированный заголовочный файл.
// Перечисленные ниже файлы компилируются только один раз, что ускоряет последующие сборки.
// Это также влияет на работу IntelliSense, включая многие функции просмотра и завершения кода.
// Однако изменение любого из приведенных здесь файлов между операциями сборки приведет к повторной компиляции всех(!) этих файлов.
// Не добавляйте сюда файлы, которые планируете часто изменять, так как в этом случае выигрыша в производительности не будет.

#ifndef PCH_H
#define PCH_H

// Добавьте сюда заголовочные файлы для предварительной компиляции
#include "framework.h"

#include <string>
#include <limits>
#include <locale>
#include <codecvt>
#include <cwctype>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>

#define _MAX_PATH 4096
#define _MAX_DIR  4096
#define _MAX_FNAME 4096
#define _MAX_EXT 4096
#define ARG_NONE 0
#define ARG_REQ 1
#define ARG_OPT 2
#define ARG_NULL 0

std::wstring utf8_to_wstring (const std::string& str);
std::string wstring_to_utf8 (const std::wstring& wstr);
void wsplitpath_s(const std::string &str,  std::string &f_path, std::string &f_name, std::string &f_ext);

// удаление заданного символа с обоих концов строки
std::wstring trim(const std::wstring &str, const wchar_t trim_char = L' ');
std::string trim(const std::string &str, const char trim_char = ' ');
// удаление заданного символа в начале строки
std::wstring trimLeft(const std::wstring &str, const wchar_t trim_char = L' ');
std::string trimLeft(const std::string &str, const char trim_char = ' ');
// удаление заданного символа в конце строки
std::wstring trimRight(const std::wstring &str, const wchar_t trim_char = L' ');
std::string trimRight(const std::string &str, const char trim_char = ' ');

std::wstring strToLower(const std::wstring &str);
std::string strToLower(const std::string &str);

std::wstring strToUpper(const std::wstring &str);
std::string strToUpper(const std::string &str);


#endif //PCH_H
