#pragma once
#include <cstddef>
#include <string>

// Данные предметной области. Без логики, без ввода-вывода.

// Длины совпадают с NVARCHAR в sql/01_sozdanie_bazy.sql.
constexpr std::size_t MAX_NAZVANIE = 50;
constexpr std::size_t MAX_FIO = 80;
constexpr std::size_t MAX_TABEL = 20;

struct Otdel {
    int id = 0;
    std::string nazvanie;
};

struct Sotrudnik {
    int id = 0;
    std::string fio;
    std::string tabel;
    int otdelId = 0;
    int norma = 0; // сколько дней отпуска положено за год
};

struct Otpusk {
    int id = 0;
    std::string dataNach;
    std::string dataKon;
    int sotrId = 0;
    int dney = 0;
};

enum class Rol {
    Sotrudnik, // только смотрит списки
    Kadrovik,  // оформляет, продлевает и удаляет отпуска
    Admin      // все то же плюс прошедшие отпуска
};

struct Polzovatel {
    int id = 0;
    std::string login;
    Rol rol = Rol::Sotrudnik;
};

inline std::string nazvanieRoli(Rol rol) {
    switch (rol) {
    case Rol::Kadrovik: return "кадровик";
    case Rol::Admin: return "админ";
    default: return "сотрудник";
    }
}
