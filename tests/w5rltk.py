"""RLTKLANT on Windows XP x64 under QEMU: Plug and Play installation of
netrtlk.inf on boards that carry other vendors' ids or the shared 10EC:8129
id, then DHCP and ping through every adapter, each on its own user network.

Uses the guest tooling of USBNT (test/vm5.py, test/w5usb.py) and the patched
QEMU models (qemu/hw-net/rtl8169.c, qemu/patches/rtl8139-test-identities.patch,
qemu/hw-usb/dev-rtl8153.c usb-rtl8153-cdcfirst).

  python3 tests/w5rltk.py [pc|q35|xhci] [--driver rltklant.sys] [--keep]
"""
import os, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, '..')
USBNT_TEST = os.environ.get('USBNT_TEST', '/home/claude/repo/usbnt/test')
sys.path.insert(0, USBNT_TEST)
import vm5
import w5usb

# (QEMU device, hardware id given to pnpinst, user network, what is checked)
NICS_PC = [
    ('rtl8169,x-vendor-id=0x1186,x-device-id=0x4300', 'PCI\\VEN_1186&DEV_4300', 3,
     'D-Link DGE-528T (1186:4300, RTL8169 under an OEM id)'),
    ('rtl8139,x-vendor-id=0x1186,x-device-id=0x1300', 'PCI\\VEN_1186&DEV_1300', 4,
     'D-Link DFE-538TX (1186:1300, RTL8139 under an OEM id)'),
    ('rtl8139,x-device-id=0x8129', 'PCI\\VEN_10EC&DEV_8129', 5,
     '10EC:8129 with an RTL8139 XID (8139 backend)'),
    ('rtl8169,x-device-id=0x8129', 'PCI\\VEN_10EC&DEV_8129', 6,
     '10EC:8129 with an RTL8169 XID (C+ backend)'),
    ('rtl8139,x-vendor-id=0x0001,x-subsys-vendor-id=0x10ec,x-subsys-id=0x8139',
     'PCI\\VEN_0001&DEV_8139&SUBSYS_813910EC', 7, 'RTL8139 with the broken vendor id 0001'),
    ('usb-rtl8153-cdcfirst,bus=ehci.0', 'USB\\VID_17EF&PID_7205', 8,
     'Lenovo 17EF:7205 RTL8153, CDC ECM first configuration (in-box EHCI)'),
]
NICS_Q35 = [
    ('rtl8169,variant=2,chip-profile=9151as', 'PCI\\VEN_10EC&DEV_8125', 3,
     'RTL9151AS (extended XID 0x7c8, TX_CONFIG_V2 = 0)'),
    ('rtl8139,x-vendor-id=0x1113,x-device-id=0x1211', 'PCI\\VEN_1113&DEV_1211', 4,
     'Accton EN-1207D (1113:1211, RTL8139 under an OEM id)'),
]

# USBNT owns the xHCI controller (XP has no xHCI driver); the CDC-first
# adapter reaches RLTKLANT as a USBNT PDO
NICS_XHCI = [
    ('usb-rtl8153-cdcfirst,bus=xhci.0', 'USB\\VID_17EF&PID_7205', 3,
     'Lenovo 17EF:7205 RTL8153, CDC ECM first configuration (USBNT xHCI)'),
]


def run(machine='pc', driver=None, keep=False, guest=None):
    guest = guest or os.environ.get('W5_GUEST', 'xp64')
    arch = 'AMD64' if vm5.GUESTS[guest]['x64'] else 'I386'
    mode, machine = machine, ('pc' if machine in ('xhci', 'ext') else machine)
    nics = {'pc': NICS_PC, 'q35': NICS_Q35, 'ext': NICS_Q35, 'xhci': NICS_XHCI}[mode]
    driver = driver or os.path.join(ROOT, 'build', 'ndis5x64' if arch == 'AMD64' else 'ndis5', 'rltklant.sys')
    pnp = os.environ.get('PNPINST', '/tmp/claude-0/pnpinst%s.exe' % ('64' if arch == 'AMD64' else '32'))
    files = [(os.path.join(ROOT, 'bin', '2KXP', 'netrtlk.inf'), '/RLTK/NETRTLK.INF'),
             (os.path.join(ROOT, 'bin', '2KXP', 'rltklant.cat'), '/RLTK/RLTKLANT.CAT'),
             (driver, '/RLTK/%s/RLTKLANT.SYS' % arch), (pnp, '/RLTK/PNPINST.EXE'),
             (w5usb.FST, '/RLTK/FSTEST.EXE')]
    if mode == 'xhci':
        files += w5usb.driver_files()
    hwids = []
    for _, hw, _, _ in nics:
        if hw not in hwids:
            hwids.append(hw)
    # the clicker answers the Found New Hardware Wizard (which PnP installs
    # wait for) and the unsigned-driver question
    bat = ['start /b C:\\RLTK\\FSTEST.EXE clicker C:\\RLTK\\STOP 1200000']
    if mode == 'xhci':
        bat += ['C:\\USBNT\\USBINST.EXE /install /nogroup /nomon /set DebugPort=0x2f8 /set DebugLevel=2',
                'ping -n 30 127.0.0.1 >nul']
    bat += [
           'C:\\RLTK\\PNPINST.EXE C:\\RLTK\\NETRTLK.INF ' + ' '.join('"%s"' % h for h in hwids),
           'ping -n 45 127.0.0.1 >nul',
           'ipconfig /all']
    for _, _, net, _ in nics:
        bat.append('ping -n 3 -w 2000 10.0.%d.2' % net)
    bat += ['C:\\RLTK\\FSTEST.EXE devlist Realtek', 'C:\\RLTK\\FSTEST.EXE devlist RTL',
            'C:\\RLTK\\FSTEST.EXE write C:\\RLTK\\STOP x', 'C:\\RLTK\\FSTEST.EXE sleep 2000']
    work, g = vm5.prepare(guest, work=os.path.join(vm5.W5_IMG, 'work-rltk-%s-%s.raw' % (guest, mode)),
                          files=files, bat=bat)
    w5usb.no_signing_prompt(work, g)
    extra = []
    if machine == 'q35':
        extra += ['-device', 'pcie-root-port,id=rp1,chassis=1,slot=1']
    if any('xhci.0' in d for d, _, _, _ in nics):
        extra += ['-device', 'qemu-xhci,id=xhci']
    if any('ehci.0' in d for d, _, _, _ in nics):
        extra += ['-device', 'usb-ehci,id=ehci']
    for i, (dev, _, net, _) in enumerate(nics):
        extra += ['-netdev', 'user,id=n%d,net=10.0.%d.0/24' % (i, net),
                  '-device', '%s,netdev=n%d,mac=52:54:00:12:%02x:%02x' % (dev, i, net, i)]
    trace = os.path.join(vm5.OUT, 'rltk-%s-debugcon.txt' % mode)
    extra += ['-debugcon', 'file:' + trace]
    vm = vm5.VM(guest, work, extra=extra, machine=machine, name='rltk-' + mode)
    ok = vm.wait_serial('##AGENT-DONE##', 1500, shots=120)
    time.sleep(2)
    vm.shot('end')
    out = vm.serial()
    usblog = vm.log()
    vm.quit()
    print(out[-9000:])
    try:
        dbg = open(trace, errors='replace').read()
    except OSError:
        dbg = ''
    print('---- driver trace (debugcon) ----')
    print(dbg[-6000:])
    if mode == 'xhci':
        print('---- COM2 (usbnt.sys) ----')
        print(usblog[-4000:])
    checks = {'agent finished': ok}
    if mode == 'xhci':
        checks['USBNT chose the vendor configuration'] = 'vendor configuration 2' in usblog
    for hw in hwids:
        checks['installed ' + hw] = ('PNPINST %s OK' % hw) in out
    for _, _, net, what in nics:
        addr = '10.0.%d.15' % net
        checks['DHCP %s: %s' % (addr, what)] = addr in out
        checks['ping 10.0.%d.2' % net] = ('Reply from 10.0.%d.2' % net) in out
    for k, v in checks.items():
        print('%-90s %s' % (k, 'PASS' if v else 'FAIL'))
    if not keep:
        os.remove(work)
    return all(checks.values())


if __name__ == '__main__':
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    drv = None
    if '--driver' in sys.argv:
        drv = sys.argv[sys.argv.index('--driver') + 1]
        args = [a for a in args if a != drv]
    sys.exit(0 if run(args[0] if args else 'pc', drv, '--keep' in sys.argv) else 1)
