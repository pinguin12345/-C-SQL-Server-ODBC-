#pragma once
#include <string>

// SHA-256 в виде строки шестнадцатеричных цифр в верхнем регистре.
// Так же считает CONVERT(..., HASHBYTES('SHA2_256', ...), 2) в MS SQL.
std::string heshParolya(const std::string& parol);
