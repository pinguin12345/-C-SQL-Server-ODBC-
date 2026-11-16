#pragma once
#include "hranilishche.h"
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>

// Хранилище в базе OtpuskKompanii на SQL Server через ODBC.
class SqlHranilishche : public Hranilishche {
public:
    SqlHranilishche() = default;
    ~SqlHranilishche() override;
    SqlHranilishche(const SqlHranilishche&) = delete;
    SqlHranilishche& operator=(const SqlHranilishche&) = delete;

    bool podklyuchit(const std::string& server);
    void otklyuchit();

    bool prochitat(std::vector<Otdel>& otdely,
                   std::vector<Sotrudnik>& sotrudniki,
                   std::vector<Otpusk>& otpuska) override;

    bool vhod(const std::string& login, const std::string& hesh,
              Polzovatel& polzovatel) override;

    bool dobavitOtdel(const Otdel& o) override;
    bool dobavitSotrudnika(const Sotrudnik& s) override;
    bool dobavitOtpusk(const Otpusk& o) override;
    bool izmenitOtpusk(const Otpusk& o) override;
    bool udalitOtpusk(int id) override;

    std::string oshibka() const override { return oshibka_; }

private:
    bool proverit(SQLRETURN kod, SQLSMALLINT tip, SQLHANDLE h, const std::string& gde);
    bool prochitatOtdely(std::vector<Otdel>& otdely);
    bool prochitatSotrudnikov(std::vector<Sotrudnik>& sotrudniki);
    bool prochitatOtpuska(std::vector<Otpusk>& otpuska);

    SQLHENV okruzhenie_ = SQL_NULL_HENV;
    SQLHDBC soedinenie_ = SQL_NULL_HDBC;
    bool podklyucheno_ = false;
    std::string oshibka_;
};

// Первая строка server.txt или lpc:localhost, если файла нет.
std::string serverIzNastroek();
