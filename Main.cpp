#include <iostream>
#include <windows.h>
#include <thread>
#include <conio.h>
SYSTEMTIME startRealTime, fixedTime;

void RequestAdminPrivileges() {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
        &AdministratorsGroup)) {
        CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
        FreeSid(AdministratorsGroup);
    }
    if (!isAdmin) {
        char szPath[MAX_PATH];
        GetModuleFileName(NULL, szPath, MAX_PATH);
        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
        sei.lpVerb = "runas";
        sei.lpFile = szPath; // Programın kendi dosya yolu
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;
        if (!ShellExecuteEx(&sei)) {
            std::cerr << "Yönetici izni alınamadı!" << std::endl;
            exit(1);
        }
        exit(0);
    }
}

int main() {
    setlocale(LC_ALL, "Turkish");
    RequestAdminPrivileges();

    GetLocalTime(&startRealTime);
    GetLocalTime(&fixedTime);

    if (fixedTime.wMinute >= 10) {
        fixedTime.wMinute -= 10;
    }
    else {
        fixedTime.wMinute = 60 - (10 - fixedTime.wMinute);
        fixedTime.wHour -= 1;
        if (fixedTime.wHour < 0)
            fixedTime.wHour = 23;
    }

    if (!SetLocalTime(&fixedTime)) {
        std::cerr << "[X] Saat değiştirme başarısız! Yönetici olarak çalıştırmayı dene. Hata kodu: " << GetLastError() << "\n";
        return 1;
    }

    std::cout << "[i] Çıkarken zaman otomatik olarak güncel gerçek zamana ayarlanacak\n";
    std::cout << "[i] Çıkmak için herhangi bir tuşa basın.\n";

    while (true) {
        std::cout << "[i] Sabit Saat: "
            << fixedTime.wHour << ":" << fixedTime.wMinute << ":" << fixedTime.wSecond
            << " | Gerçek Saat: " << startRealTime.wHour << ":" << startRealTime.wMinute << ":" << startRealTime.wSecond << "  \r";
        SetLocalTime(&fixedTime);
        startRealTime.wSecond++;
        if (startRealTime.wSecond >= 60) {
            startRealTime.wSecond -= 60;
            startRealTime.wMinute++;
            if (startRealTime.wMinute >= 60) {
                startRealTime.wMinute -= 60;
                startRealTime.wHour++;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (_kbhit()) { // Kullanıcı bir tuşa basarsa çık
            _getch();
            if (SetLocalTime(&startRealTime)) {
                std::cout << "\n[✔] Saat gerçek zamana geri ayarlandı: "
                    << startRealTime.wHour << ":" << startRealTime.wMinute << ":" << startRealTime.wSecond << "\n";
            }
            else {
                std::cerr << "\n[X] Saati geri yüklerken hata oluştu! Yönetici olarak çalıştırmayı deneyin. Hata kodu: " << GetLastError() << "\n";
            }
            break;
        }
    }
    return 0;
}