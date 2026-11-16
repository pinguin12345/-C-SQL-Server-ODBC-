#include "parol.h"
#include <windows.h>
#include <wincrypt.h>
#include <iomanip>
#include <sstream>

std::string heshParolya(const std::string& parol) {
    HCRYPTPROV provajder = 0;
    HCRYPTHASH hesh = 0;
    if (!CryptAcquireContextW(&provajder, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        return "";
    }

    std::string rezultat;
    if (CryptCreateHash(provajder, CALG_SHA_256, 0, 0, &hesh)) {
        BYTE znachenie[32];
        DWORD dlina = sizeof(znachenie);
        if (CryptHashData(hesh, reinterpret_cast<const BYTE*>(parol.data()),
                          static_cast<DWORD>(parol.size()), 0)
            && CryptGetHashParam(hesh, HP_HASHVAL, znachenie, &dlina, 0)) {
            std::ostringstream vyvod;
            vyvod << std::uppercase << std::hex << std::setfill('0');
            for (DWORD i = 0; i < dlina; i++) {
                vyvod << std::setw(2) << static_cast<int>(znachenie[i]);
            }
            rezultat = vyvod.str();
        }
        CryptDestroyHash(hesh);
    }
    CryptReleaseContext(provajder, 0);
    return rezultat;
}
