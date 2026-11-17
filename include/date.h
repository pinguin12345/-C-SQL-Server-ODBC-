#pragma once
#include <string>




// true, если строка — существующая дата в формате ДД.ММ.ГГГГ.
bool dataHoroshaya(const std::string& s);

// Календарные дни от начала до конца включительно, -1 если даты неверные.
int skolkoDney(const std::string& nach, const std::string& kon);

// Даты нельзя сравнивать как строки: в ДД.ММ.ГГГГ день стоит первым.
// Для неверных дат вернет false.
bool dataNePozdnee(const std::string& a, const std::string& b);

// Строго раньше. Для неверных дат вернет false.
bool dataRanshe(const std::string& a, const std::string& b);

// Сегодняшняя дата этого компьютера в формате ДД.ММ.ГГГГ.
std::string segodnya();
