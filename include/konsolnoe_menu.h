#pragma once
#include "uchet_otpuskov.h"
#include <string>

// Чтение строки с клавиатуры. Узкий ввод UTF-8 в консоли Windows
// заменяет русские буквы нулями, поэтому строка читается через ReadConsoleW.
std::string chitatStroku();

// То же, но ввод пароля не показывается на экране.
std::string chitatParol();

// Консольный интерфейс: ввод, вывод и вызов методов UchetOtpuskov.
class KonsolnoeMenu {
public:
    explicit KonsolnoeMenu(UchetOtpuskov& uchet);
    void zapustit();

private:
    void pokazatPunkty() const;
    void pokazatOtdely() const;
    void dobavitOtdel();
    void pokazatSotrudnikov() const;
    void dobavitSotrudnika();
    void oformitOtpusk();
    void pokazatOtpuska() const;
    void pokazatOstatok() const;
    void dobavitDni();
    void udalitOtpusk();

    UchetOtpuskov& uchet_;
};
