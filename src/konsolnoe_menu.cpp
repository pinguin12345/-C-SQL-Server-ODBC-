#include "konsolnoe_menu.h"
#include "tekst.h"
#include <iostream>
#include <windows.h>

namespace {

bool chitatChislo(int& chislo) {
    if (std::cin >> chislo) return true;
    if (std::cin.eof()) return false;
    std::cin.clear();
    std::cin.ignore(1000, '\n');
    std::cout << "Нужно ввести число.\n";
    return false;
}

// Печатает причину отказа и возвращает true, если отказа не было.
bool bezOshibki(const std::string& prichina) {
    if (prichina.empty()) return true;
    std::cout << prichina << "\n";
    return false;
}

} // namespace

std::string chitatStroku() {
    std::cout.flush();
    HANDLE vhod = GetStdHandle(STD_INPUT_HANDLE);
    DWORD rezhim;
    if (!GetConsoleMode(vhod, &rezhim)) {
        std::string tekst;
        std::getline(std::cin, tekst);
        if (!tekst.empty() && tekst.back() == '\r') tekst.pop_back();
        if (tekst.find('\0') != std::string::npos) return "";
        return tekst;
    }

    std::wstring stroka;
    wchar_t chast[1024];
    DWORD kol = 0;
    do {
        if (!ReadConsoleW(vhod, chast, 1024, &kol, nullptr) || kol == 0) return "";
        stroka.append(chast, kol);
    } while (stroka.back() != L'\n');
    while (!stroka.empty() && (stroka.back() == L'\n' || stroka.back() == L'\r')) {
        stroka.pop_back();
    }
    return vUtf8(stroka);
}

std::string chitatParol() {
    HANDLE vhod = GetStdHandle(STD_INPUT_HANDLE);
    DWORD rezhim;
    if (!GetConsoleMode(vhod, &rezhim)) return chitatStroku();

    SetConsoleMode(vhod, rezhim & ~ENABLE_ECHO_INPUT);
    std::string parol = chitatStroku();
    SetConsoleMode(vhod, rezhim);
    std::cout << "\n";
    return parol;
}

KonsolnoeMenu::KonsolnoeMenu(UchetOtpuskov& uchet) : uchet_(uchet) {}

void KonsolnoeMenu::zapustit() {
    while (true) {
        pokazatPunkty();
        std::cout << "Пункт: ";
        int punkt = -1;
        if (!chitatChislo(punkt)) {
            if (std::cin.eof()) return;
            continue;
        }
        switch (punkt) {
        case 0:
            std::cout << "Выход. Все изменения уже записаны в базу.\n";
            return;
        case 1: pokazatOtdely(); break;
        case 2:
        case 4:
        case 5:
        case 8:
        case 9:
            if (!uchet_.mozhnoPisat()) {
                std::cout << "Эта роль может только смотреть списки.\n";
                break;
            }
            if (punkt == 2) dobavitOtdel();
            else if (punkt == 4) dobavitSotrudnika();
            else if (punkt == 5) oformitOtpusk();
            else if (punkt == 8) udalitOtpusk();
            else dobavitDni();
            break;
        case 3: pokazatSotrudnikov(); break;
        case 6: pokazatOtpuska(); break;
        case 7: pokazatOstatok(); break;
        default: std::cout << "Нет такого пункта.\n";
        }
    }
}

void KonsolnoeMenu::pokazatPunkty() const {
    std::cout << "\n===== Учет отпусков, роль: " << nazvanieRoli(uchet_.rol())
              << " =====\n"
              << "1. Список отделов\n";
    if (uchet_.mozhnoPisat()) {
        std::cout << "2. Добавить отдел\n";
    }
    std::cout << "3. Список сотрудников\n";
    if (uchet_.mozhnoPisat()) {
        std::cout << "4. Добавить сотрудника\n"
                  << "5. Оформить отпуск\n";
    }
    std::cout << "6. Список отпусков\n"
              << "7. Остаток дней\n";
    if (uchet_.mozhnoPisat()) {
        std::cout << "8. Удалить отпуск\n"
                  << "9. Добавить дни отпуска\n";
    }
    std::cout << "0. Выход\n";
}

void KonsolnoeMenu::pokazatOtdely() const {
    if (uchet_.otdely().empty()) {
        std::cout << "Отделов пока нет.\n";
        return;
    }
    std::cout << "\nНомер | Отдел\n";
    for (const Otdel& o : uchet_.otdely()) {
        std::cout << o.id << " | " << o.nazvanie << "\n";
    }
}

void KonsolnoeMenu::dobavitOtdel() {
    std::cout << "Название отдела: ";
    std::cin.ignore(1000, '\n');
    std::string nazvanie = chitatStroku();

    Otdel novyi;
    if (bezOshibki(uchet_.dobavitOtdel(nazvanie, novyi))) {
        std::cout << "Отдел добавлен, номер " << novyi.id << "\n";
    }
}

void KonsolnoeMenu::pokazatSotrudnikov() const {
    if (uchet_.sotrudniki().empty()) {
        std::cout << "Сотрудников нет.\n";
        return;
    }
    std::cout << "\nНомер | Табель | ФИО | Отдел | Дней в году\n";
    for (const Sotrudnik& s : uchet_.sotrudniki()) {
        std::cout << s.id << " | " << s.tabel << " | " << s.fio << " | "
                  << uchet_.nazvanieOtdela(s.otdelId) << " | " << s.norma << "\n";
    }
}

void KonsolnoeMenu::dobavitSotrudnika() {
    if (uchet_.otdely().empty()) {
        std::cout << "Сначала добавьте отдел.\n";
        return;
    }

    std::cout << "ФИО: ";
    std::cin.ignore(1000, '\n');
    std::string fio = chitatStroku();
    if (!bezOshibki(uchet_.proveritFio(fio))) return;

    std::cout << "Табельный номер: ";
    std::string tabel = chitatStroku();
    if (!bezOshibki(uchet_.proveritTabel(tabel))) return;

    pokazatOtdely();
    std::cout << "Номер отдела: ";
    int otdelId = 0;
    if (!chitatChislo(otdelId) || !bezOshibki(uchet_.proveritOtdel(otdelId))) return;

    std::cout << "Сколько дней отпуска в году (обычно 28): ";
    int norma = 0;
    if (!chitatChislo(norma)) return;

    Sotrudnik novyi;
    if (bezOshibki(uchet_.dobavitSotrudnika(fio, tabel, otdelId, norma, novyi))) {
        std::cout << "Сотрудник добавлен, номер " << novyi.id << "\n";
    }
}

void KonsolnoeMenu::oformitOtpusk() {
    if (uchet_.sotrudniki().empty()) {
        std::cout << "Сначала добавьте сотрудника.\n";
        return;
    }

    std::cout << "Номер сотрудника: ";
    int sotrId = 0;
    if (!chitatChislo(sotrId)) return;

    std::string nach, kon;
    std::cout << "Дата начала (например 01.07.2027): ";
    std::cin >> nach;
    std::cout << "Дата конца (например 14.07.2027): ";
    std::cin >> kon;

    Otpusk novyi;
    if (!bezOshibki(uchet_.oformitOtpusk(sotrId, nach, kon, novyi))) return;

    std::cout << "Программа насчитала дней: " << novyi.dney << "\n";
    // по ТК одна часть обычно не меньше 14 дней, здесь только напоминание
    if (novyi.dney < UchetOtpuskov::MIN_DNEY_CHASTI) {
        std::cout << "Напоминание: хотя бы одна часть отпуска должна быть от "
                  << UchetOtpuskov::MIN_DNEY_CHASTI << " дней.\n";
    }
    std::cout << "Отпуск записан, номер " << novyi.id << ".\n";
    std::cout << "Остаток дней: " << uchet_.ostatokDney(sotrId) << "\n";
}

void KonsolnoeMenu::pokazatOtpuska() const {
    if (uchet_.otpuska().empty()) {
        std::cout << "Отпусков нет.\n";
        return;
    }
    std::cout << "\nНомер | Сотрудник | С | По | Дней\n";
    for (const Otpusk& o : uchet_.otpuska()) {
        std::cout << o.id << " | " << uchet_.fioSotrudnika(o.sotrId) << " | "
                  << o.dataNach << " | " << o.dataKon << " | " << o.dney << "\n";
    }
}

void KonsolnoeMenu::pokazatOstatok() const {
    std::cout << "Номер сотрудника: ";
    int sotrId = 0;
    if (!chitatChislo(sotrId)) return;

    const Sotrudnik* s = uchet_.najtiSotrudnika(sotrId);
    if (!s) {
        std::cout << "Сотрудник не найден.\n";
        return;
    }
    std::cout << s->fio << "\n"
              << "Положено: " << s->norma << "\n"
              << "Уже записано: " << uchet_.zanyatoDney(sotrId) << "\n"
              << "Остаток: " << uchet_.ostatokDney(sotrId) << "\n";
}

void KonsolnoeMenu::dobavitDni() {
    if (uchet_.otpuska().empty()) {
        std::cout << "Отпусков нет.\n";
        return;
    }

    pokazatOtpuska();
    std::cout << "Номер отпуска: ";
    int id = 0;
    if (!chitatChislo(id)) return;

    std::cout << "Новая дата конца (например 20.07.2027): ";
    std::string konec;
    std::cin >> konec;

    Otpusk izmenennyi;
    if (!bezOshibki(uchet_.prodlenieOtpuska(id, konec, izmenennyi))) return;

    std::cout << "Отпуск продлен до " << izmenennyi.dataKon
              << ", всего дней: " << izmenennyi.dney << ".\n";
    std::cout << "Остаток дней: " << uchet_.ostatokDney(izmenennyi.sotrId) << "\n";
}

void KonsolnoeMenu::udalitOtpusk() {
    std::cout << "Номер отпуска, который нужно удалить: ";
    int id = 0;
    if (!chitatChislo(id)) return;
    if (bezOshibki(uchet_.udalitOtpusk(id))) {
        std::cout << "Отпуск удален.\n";
    }
}
