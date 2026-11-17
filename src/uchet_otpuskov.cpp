#include "uchet_otpuskov.h"
#include "date.h"
#include "log.h"
#include "tekst.h"
#include <algorithm>

namespace {

template <typename T>
int noviyId(const std::vector<T>& spisok) {
    int m = 0;
    for (const T& x : spisok) m = std::max(m, x.id);
    return m + 1;
}

template <typename T>
const T* najti(const std::vector<T>& spisok, int id) {
    for (const T& x : spisok) {
        if (x.id == id) return &x;
    }
    return nullptr;
}

std::string slishkomDlinno(const std::string& chto, std::size_t predel) {
    return chto + " длиннее " + std::to_string(predel) + " символов.";
}


bool datiPeresekayutsya(const std::string& nach1, const std::string& kon1,
                        const std::string& nach2, const std::string& kon2) {
    return dataNePozdnee(nach1, kon2) && dataNePozdnee(nach2, kon1);
}

}

UchetOtpuskov::UchetOtpuskov(Hranilishche& hranilishche, Rol rol)
    : hranilishche_(hranilishche), rol_(rol) {}

bool UchetOtpuskov::mozhnoPisat() const {
    return rol_ == Rol::Kadrovik || rol_ == Rol::Admin;
}

std::string UchetOtpuskov::proveritPravoZapisi() const {
    if (!mozhnoPisat()) return "Роль «сотрудник» может только смотреть списки.";
    return "";
}

// Прошедшие отпуска и их правку разрешено вносить только администратору.
std::string UchetOtpuskov::proveritProshluyuDatu(const std::string& dataKon) const {
    if (rol_ == Rol::Admin) return "";
    if (dataRanshe(dataKon, segodnya())) {
        return "Прошедший отпуск может оформить или изменить только администратор.";
    }
    return "";
}

bool UchetOtpuskov::zagruzit() {
    return hranilishche_.prochitat(otdely_, sotrudniki_, otpuska_);
}

const Otdel* UchetOtpuskov::najtiOtdel(int id) const {
    return najti(otdely_, id);
}

const Sotrudnik* UchetOtpuskov::najtiSotrudnika(int id) const {
    return najti(sotrudniki_, id);
}

std::string UchetOtpuskov::nazvanieOtdela(int id) const {
    const Otdel* o = najtiOtdel(id);
    return o ? o->nazvanie : "нет отдела";
}

std::string UchetOtpuskov::fioSotrudnika(int id) const {
    const Sotrudnik* s = najtiSotrudnika(id);
    return s ? s->fio : "нет сотрудника";
}

int UchetOtpuskov::zanyatoDney(int sotrId) const {
    int summa = 0;
    for (const Otpusk& o : otpuska_) {
        if (o.sotrId == sotrId) summa += o.dney;
    }
    return summa;
}

int UchetOtpuskov::ostatokDney(int sotrId) const {
    const Sotrudnik* s = najtiSotrudnika(sotrId);
    if (!s) return -1;
    return s->norma - zanyatoDney(sotrId);
}

std::string UchetOtpuskov::pochemuNelzya(int sotrId, int dney) const {
    if (!najtiSotrudnika(sotrId)) return "Сотрудник не найден.";
    if (dney < 1) return "Дней должно быть больше нуля.";
    int ostatok = ostatokDney(sotrId);
    if (dney > ostatok) {
        return "Не хватает дней. Осталось " + std::to_string(ostatok)
            + ", а запрошено " + std::to_string(dney) + ".";
    }
    return "";
}

std::string UchetOtpuskov::proveritNazvanieOtdela(const std::string& nazvanie) const {
    if (nazvanie.empty()) return "Название пустое, отдел не добавлен.";
    if (kolichestvoSimvolov(nazvanie) > MAX_NAZVANIE) return slishkomDlinno("Название", MAX_NAZVANIE);
    return "";
}

std::string UchetOtpuskov::proveritFio(const std::string& fio) const {
    if (fio.empty()) return "ФИО введено неверно.";
    if (kolichestvoSimvolov(fio) > MAX_FIO) return slishkomDlinno("ФИО", MAX_FIO);
    return "";
}

std::string UchetOtpuskov::proveritTabel(const std::string& tabel) const {
    if (tabel.empty()) return "Табельный номер пустой.";
    if (kolichestvoSimvolov(tabel) > MAX_TABEL) return slishkomDlinno("Табельный номер", MAX_TABEL);
    for (const Sotrudnik& s : sotrudniki_) {
        if (s.tabel == tabel) return "Такой табельный номер уже есть.";
    }
    return "";
}

std::string UchetOtpuskov::proveritOtdel(int otdelId) const {
    return najtiOtdel(otdelId) ? "" : "Такого отдела нет.";
}

std::string UchetOtpuskov::proveritNormu(int norma) const {
    if (norma < 1 || norma > MAX_NORMA) {
        return "Можно указать от 1 до " + std::to_string(MAX_NORMA) + " дней.";
    }
    return "";
}

std::string UchetOtpuskov::dobavitOtdel(const std::string& nazvanie, Otdel& novyi) {
    std::string prichina = proveritPravoZapisi();
    if (prichina.empty()) prichina = proveritNazvanieOtdela(nazvanie);
    if (!prichina.empty()) return prichina;

    Otdel o{noviyId(otdely_), nazvanie};
    if (!hranilishche_.dobavitOtdel(o)) {
        return "Не получилось записать отдел в базу. " + hranilishche_.oshibka();
    }
    otdely_.push_back(o);
    novyi = o;
    return "";
}

std::string UchetOtpuskov::dobavitSotrudnika(const std::string& fio, const std::string& tabel,
                                             int otdelId, int norma, Sotrudnik& novyi) {
    std::string pravo = proveritPravoZapisi();
    if (!pravo.empty()) return pravo;
    for (const std::string& prichina : {proveritFio(fio), proveritTabel(tabel),
                                        proveritOtdel(otdelId), proveritNormu(norma)}) {
        if (!prichina.empty()) return prichina;
    }

    Sotrudnik s{noviyId(sotrudniki_), fio, tabel, otdelId, norma};
    if (!hranilishche_.dobavitSotrudnika(s)) {
        return "Не получилось записать сотрудника в базу. " + hranilishche_.oshibka();
    }
    sotrudniki_.push_back(s);
    novyi = s;
    return "";
}

std::string UchetOtpuskov::proveritPeresechenie(int sotrId, const std::string& nach,
                                                const std::string& kon, int kromeId) const {
    for (const Otpusk& o : otpuska_) {
        if (o.sotrId != sotrId || o.id == kromeId) continue;
        if (datiPeresekayutsya(nach, kon, o.dataNach, o.dataKon)) {
            return "Отпуск пересекается с отпуском №" + std::to_string(o.id)
                + " (" + o.dataNach + " — " + o.dataKon + ").";
        }
    }
    return "";
}

std::string UchetOtpuskov::oformitOtpusk(int sotrId, const std::string& nach,
                                         const std::string& kon, Otpusk& novyi) {
    std::string prichina = proveritPravoZapisi();
    if (!prichina.empty()) return prichina;

    // длительность считает программа, пользователь дни не вводит
    int dney = skolkoDney(nach, kon);
    if (dney < 0) prichina = "Даты неверные или конец раньше начала.";
    if (prichina.empty()) prichina = proveritProshluyuDatu(kon);
    // Пересечение проверяем раньше остатка: это более точная причина отказа.
    if (prichina.empty()) prichina = proveritPeresechenie(sotrId, nach, kon, 0);
    if (prichina.empty()) prichina = pochemuNelzya(sotrId, dney);
    if (!prichina.empty()) {
        logStroka("Отказ в отпуске: " + prichina);
        return prichina;
    }

    Otpusk o{noviyId(otpuska_), nach, kon, sotrId, dney};
    if (!hranilishche_.dobavitOtpusk(o)) {
        return "Не получилось записать отпуск в базу. " + hranilishche_.oshibka();
    }
    otpuska_.push_back(o);
    novyi = o;
    return "";
}

std::string UchetOtpuskov::prodlenieOtpuska(int id, const std::string& novyiKonec,
                                            Otpusk& izmenennyi) {
    std::string prichina = proveritPravoZapisi();
    if (!prichina.empty()) return prichina;

    auto it = std::find_if(otpuska_.begin(), otpuska_.end(),
                           [id](const Otpusk& o) { return o.id == id; });
    if (it == otpuska_.end()) return "Такого отпуска нет.";

    const Otpusk staryi = *it;
    prichina = proveritProshluyuDatu(staryi.dataKon);
    if (prichina.empty() && !dataHoroshaya(novyiKonec)) {
        prichina = "Дата конца неверная.";
    }
    if (prichina.empty() && dataNePozdnee(novyiKonec, staryi.dataKon)) {
        prichina = "Новая дата конца не позже текущей.";
    }

    Otpusk novyi = staryi;
    if (prichina.empty()) {
        novyi.dataKon = novyiKonec;
        novyi.dney = skolkoDney(staryi.dataNach, novyiKonec);
        int ostatok = ostatokDney(staryi.sotrId);
        int dobavka = novyi.dney - staryi.dney;
        if (dobavka > ostatok) {
            prichina = "Не хватает дней. Осталось " + std::to_string(ostatok)
                + ", а нужно еще " + std::to_string(dobavka) + ".";
        }
    }
    if (prichina.empty()) {
        prichina = proveritPeresechenie(staryi.sotrId, staryi.dataNach, novyiKonec, id);
    }
    if (!prichina.empty()) {
        logStroka("Отказ в продлении отпуска " + std::to_string(id) + ": " + prichina);
        return prichina;
    }

    if (!hranilishche_.izmenitOtpusk(novyi)) {
        return "Не получилось изменить отпуск в базе. " + hranilishche_.oshibka();
    }
    *it = novyi;
    izmenennyi = novyi;
    return "";
}

std::string UchetOtpuskov::udalitOtpusk(int id) {
    std::string prichina = proveritPravoZapisi();
    if (!prichina.empty()) return prichina;

    auto it = std::find_if(otpuska_.begin(), otpuska_.end(),
                           [id](const Otpusk& o) { return o.id == id; });
    if (it == otpuska_.end()) return "Такого отпуска нет.";
    if (!hranilishche_.udalitOtpusk(id)) {
        return "Не получилось удалить отпуск из базы. " + hranilishche_.oshibka();
    }
    otpuska_.erase(it);
    return "";
}
