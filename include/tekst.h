#pragma once
#include <cstddef>
#include <string>

// Внутри программы текст хранится в UTF-8, ODBC и консоль работают с wchar_t.
std::wstring vShirokuyu(const std::string& s);
std::string vUtf8(const std::wstring& s);

// Число букв, а не байтов: русская буква в UTF-8 занимает два байта.
std::size_t kolichestvoSimvolov(const std::string& s);
