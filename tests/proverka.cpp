#include "date.h"
#include "hranilishche.h"
#include "parol.h"
#include "uchet_otpuskov.h"
#include <iostream>
#include <windows.h>

// --- Мини-фреймворк для тестов (вместо самописного zhdu) ---
static int vsego_testov = 0;
static int oshibok = 0;

#define CHECK(cond) do { \
    ++vsego_testov; \
    if (!(cond)) { \
        std::cout << "  [FAIL] строка " << __LINE__ << ": " << #cond << "\n"; \
        ++oshibok; \
    } \
} while(0)

#define CHECK_EQ(val, expected) do { \
    ++vsego_testov; \
    auto _val = (val); auto _exp = (expected); \
    if (!(_val == _exp)) { \
        std::cout << "  [FAIL] строка " << __LINE__ << ": " << #val << " == " << #expected << "\n" \
                  << "    получено: " << _val << "\n" \
                  << "    ожидалось: " << _exp << "\n"; \
        ++oshibok; \
    } \
} while(0)

// Хранилище в памяти для тестов
class PamyatHranilishche : public Hranilishche {
public:
    std::vector<Otdel> otdely;
    std::vector<Sotrudnik> sotrudniki;
    std::vector<Otpusk> otpuska;
    std::vector<Polzovatel> polzovateli;
    
    int vstavok_otdel = 0, vstavok_sotr = 0, vstavok_otpusk = 0;
    int izmeneniy = 0;
    int udaleniy = 0;
    bool lomat = false;

    bool prochitat(std::vector<Otdel>& o, std::vector<Sotrudnik>& s, std::vector<Otpusk>& p) override {
        o = otdely; s = sotrudniki; p = otpuska;
        return true;
    }

    bool vhod(const std::string& login, const std::string&, Polzovatel& polzovatel) override {
        for (const Polzovatel& p : polzovateli) {
            if (p.login == login) { polzovatel = p; return true; }
        }
        return false;
    }

    bool dobavitOtdel(const Otdel& o) override { return vstavit(otdely, o, vstavok_otdel); }
    bool dobavitSotrudnika(const Sotrudnik& s) override { return vstavit(sotrudniki, s, vstavok_sotr); }
    bool dobavitOtpusk(const Otpusk& o) override { return vstavit(otpuska, o, vstavok_otpusk); }

    bool izmenitOtpusk(const Otpusk& o) override {
        if (lomat) return false;
        for (Otpusk& x : otpuska) {
            if (x.id == o.id) { x = o; izmeneniy++; return true; }
        }
        return false;
    }

    bool udalitOtpusk(int id) override {
        if (lomat) return false;
        for (std::size_t i = 0; i < otpuska.size(); i++) {
            if (otpuska[i].id == id) {
                otpuska.erase(otpuska.begin() + static_cast<std::ptrdiff_t>(i));
                udaleniy++;
                return true;
            }
        }
        return false;
    }

    std::string oshibka() const override { return lomat ? "база недоступна" : ""; }

private:
    template <typename T>
    bool vstavit(std::vector<T>& tablica, const T& stroka, int& schetchik) {
        if (lomat) return false;
        for (const T& x : tablica) { if (x.id == stroka.id) return false; }
        tablica.push_back(stroka);
        schetchik++;
        return true;
    }
};

// --- Блок 1: Тесты дат ---
void test_dates() {
    std::cout << "Тесты дат:\n";
    CHECK_EQ(skolkoDney("01.06.2027", "14.06.2027"), 14);
    CHECK_EQ(skolkoDney("01.01.2027", "01.01.2027"), 1);
    CHECK_EQ(skolkoDney("10.01.2027", "01.01.2027"), -1);
    CHECK_EQ(skolkoDney("29.02.2024", "01.03.2024"), 2); // високосный
    CHECK_EQ(skolkoDney("01.01.2027", "31.12.2027"), 365);
    CHECK_EQ(skolkoDney("31.12.2026", "01.01.2027"), 2);

    CHECK(dataHoroshaya("29.02.2000"));
    CHECK(dataHoroshaya("31.12.1999"));
    CHECK(dataHoroshaya("01.01.2101"));
    CHECK(dataHoroshaya("01.01.1900"));
    CHECK(!dataHoroshaya("31.02.2027"));
    CHECK(!dataHoroshaya("29.02.2100")); // невисокосный век
    CHECK(!dataHoroshaya("31.04.2027"));
    CHECK(!dataHoroshaya("00.06.2027"));
    CHECK(!dataHoroshaya("01.00.2027"));
    CHECK(!dataHoroshaya("01.13.2027"));
    CHECK(!dataHoroshaya("1x.06.2027"));
    CHECK(!dataHoroshaya("01.06.20x7"));
    CHECK(!dataHoroshaya("01/06/2027"));

    CHECK(dataNePozdnee("01.07.2027", "10.08.2027"));
    CHECK(!dataNePozdnee("10.08.2027", "01.07.2027"));
    CHECK(dataNePozdnee("01.07.2027", "01.07.2027"));
    CHECK(!dataNePozdnee("31.02.2027", "01.01.2027")); // неверная дата
}

// --- Блок 2: Оформление и пересечения ---
void test_otpusk_intersections(UchetOtpuskov& uchet) {
    std::cout << "Пересечения отпусков:\n";
    Otpusk otp;
    CHECK(uchet.oformitOtpusk(2, "01.07.2027", "14.07.2027", otp).empty()); // успешно
    CHECK_EQ(otp.dney, 14);

    CHECK(uchet.oformitOtpusk(2, "07.07.2027", "16.07.2027", otp).find("пересекается") != std::string::npos);
    CHECK(uchet.oformitOtpusk(2, "03.07.2027", "05.07.2027", otp).find("пересекается") != std::string::npos);
    CHECK(uchet.oformitOtpusk(2, "10.06.2027", "05.07.2027", otp).find("пересекается") != std::string::npos);
    
    // Соседние даты не должны пересекаться
    CHECK(uchet.oformitOtpusk(2, "15.07.2027", "20.07.2027", otp).empty());
}

// --- Блок 3: Продление и удаление ---
void test_prodlenie_i_udalenie(UchetOtpuskov& uchet, PamyatHranilishche& baza) {
    std::cout << "Продление и удаление:\n";
    Otpusk prodl;
    CHECK(!uchet.prodlenieOtpuska(3, "31.02.2027", prodl).empty()); // неверная дата
    CHECK_EQ(uchet.prodlenieOtpuska(99, "20.07.2027", prodl), "Такого отпуска нет.");
    CHECK(uchet.prodlenieOtpuska(4, "25.07.2027", prodl).empty()); // успех
    CHECK_EQ(prodl.dney, 11);
    CHECK_EQ(baza.izmeneniy, 1);

    CHECK_EQ(uchet.udalitOtpusk(1), "");
    CHECK_EQ(baza.udaleniy, 1);
    CHECK_EQ(uchet.udalitOtpusk(1), "Такого отпуска нет.");
}

// --- Блок 4: Отказ базы данных ---
void test_oshibki_bd(UchetOtpuskov& uchet, PamyatHranilishche& baza) {
    std::cout << "Ошибки БД:\n";
    baza.lomat = true;
    Otpusk prodl;
    CHECK(!uchet.prodlenieOtpuska(2, "28.07.2027", prodl).empty()); // ошибка
    CHECK_EQ(uchet.otpuska().back().dataKon, "20.07.2027"); // память не изменилась

    std::size_t bylo = uchet.otdely().size();
    Otdel otdel;
    CHECK(!uchet.dobavitOtdel("Охрана", otdel).empty());
    CHECK_EQ(uchet.otdely().size(), bylo); // память не изменилась
    baza.lomat = false;
}

// --- Блок 5: Роли ---
void test_roles() {
    std::cout << "Роли и пароли:\n";
    CHECK_EQ(heshParolya("admin123"), "240BE518FABD2724DDB6F04EEB1DA5967448D7E831C08C8FA822809F74C720A9");

    PamyatHranilishche baza;
    baza.otdely = {{1, "Бухгалтерия"}};
    baza.sotrudniki = {{1, "Иванов", "1001", 1, 28}};
    baza.otpuska = {{1, "01.03.2020", "14.03.2020", 1, 14}};
    baza.polzovateli = {{1, "admin", Rol::Admin}};

    // Сотрудник
    UchetOtpuskov sotrudnik(baza, Rol::Sotrudnik);
    sotrudnik.zagruzit();
    CHECK(!sotrudnik.mozhnoPisat());
    Otpusk otp;
    CHECK_EQ(sotrudnik.oformitOtpusk(1, "01.03.2030", "14.03.2030", otp),
             "Роль «сотрудник» может только смотреть списки.");

    // Кадровик
    UchetOtpuskov kadrovik(baza, Rol::Kadrovik);
    kadrovik.zagruzit();
    CHECK(kadrovik.mozhnoPisat());
    CHECK(kadrovik.oformitOtpusk(1, "01.03.2030", "14.03.2030", otp).empty()); // будущий можно
    CHECK(!kadrovik.oformitOtpusk(1, "01.03.2020", "14.03.2020", otp).empty()); // прошедший нельзя

    // Админ
    UchetOtpuskov admin(baza, Rol::Admin);
    admin.zagruzit();
    CHECK(admin.oformitOtpusk(1, "01.03.2020", "14.03.2020", otp).empty()); // прошедший можно
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    std::cout << "=== ЗАПУСК ТЕСТОВ ===\n\n";

    test_dates();
    test_roles();
    
    // Общая база для тестов учета
    PamyatHranilishche baza;
    baza.otdely = {{1, "Бухгалтерия"}, {2, "Отдел продаж"}};
    baza.sotrudniki = {{1, "Иванов", "1001", 1, 28}, {2, "Петрова", "1002", 2, 28}};
    baza.otpuska = {{1, "01.06.2027", "14.06.2027", 2, 14}};

    UchetOtpuskov uchet(baza, Rol::Admin);
    uchet.zagruzit();
    
    Otdel otdel;
    Sotrudnik sotr;
    CHECK(uchet.dobavitOtdel("Склад", otdel).empty());
    CHECK_EQ(otdel.id, 3);
    CHECK(uchet.dobavitSotrudnika("Волкова", "1003", 3, 28, sotr).empty());
    CHECK_EQ(sotr.id, 3);

    test_otpusk_intersections(uchet);
    test_prodlenie_i_udalenie(uchet, baza);
    test_oshibki_bd(uchet, baza);

    std::cout << "\n=== ИТОГО ===\n";
    std::cout << "Всего проверок: " << vsego_testov << "\n";
    std::cout << "Ошибок: " << oshibok << "\n";
    return oshibok == 0 ? 0 : 1;
}