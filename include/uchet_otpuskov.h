#pragma once
#include "hranilishche.h"
#include "model.h"
#include <string>
#include <vector>

// Правила учета отпусков. Ничего не печатает и не читает с клавиатуры.
// Методы изменения возвращают пустую строку при успехе, иначе причину отказа.
// Сначала запись в хранилище, и только после успеха меняется список в памяти.
class UchetOtpuskov {
public:
    static constexpr int MIN_DNEY_CHASTI = 14;
    static constexpr int MAX_NORMA = 40;

    explicit UchetOtpuskov(Hranilishche& hranilishche, Rol rol);

    Rol rol() const { return rol_; }
    bool mozhnoPisat() const;

    bool zagruzit();

    const std::vector<Otdel>& otdely() const { return otdely_; }
    const std::vector<Sotrudnik>& sotrudniki() const { return sotrudniki_; }
    const std::vector<Otpusk>& otpuska() const { return otpuska_; }

    const Otdel* najtiOtdel(int id) const;
    const Sotrudnik* najtiSotrudnika(int id) const;
    std::string nazvanieOtdela(int id) const;
    std::string fioSotrudnika(int id) const;

    int zanyatoDney(int sotrId) const;
    int ostatokDney(int sotrId) const;
    std::string pochemuNelzya(int sotrId, int dney) const;

    std::string proveritNazvanieOtdela(const std::string& nazvanie) const;
    std::string proveritFio(const std::string& fio) const;
    std::string proveritTabel(const std::string& tabel) const;
    std::string proveritOtdel(int otdelId) const;
    std::string proveritNormu(int norma) const;

    std::string dobavitOtdel(const std::string& nazvanie, Otdel& novyi);
    std::string dobavitSotrudnika(const std::string& fio, const std::string& tabel,
                                  int otdelId, int norma, Sotrudnik& novyi);
    std::string oformitOtpusk(int sotrId, const std::string& nach,
                              const std::string& kon, Otpusk& novyi);
    std::string prodlenieOtpuska(int id, const std::string& novyiKonec, Otpusk& izmenennyi);
    std::string udalitOtpusk(int id);

private:
    std::string proveritPravoZapisi() const;
    std::string proveritProshluyuDatu(const std::string& dataKon) const;
    std::string proveritPeresechenie(int sotrId, const std::string& nach,
                                     const std::string& kon, int kromeId) const;

    Hranilishche& hranilishche_;
    Rol rol_;
    std::vector<Otdel> otdely_;
    std::vector<Sotrudnik> sotrudniki_;
    std::vector<Otpusk> otpuska_;
};
