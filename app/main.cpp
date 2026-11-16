#include "konsolnoe_menu.h"
#include "parol.h"
#include "sql_hranilishche.h"
#include "uchet_otpuskov.h"
#include <iostream>
#include <windows.h>

namespace {

bool voiti(Hranilishche& baza, Polzovatel& polzovatel) {
    const int popytok = 3;
    for (int nomer = 1; nomer <= popytok; nomer++) {
        std::cout << "Логин: ";
        std::string login = chitatStroku();
        std::cout << "Пароль: ";
        std::string parol = chitatParol();
        if (baza.vhod(login, heshParolya(parol), polzovatel)) return true;
        std::string oshibka = baza.oshibka();
        if (!oshibka.empty() && oshibka != "Неверный логин или пароль.") {
            std::cout << oshibka << "\n";
            return false;
        }
        std::cout << "Неверный логин или пароль. Осталось попыток: "
                  << popytok - nomer << "\n";
    }
    return false;
}

} // namespace

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    std::cout << "Модуль учета отпусков\n";

    SqlHranilishche baza;
    if (!baza.podklyuchit(serverIzNastroek())) {
        std::cout << baza.oshibka() << "\n";
        std::cout << "Меню не запущено: нет связи с SQL Server.\n";
        return 1;
    }

    Polzovatel polzovatel;
    if (!voiti(baza, polzovatel)) {
        std::cout << "Вход не выполнен.\n";
        return 1;
    }
    std::cout << "Добро пожаловать, " << polzovatel.login
              << ". Роль: " << nazvanieRoli(polzovatel.rol) << "\n";

    UchetOtpuskov uchet(baza, polzovatel.rol);
    if (!uchet.zagruzit()) {
        std::cout << "Не получилось прочитать таблицы. " << baza.oshibka() << "\n";
        return 1;
    }
    std::cout << "База SQL Server прочитана. Отделов: " << uchet.otdely().size()
              << ", сотрудников: " << uchet.sotrudniki().size()
              << ", отпусков: " << uchet.otpuska().size() << "\n";

    KonsolnoeMenu menu(uchet);
    menu.zapustit();
    return 0;
}
