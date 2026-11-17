#include "sql_hranilishche.h"
#include "log.h"
#include "tekst.h"
#include <array>
#include <fstream>

namespace {


class Operator {
public:
    explicit Operator(SQLHDBC soedinenie) {
        SQLAllocHandle(SQL_HANDLE_STMT, soedinenie, &h_);
    }
    ~Operator() {
        if (h_ != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, h_);
    }
    Operator(const Operator&) = delete;
    Operator& operator=(const Operator&) = delete;

    SQLHSTMT h() const { return h_; }

    void chislo(SQLUSMALLINT nomer, int znachenie) {
        chisla_[nomer] = znachenie;
        SQLBindParameter(h_, nomer, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                         0, 0, &chisla_[nomer], 0, nullptr);
    }

    void tekst(SQLUSMALLINT nomer, const std::string& znachenie, SQLULEN razmer) {
        stroki_[nomer] = vShirokuyu(znachenie);
        dliny_[nomer] = static_cast<SQLLEN>(stroki_[nomer].size() * sizeof(wchar_t));
        SQLBindParameter(h_, nomer, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,
                         razmer, 0, const_cast<wchar_t*>(stroki_[nomer].c_str()),
                         dliny_[nomer], &dliny_[nomer]);
    }

    SQLRETURN vypolnit(const wchar_t* zapros) {
        return SQLExecDirectW(h_, const_cast<SQLWCHAR*>(reinterpret_cast<const SQLWCHAR*>(zapros)),
                              SQL_NTS);
    }

private:
    SQLHSTMT h_ = SQL_NULL_HSTMT;
    std::array<SQLINTEGER, 8> chisla_{};
    std::array<std::wstring, 8> stroki_;
    std::array<SQLLEN, 8> dliny_{};
};

std::string izBufera(const SQLWCHAR* bufer, SQLLEN ind) {
    if (ind == SQL_NULL_DATA) return "";
    return vUtf8(reinterpret_cast<const wchar_t*>(bufer));
}

bool estStroka(SQLRETURN kod) {
    return kod == SQL_SUCCESS || kod == SQL_SUCCESS_WITH_INFO;
}

// Порядок важен: на новых компьютерах чаще стоит драйвер 18, на старых - 17.
const wchar_t* const kDraivery[] = {
    L"ODBC Driver 18 for SQL Server",
    L"ODBC Driver 17 for SQL Server",
    L"ODBC Driver 13 for SQL Server",
    L"SQL Server"
};

// IM002 - драйвер не установлен. Остальные ошибки (нет базы, нет сервера,
// неверная учетная запись) означают, что другой драйвер уже не поможет.
bool draiverNeUstanovlen(SQLHDBC soedinenie) {
    SQLWCHAR sostoyanie[8] = {};
    SQLINTEGER nomer = 0;
    SQLSMALLINT dlina = 0;
    if (!estStroka(SQLGetDiagRecW(SQL_HANDLE_DBC, soedinenie, 1, sostoyanie, &nomer,
                                  nullptr, 0, &dlina))) {
        return false;
    }
    return std::wstring(sostoyanie) == L"IM002";
}

std::string papkaProgrammy() {
    wchar_t put[MAX_PATH];
    DWORD dlina = GetModuleFileNameW(nullptr, put, MAX_PATH);
    if (dlina == 0 || dlina == MAX_PATH) return "";
    std::wstring polnyi(put, dlina);
    std::size_t pos = polnyi.find_last_of(L"\\/");
    if (pos == std::wstring::npos) return "";
    return vUtf8(polnyi.substr(0, pos));
}

std::string serverIzFaila(const std::string& imya) {
    std::ifstream fail(imya);
    std::string stroka;
    if (!std::getline(fail, stroka)) return "";
    if (stroka.rfind("\xEF\xBB\xBF", 0) == 0) stroka.erase(0, 3);
    while (!stroka.empty() && (stroka.back() == '\r' || stroka.back() == ' ')) stroka.pop_back();
    while (!stroka.empty() && stroka.front() == ' ') stroka.erase(0, 1);
    if (stroka.empty() || stroka.find(';') != std::string::npos) return "";
    return stroka;
}

} // namespace

SqlHranilishche::~SqlHranilishche() {
    otklyuchit();
}

bool SqlHranilishche::proverit(SQLRETURN kod, SQLSMALLINT tip, SQLHANDLE h,
                               const std::string& gde) {
    if (estStroka(kod)) return true;
    SQLWCHAR sostoyanie[8];
    SQLWCHAR tekst[512];
    SQLINTEGER nomer = 0;
    SQLSMALLINT dlina = 0;
    oshibka_.clear();
    if (estStroka(SQLGetDiagRecW(tip, h, 1, sostoyanie, &nomer, tekst, 512, &dlina))) {
        oshibka_ = vUtf8(reinterpret_cast<const wchar_t*>(tekst));
    }
    if (oshibka_.empty()) oshibka_ = "Ошибка SQL Server";
    logStroka(gde + ": " + oshibka_);
    return false;
}

bool SqlHranilishche::podklyuchit(const std::string& server) {
    if (podklyucheno_) return true;
    oshibka_.clear();

    if (!proverit(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &okruzhenie_),
                  SQL_HANDLE_ENV, okruzhenie_, "среда ODBC")) {
        otklyuchit();
        return false;
    }
    if (!proverit(SQLSetEnvAttr(okruzhenie_, SQL_ATTR_ODBC_VERSION,
                                reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0),
                  SQL_HANDLE_ENV, okruzhenie_, "настройка ODBC")) {
        otklyuchit();
        return false;
    }

    const std::wstring shirokiyServer = vShirokuyu(server);
    for (const wchar_t* draiver : kDraivery) {
        if (soedinenie_ != SQL_NULL_HDBC) {
            SQLFreeHandle(SQL_HANDLE_DBC, soedinenie_);
            soedinenie_ = SQL_NULL_HDBC;
        }
        if (!proverit(SQLAllocHandle(SQL_HANDLE_DBC, okruzhenie_, &soedinenie_),
                      SQL_HANDLE_ENV, okruzhenie_, "создание соединения")
            || !proverit(SQLSetConnectAttr(soedinenie_, SQL_ATTR_LOGIN_TIMEOUT,
                                           reinterpret_cast<SQLPOINTER>(5), SQL_IS_UINTEGER),
                         SQL_HANDLE_DBC, soedinenie_, "время подключения")) {
            otklyuchit();
            return false;
        }

        std::wstring stroka = L"Driver={" + std::wstring(draiver) + L"};Server="
            + shirokiyServer + L";Database=OtpuskKompanii;Trusted_Connection=yes;";
        // Драйвер 18 по умолчанию требует шифрование и проверку сертификата.
        if (std::wstring(draiver).find(L"18") != std::wstring::npos) {
            stroka += L"Encrypt=no;TrustServerCertificate=yes;";
        }

        SQLRETURN kod = SQLDriverConnectW(soedinenie_, nullptr,
                                          reinterpret_cast<SQLWCHAR*>(stroka.data()), SQL_NTS,
                                          nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT);
        if (estStroka(kod)) {
            podklyucheno_ = true;
            logStroka("Подключение к SQL Server, база OtpuskKompanii, сервер " + server
                      + ", драйвер " + vUtf8(draiver));
            return true;
        }
        const bool netDraivera = draiverNeUstanovlen(soedinenie_);
        proverit(kod, SQL_HANDLE_DBC, soedinenie_, "вход в базу");
        if (!netDraivera) break;
    }

    otklyuchit();
    return false;
}

void SqlHranilishche::otklyuchit() {
    if (soedinenie_ != SQL_NULL_HDBC) {
        if (podklyucheno_) SQLDisconnect(soedinenie_);
        SQLFreeHandle(SQL_HANDLE_DBC, soedinenie_);
        soedinenie_ = SQL_NULL_HDBC;
    }
    if (okruzhenie_ != SQL_NULL_HENV) {
        SQLFreeHandle(SQL_HANDLE_ENV, okruzhenie_);
        okruzhenie_ = SQL_NULL_HENV;
    }
    podklyucheno_ = false;
}

bool SqlHranilishche::prochitatOtdely(std::vector<Otdel>& otdely) {
    Operator op(soedinenie_);
    if (!proverit(op.vypolnit(L"SELECT Id, Nazvanie FROM Otdely ORDER BY Id"),
                  SQL_HANDLE_STMT, op.h(), "чтение отделов")) return false;

    SQLINTEGER id = 0;
    SQLWCHAR nazvanie[MAX_NAZVANIE + 1];
    SQLLEN ind[2] = {};
    SQLBindCol(op.h(), 1, SQL_C_SLONG, &id, 0, &ind[0]);
    SQLBindCol(op.h(), 2, SQL_C_WCHAR, nazvanie, sizeof(nazvanie), &ind[1]);

    SQLRETURN kod;
    while (estStroka(kod = SQLFetch(op.h()))) {
        otdely.push_back({id, izBufera(nazvanie, ind[1])});
    }
    return kod == SQL_NO_DATA || proverit(kod, SQL_HANDLE_STMT, op.h(), "строка отдела");
}

bool SqlHranilishche::prochitatSotrudnikov(std::vector<Sotrudnik>& sotrudniki) {
    Operator op(soedinenie_);
    if (!proverit(op.vypolnit(L"SELECT Id, Fio, Tabel, OtdelId, Norma FROM Sotrudniki ORDER BY Id"),
                  SQL_HANDLE_STMT, op.h(), "чтение сотрудников")) return false;

    SQLINTEGER id = 0, otdelId = 0, norma = 0;
    SQLWCHAR fio[MAX_FIO + 1];
    SQLWCHAR tabel[MAX_TABEL + 1];
    SQLLEN ind[5] = {};
    SQLBindCol(op.h(), 1, SQL_C_SLONG, &id, 0, &ind[0]);
    SQLBindCol(op.h(), 2, SQL_C_WCHAR, fio, sizeof(fio), &ind[1]);
    SQLBindCol(op.h(), 3, SQL_C_WCHAR, tabel, sizeof(tabel), &ind[2]);
    SQLBindCol(op.h(), 4, SQL_C_SLONG, &otdelId, 0, &ind[3]);
    SQLBindCol(op.h(), 5, SQL_C_SLONG, &norma, 0, &ind[4]);

    SQLRETURN kod;
    while (estStroka(kod = SQLFetch(op.h()))) {
        sotrudniki.push_back({id, izBufera(fio, ind[1]), izBufera(tabel, ind[2]), otdelId, norma});
    }
    return kod == SQL_NO_DATA || proverit(kod, SQL_HANDLE_STMT, op.h(), "строка сотрудника");
}

bool SqlHranilishche::prochitatOtpuska(std::vector<Otpusk>& otpuska) {
    Operator op(soedinenie_);
    if (!proverit(op.vypolnit(L"SELECT Id, SotrId, DataNachala, DataKonca, Dney FROM Otpuska ORDER BY Id"),
                  SQL_HANDLE_STMT, op.h(), "чтение отпусков")) return false;

    SQLINTEGER id = 0, sotrId = 0, dney = 0;
    SQLWCHAR nach[32];
    SQLWCHAR kon[32];
    SQLLEN ind[5] = {};
    SQLBindCol(op.h(), 1, SQL_C_SLONG, &id, 0, &ind[0]);
    SQLBindCol(op.h(), 2, SQL_C_SLONG, &sotrId, 0, &ind[1]);
    SQLBindCol(op.h(), 3, SQL_C_WCHAR, nach, sizeof(nach), &ind[2]);
    SQLBindCol(op.h(), 4, SQL_C_WCHAR, kon, sizeof(kon), &ind[3]);
    SQLBindCol(op.h(), 5, SQL_C_SLONG, &dney, 0, &ind[4]);

    SQLRETURN kod;
    while (estStroka(kod = SQLFetch(op.h()))) {
        otpuska.push_back({id, izBufera(nach, ind[2]), izBufera(kon, ind[3]), sotrId, dney});
    }
    return kod == SQL_NO_DATA || proverit(kod, SQL_HANDLE_STMT, op.h(), "строка отпуска");
}

bool SqlHranilishche::prochitat(std::vector<Otdel>& otdely,
                                std::vector<Sotrudnik>& sotrudniki,
                                std::vector<Otpusk>& otpuska) {
    if (!podklyucheno_) {
        oshibka_ = "Нет подключения к SQL Server.";
        return false;
    }
    std::vector<Otdel> o;
    std::vector<Sotrudnik> s;
    std::vector<Otpusk> p;
    if (!prochitatOtdely(o) || !prochitatSotrudnikov(s) || !prochitatOtpuska(p)) return false;
    otdely = std::move(o);
    sotrudniki = std::move(s);
    otpuska = std::move(p);
    logStroka("Загрузка из базы: отделов " + std::to_string(otdely.size())
              + ", сотрудников " + std::to_string(sotrudniki.size())
              + ", отпусков " + std::to_string(otpuska.size()));
    return true;
}

bool SqlHranilishche::vhod(const std::string& login, const std::string& hesh,
                           Polzovatel& polzovatel) {
    if (!podklyucheno_) {
        oshibka_ = "Нет подключения к SQL Server.";
        return false;
    }
    Operator op(soedinenie_);
    op.tekst(1, login, 30);
    op.tekst(2, hesh, 64);
    if (!proverit(op.vypolnit(L"SELECT Id, Login, Rol FROM Polzovateli "
                              L"WHERE Login = ? AND ParolHash = ?"),
                  SQL_HANDLE_STMT, op.h(), "вход в программу")) return false;

    SQLINTEGER id = 0;
    SQLWCHAR buferLogin[40];
    SQLWCHAR buferRoli[30];
    SQLLEN ind[3] = {};
    SQLBindCol(op.h(), 1, SQL_C_SLONG, &id, 0, &ind[0]);
    SQLBindCol(op.h(), 2, SQL_C_WCHAR, buferLogin, sizeof(buferLogin), &ind[1]);
    SQLBindCol(op.h(), 3, SQL_C_WCHAR, buferRoli, sizeof(buferRoli), &ind[2]);

    SQLRETURN kod = SQLFetch(op.h());
    if (kod == SQL_NO_DATA) {
        oshibka_ = "Неверный логин или пароль.";
        return false;
    }
    if (!proverit(kod, SQL_HANDLE_STMT, op.h(), "чтение пользователя")) return false;

    std::string rol = izBufera(buferRoli, ind[2]);
    if (rol == "сотрудник") polzovatel.rol = Rol::Sotrudnik;
    else if (rol == "кадровик") polzovatel.rol = Rol::Kadrovik;
    else if (rol == "админ") polzovatel.rol = Rol::Admin;
    else {
        oshibka_ = "В базе неизвестная роль: " + rol;
        return false;
    }
    polzovatel.id = id;
    polzovatel.login = izBufera(buferLogin, ind[1]);
    logStroka("Вход в программу: " + polzovatel.login + ", роль " + rol);
    return true;
}

bool SqlHranilishche::dobavitOtdel(const Otdel& o) {
    Operator op(soedinenie_);
    op.chislo(1, o.id);
    op.tekst(2, o.nazvanie, MAX_NAZVANIE);
    if (!proverit(op.vypolnit(L"INSERT INTO Otdely (Id, Nazvanie) VALUES (?, ?)"),
                  SQL_HANDLE_STMT, op.h(), "запись отдела")) return false;
    logStroka("Добавлен отдел " + std::to_string(o.id));
    return true;
}

bool SqlHranilishche::dobavitSotrudnika(const Sotrudnik& s) {
    Operator op(soedinenie_);
    op.chislo(1, s.id);
    op.tekst(2, s.fio, MAX_FIO);
    op.tekst(3, s.tabel, MAX_TABEL);
    op.chislo(4, s.otdelId);
    op.chislo(5, s.norma);
    if (!proverit(op.vypolnit(L"INSERT INTO Sotrudniki (Id, Fio, Tabel, OtdelId, Norma) "
                              L"VALUES (?, ?, ?, ?, ?)"),
                  SQL_HANDLE_STMT, op.h(), "запись сотрудника")) return false;
    logStroka("Добавлен сотрудник " + std::to_string(s.id));
    return true;
}

bool SqlHranilishche::dobavitOtpusk(const Otpusk& o) {
    Operator op(soedinenie_);
    op.chislo(1, o.id);
    op.chislo(2, o.sotrId);
    op.tekst(3, o.dataNach, 20);
    op.tekst(4, o.dataKon, 20);
    op.chislo(5, o.dney);
    if (!proverit(op.vypolnit(L"INSERT INTO Otpuska (Id, SotrId, DataNachala, DataKonca, Dney) "
                              L"VALUES (?, ?, ?, ?, ?)"),
                  SQL_HANDLE_STMT, op.h(), "запись отпуска")) return false;
    logStroka("Добавлен отпуск " + std::to_string(o.id));
    return true;
}

bool SqlHranilishche::izmenitOtpusk(const Otpusk& o) {
    Operator op(soedinenie_);
    op.tekst(1, o.dataKon, 20);
    op.chislo(2, o.dney);
    op.chislo(3, o.id);
    SQLRETURN kod = op.vypolnit(L"UPDATE Otpuska SET DataKonca = ?, Dney = ? WHERE Id = ?");
    SQLLEN izmeneno = 0;
    if (kod != SQL_NO_DATA) {
        if (!proverit(kod, SQL_HANDLE_STMT, op.h(), "изменение отпуска")) return false;
        SQLRowCount(op.h(), &izmeneno);
    }
    if (izmeneno == 0) {
        oshibka_ = "В базе нет отпуска с номером " + std::to_string(o.id) + ".";
        return false;
    }
    logStroka("Изменен отпуск " + std::to_string(o.id) + ", дней " + std::to_string(o.dney));
    return true;
}

bool SqlHranilishche::udalitOtpusk(int id) {
    Operator op(soedinenie_);
    op.chislo(1, id);
    SQLRETURN kod = op.vypolnit(L"DELETE FROM Otpuska WHERE Id = ?");
    SQLLEN udaleno = 0;
    if (kod != SQL_NO_DATA) {
        if (!proverit(kod, SQL_HANDLE_STMT, op.h(), "удаление отпуска")) return false;
        SQLRowCount(op.h(), &udaleno);
    }
    if (udaleno == 0) {
        oshibka_ = "В базе нет отпуска с номером " + std::to_string(id) + ".";
        return false;
    }
    logStroka("Удален отпуск " + std::to_string(id));
    return true;
}

std::string serverIzNastroek() {
    std::string server = serverIzFaila("server.txt");
    if (!server.empty()) return server;

    // Программу можно запускать и из корня проекта, и двойным щелчком из build.
    std::string papka = papkaProgrammy();
    if (!papka.empty()) {
        server = serverIzFaila(papka + "\\server.txt");
        if (!server.empty()) return server;
    }
    return "localhost";
}
