#include <iostream>
#include <windows.h>
#include <vector>

/**
 * Prefixele utilizate:
 * h     - Handle (ex: hKey)
 * l     - Long (rezultat de tip LSTATUS)
 * sz    - String (Zero-terminated)
 * dw    - Double Word (32-bit unsigned integer)
 * c     - Count
 */

int main() {
    HKEY hRootKey = HKEY_CURRENT_USER;
    LPCSTR szSubKeyPath = "Software"; //Cheia la alegere, nu a fost specificat sa o luam de la intrarea standard
    HKEY hKey;

    std::cout << "Se citesc subcheile pentru: HKEY_CURRENT_USER\\" << szSubKeyPath << "\n" << std::endl;

    //Deschid cheia de registru
    LSTATUS lResult = RegOpenKeyExA(hRootKey, szSubKeyPath, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            std::cerr << "Cheia de registru not found." << std::endl;
        }
        else {
            std::cerr << "Eroare la deschiderea cheii. Cod: " << lResult << std::endl;
        }
        return 1;
    }

    DWORD dwSubKeysCount = 0;
    DWORD dwMaxSubKeyLen = 0;

    //Obtinem informatii despre numarul de subchei si lungimea maxima a numelui
    lResult = RegQueryInfoKey(
        hKey, NULL, NULL, NULL, &dwSubKeysCount, &dwMaxSubKeyLen,
        NULL, NULL, NULL, NULL, NULL, NULL
    );

    if (lResult == ERROR_SUCCESS) {
        //Alocam un buffer pentru numele subcheii (+1 pentru null)
        std::vector<char> szNameBuffer(dwMaxSubKeyLen + 1);

        for (DWORD dwIndex = 0; dwIndex < dwSubKeysCount; dwIndex++) {
            DWORD dwNameSize = dwMaxSubKeyLen + 1;

            //Enumeram subcheile una cate una folosind indexul
            lResult = RegEnumKeyExA(
                hKey, dwIndex, szNameBuffer.data(), &dwNameSize,
                NULL, NULL, NULL, NULL
            );

            if (lResult == ERROR_SUCCESS) {
                std::cout << "[" << dwIndex << "] " << szNameBuffer.data() << std::endl;
            }
        }
    }

    //Inchid handle-ul pentru a elibera memoria (FREE)
    RegCloseKey(hKey);

    std::cout << "\nEnumerare finalizata. Total subchei gasite: " << dwSubKeysCount << std::endl;

    return 0;
}
