#pragma once
#include "model.h"
#include <string>
#include <vector>

// Интерфейс хранилища. Каждый метод меняет одну строку в базе,
// таблицы целиком не очищаются и не переписываются.
class Hranilishche {
public:
    virtual ~Hranilishche() = default;

    virtual bool prochitat(std::vector<Otdel>& otdely,
                           std::vector<Sotrudnik>& sotrudniki,
                           std::vector<Otpusk>& otpuska) = 0;

    // Пароль передается уже хешем (heshParolya из parol.h).
    virtual bool vhod(const std::string& login, const std::string& hesh,
                      Polzovatel& polzovatel) = 0;

    virtual bool dobavitOtdel(const Otdel& o) = 0;
    virtual bool dobavitSotrudnika(const Sotrudnik& s) = 0;
    virtual bool dobavitOtpusk(const Otpusk& o) = 0;
    virtual bool izmenitOtpusk(const Otpusk& o) = 0;
    virtual bool udalitOtpusk(int id) = 0;

    virtual std::string oshibka() const = 0;
};
