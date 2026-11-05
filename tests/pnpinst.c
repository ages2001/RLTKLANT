/*
 * pnpinst.c - test helper for the Windows 2000 / XP guests: installs
 * netrtlk.inf on every present device with one of the given hardware ids
 * (UpdateDriverForPlugAndPlayDevices), then prints one line per id.
 *   pnpinst <inf> <hwid> [<hwid> ...]
 */
#include <windows.h>
#include <newdev.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    char inf[MAX_PATH];
    int i, failed = 0;
    if (argc < 3 || !GetFullPathNameA(argv[1], sizeof(inf), inf, NULL)) {
        printf("usage: pnpinst <inf> <hwid>...\n");
        return 2;
    }
    for (i = 2; i < argc; i++) {
        BOOL reboot = FALSE;
        BOOL ok = UpdateDriverForPlugAndPlayDevicesA(NULL, argv[i], inf, INSTALLFLAG_FORCE, &reboot);
        DWORD err = ok ? 0 : GetLastError();
        printf("PNPINST %s %s err=0x%08lx reboot=%d\n", argv[i], ok ? "OK" : "FAIL", err, reboot);
        fflush(stdout);
        if (!ok) failed++;
    }
    return failed;
}
