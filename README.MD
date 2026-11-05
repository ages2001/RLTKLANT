<div align="center">

# RLTKLANT

**One Realtek LAN driver for Windows NT 3.1 through Windows XP x64 / Server 2003 x64**

From the 10 Mbit RTL8029 to the 10 Gbit RTL8127 on PCI / PCIe, and the
RTL8150 / RTL815x family on USB, built from one source tree.

![Version](https://img.shields.io/badge/version-1.0-blue)
![Windows NT](https://img.shields.io/badge/Windows%20NT-3.1%20%7C%203.5%20%7C%203.51%20%7C%204.0-informational)
![Windows](https://img.shields.io/badge/Windows-2000%20%7C%20XP%20%7C%20XP%20x64%20%7C%202003%20x64-informational)
![Bus](https://img.shields.io/badge/bus-PCI%20%7C%20PCIe%20%7C%20CardBus%20%7C%20USB-success)
![Speed](https://img.shields.io/badge/speed-10M%20to%2010G-success)

</div>

> [!NOTE]
> RLTKLANT 1.0 is a validation candidate. Every result in this README comes
> from QEMU: the stock QEMU models plus the device models in [`qemu/`](qemu/).
> No physical hardware was tested. Support for a chip means the driver
> identifies it and has the matching register model and sequences. It does
> not mean that board was tested in silicon. See [Testing](#testing).

## Contents

- [Overview](#overview)
- [Supported operating systems](#supported-operating-systems)
- [Supported devices](#supported-devices)
  - [PCI / PCIe / CardBus](#pci--pcie--cardbus)
  - [Boards under other vendors' ids](#boards-under-other-vendors-ids)
  - [USB](#usb)
- [Installing](#installing)
- [DHCPNT31: NT 3.1 DHCP client](#dhcpnt31-nt-31-dhcp-client)
- [Building](#building)
- [Testing](#testing)
- [Technical notes](#technical-notes)
- [Repository layout](#repository-layout)
- [Changelog](#changelog)
- [Credits](#credits)

## Overview

```text
Realtek LAN family ──► RLTKLANT.SYS
  10M   RTL8029(AS)                          NE2000 register model
  100M  RTL8129 / 8139 / 810x                RTL8139 register model
  1G    RTL8169 / 8110 / 8168 / 8111 / 8411  C+ descriptor model
  2.5G  RTL8125 / RTL9151                    C+ descriptor model, 8125 registers
  5G    RTL8126                              C+ descriptor model, 8125 registers
  10G   RTL8127                              C+ descriptor model, 8125 registers
  USB   RTL8150, RTL8152 ... RTL8159         r8152 / RTL8150 vendor protocol
```

- **The chip is identified at run time.** The PCI id selects the register
  model. The chip's version register (XID or HwVerId, and on the RTL8150 /
  RTL815x the PLA version) then gives the exact part, its name and its
  **hard media ceiling**. A Fast Ethernet part on a Gigabit id never reports
  or forces a Gigabit link.
- **Linux is the reference.** The PCI and USB id tables are the Realtek
  rows of the Linux `r8169`, `8139too`, `8139cp`, `ne2k-pci`, `r8152` and
  `rtl8150` drivers. They match vendor, device and subsystem the way Linux
  does. The MAC / PHY start-up sequences and firmware come from the pinned
  Linux `r8169` sources in [`third_party/r8169`](third_party/r8169).
- **One binary per NDIS generation.** NT 3.1 gets an NDIS 3.0 MAC driver.
  NT 3.5x / 4.0 and 2000 / XP get an NDIS 3.0 or 5.1 miniport built from the
  same code.

## Supported operating systems

| Package | Operating system | NDIS model |
|---|---|---|
| [`bin/NT31`](bin/NT31) | Windows NT 3.1 | NDIS 3.0 **MAC** driver |
| [`bin/NT35x-4`](bin/NT35x-4) | Windows NT 3.5 / 3.51 / 4.0 | NDIS 3.0 miniport |
| [`bin/2KXP`](bin/2KXP) | Windows 2000 / XP / XP x64 / Server 2003 x64 | NDIS 5.1 miniport, one INF for x86 (`i386`) and x64 (`amd64`) |

<details>
<summary><b>Why NT 3.1 has its own binary</b></summary>

NT 3.1's `ndis.sys` predates the miniport model. It exports
`NdisRegisterMac` / `NdisRegisterAdapter` but **not**
`NdisMRegisterMiniport`, so a miniport cannot load there. The NT 3.1 build
(`src/drvnt31.c`) is a full NDIS 3.0 MAC driver. NT 3.1 also has no
`NdisReadPciSlotInformation`, so this build scans PCI configuration space
itself. It has the NE2000, RTL8139 and C+ backends and the persistent USB
adapters over USBNT. Everything from NT 3.5 to XP x64 is one miniport code
base, selected by the `RTL_NDIS3` / `RTL_NDIS5` build define.

</details>

## Supported devices

### PCI / PCIe / CardBus

Realtek's own ids are `PCI\VEN_10EC&DEV_xxxx`.

| Device id | Chips | Register model | Ceiling |
|---|---|---|---|
| `8029` | RTL8029(AS) | NE2000 | 10 Mbit |
| `8129` | RTL8129, or an RTL8169 / 8110 (the XID decides) | RTL8139 or C+ | 100 Mbit / 1 Gbit |
| `8139` | RTL8139, 8139A/B/C/C+/D, 8100, 8100B, 8101 | RTL8139 | 100 Mbit |
| `8138` | RTL8139B CardBus | RTL8139 | 100 Mbit |
| `8130`, `8100` | RTL8139D, RTL8100 | RTL8139 | 100 Mbit |
| `8169` | RTL8169 / 8169S / 8169SB / 8169SC, RTL8110S / SB / SC | C+ | 1 Gbit |
| `8167` | RTL8110SC / 8169SC | C+ | 1 Gbit |
| `8168`, `8161`, `8162`, `2502`, `2600` | RTL8168 / 8111 B, C, CP, D, DP, E, E-VL, F, G, GU, H, EP, FP, RTL8168M, RTL8117 | C+ | 1 Gbit |
| `8136`, `8101` | RTL8101E / 8102E / 8103E / 8105E / 8106E / 8106EUS / 8107E, RTL8401, RTL8402 | C+ (Fast Ethernet) | 100 Mbit |
| `8411` | RTL8411 / 8411B | C+ | 1 Gbit |
| `8125`, `3000`, `5000`, `0E10` | RTL8125A / B / BP / CP / D / K, RTL9151A, RTL9151AS | C+ (8125 registers) | 2.5 Gbit |
| `8126` | RTL8126A | C+ (8125 registers) | 5 Gbit |
| `8127` | RTL8127A | C+ (8125 registers) | 10 Gbit |

- **10EC:8129** is listed by both Linux `r8169` and `8139too`. If the
  TxConfig XID names an RTL8169 / 8110 part, the device runs on the C+
  backend. Otherwise it runs as an RTL8129 on the RTL8139 backend.
- **Extended XID.** XID `0x7c8` means "read TX_CONFIG_V2 (0x60b0)", as in
  Linux. The RTL9151AS found that way runs the RTL8125D (VER_64) sequences
  with `rtl9151a-1.fw`.
- **Exact name.** The driver reads TxConfig bits 31..20 and reports the
  marketing name, for example *Realtek RTL8168h/8111h PCIe Gigabit
  Ethernet*. The negotiated speed is clamped to that part's ceiling
  (`RtlCapSpeed`).

### Boards under other vendors' ids

Many boards carry their maker's PCI vendor id on a Realtek chip. All the
Realtek-chip rows of the Linux tables are in [`src/rtlchip.h`](src/rtlchip.h).
NT 3.1 and NT 3.5x / 4.0 use that table, and
[`bin/2KXP/netrtlk.inf`](bin/2KXP/netrtlk.inf) is generated from it by
[`tools/mkinfids.py`](tools/mkinfids.py).

<details>
<summary><b>RTL8139-based boards (8139too, 8139cp)</b></summary>

| Vendor:Device | Board |
|---|---|
| `1113:1211` | Accton EN-1207D |
| `1500:1360` | Delta Electronics |
| `4033:1360` | Addtron |
| `1186:1300` | D-Link DFE-538TX |
| `1186:1340` | D-Link DFE-690TXD (CardBus) |
| `13D1:AB06` | AboCom FE2000VX |
| `1259:A117`, `1259:A11E` | Allied Telesyn |
| `14EA:AB06` | Planex FNW-3603-TX (CardBus) |
| `14EA:AB07` | Planex FNW-3800-TX (CardBus) |
| `11DB:1234` | Sega |
| `1432:9130` | Edimax |
| `02AC:1012` | SpeedStream 1012 (CardBus) |
| `018A:0106` | LevelOne FPC-0106TX (CardBus) |
| `126C:1211` | Nortel |
| `1743:8139` | Peppercon ROL/F-100 |
| `021B:8139` | Compaq HNE-300 (CardBus) |
| `16EC:AB06` | U.S. Robotics |
| `0357:000A` | TTTech MC322 (RTL8139C+) |
| *any*`:8139` with subsystem `10EC:8139`, `1186:1300` or `13D1:AB06` | RTL8139 cards with a broken vendor id (for example `0001`) |

</details>

<details>
<summary><b>RTL8169 / RTL8168-based boards (r8169)</b></summary>

| Vendor:Device | Board |
|---|---|
| `1186:4300` | D-Link DGE-528T |
| `1186:4302` | D-Link DGE-530T |
| `1259:C107` | Allied Telesis |
| `16EC:0116` | U.S. Robotics USR997902 |
| `10FF:8168` | nCUBE |
| `1737:1032`, subsystem `xxxx:0024` | Linksys EG1032 v3 |
| `0001:8168`, subsystem `xxxx:2410` | RTL8168 with a broken vendor id |

</details>

> [!TIP]
> On Windows 2000 / XP, an RTL8139 card with a broken vendor id other than
> `0001` has no hardware id that an INF can list. Pick RLTKLANT by hand in
> Device Manager (*Update Driver → Don't search → Have Disk*). The driver
> accepts the card by its subsystem id. NT 3.x / 4.0 find it on their own.

### USB

USB adapters use the RTL8150 protocol (USB 1.1) or the r8152 vendor
protocol, which covers RTL8152 through RTL8159. The silicon revision is read
from the chip (PLA `0xE610`), so the USB id only admits the device. The name
and the speed ceiling come from the detected silicon. The ids are those of
the Linux `r8152` and `rtl8150` drivers, plus the Realtek ids that Linux
leaves to `cdc_ether` / `cdc_ncm`.

| Vendor:Product | Chip family | Max speed |
|---|---|---|
| `0BDA:8150` | RTL8150 (USB 1.1, own protocol) | 100 Mbit |
| `0BDA:8151` | RTL8151 | 100 Mbit |
| `0BDA:8152`, `0BDA:8050` | RTL8152 / 8152B | 100 Mbit |
| `0BDA:8153`, `0BDA:8053` | RTL8153, 8153B, 8153C, 8153D, 8153E | 1 Gbit |
| `0BDA:8154` | RTL8154 / 8154B (USB 2.0) | 1 Gbit |
| `0BDA:8155`, `0BDA:8156` | RTL8156, 8156B, 8156BG | 2.5 Gbit |
| `0BDA:8157` | RTL8157 | 5 Gbit |
| `0BDA:8159`, `0BDA:815A` | RTL8159 | 10 Gbit |

<details>
<summary><b>Recognised r8152-family silicon revisions</b></summary>

| PLA version | Chip | Ceiling |
|---|---|---|
| `4C00`, `4C10`, `4800` | RTL8152 | 100 Mbit |
| `5C00`, `5C10`, `5C20`, `5C30` | RTL8153 | 1 Gbit |
| `6000`, `6010` | RTL8153B | 1 Gbit |
| `6400` | RTL8153C | 1 Gbit |
| `7420` | RTL8153D | 1 Gbit |
| `7020`, `7030` | RTL8156 | 2.5 Gbit |
| `7400`, `7410` | RTL8156B | 2.5 Gbit |
| `1030` | RTL8157 | 5 Gbit |
| `2020` | RTL8159 (two package paths) | 10 Gbit |

</details>

<details>
<summary><b>Adapters and docks under other vendors' ids (r8152, rtl8150)</b></summary>

| Vendor | Product ids |
|---|---|
| Microsoft (`045E`) | `07AB`, `07C6`, `0927`, `0C5E` |
| Samsung (`04E8`) | `A101` |
| Lenovo (`17EF`) | `304F`, `3054`, `3062`, `3069`, `3082`, `3098`, `7205`, `720C`, `7214`, `721E`, `A359`, `A387` |
| Linksys (`13B1`) | `0041` |
| NVIDIA (`0955`) | `09FF` |
| TP-Link (`2357`) | `0601`, `0602` |
| D-Link (`2001`) | `B301` |
| Dell (`413C`) | `B097` |
| ASUS (`0B05`) | `1976` |
| TRENDnet (`20F4`) | `E02B`, `E02C` |
| RTL8150 adapters | Buffalo LUA-KTX `0411:0012`, Micronet SP128AR `3980:0003`, Longshine LCS-8138TX `07B8:401A`, OQO `1557:8150`, ZyXEL Prestige `0586:401A` |

</details>

**Which USB stack carries the adapter**

| Windows | Host controller | USB stack |
|---|---|---|
| NT 3.1 / 3.5 / 3.51 / 4.0 | any (UHCI, OHCI, EHCI, xHCI) | [USBNT](https://github.com/ages2001/USBNT): persistent adapter matched by its MAC |
| 2000 / XP / XP x64 | UHCI, OHCI, EHCI | Windows' own USB stack, native URBs |
| 2000 / XP / XP x64 | xHCI, USB4 | USBNT (Windows has no xHCI driver) |

Some adapters and docks present a CDC ECM / NCM configuration first and the
vendor mode second. RLTKLANT on 2000 / XP and USBNT on every NT version use
the vendor configuration, as Linux `r8152` does.

## Installing

### Windows 2000 / XP / XP x64

Point *Found New Hardware* or *Device Manager → Update Driver* at
[`bin/2KXP/netrtlk.inf`](bin/2KXP/netrtlk.inf). One INF covers x86 and x64
and both PCI and USB; SetupAPI picks `i386/` or `amd64/`. The driver binds
to the installed TCP/IP and offers a *Speed & Duplex* property limited to
the detected chip.

> [!IMPORTANT]
> `rltklant.cat` is a self-signed test catalog. Windows asks before it
> installs the driver. For a signed release, rebuild the catalog with
> `inf2cat` and `signtool` ([`bin/2KXP/makecat.cmd`](bin/2KXP/makecat.cmd)).

### Windows NT 3.5x / 4.0

*Control Panel → Network → Adapters → Add → Have Disk*, then select
[`bin/NT35x-4`](bin/NT35x-4).

### Windows NT 3.1

*Control Panel → Network → Add Adapter → Other*, then select
[`bin/NT31`](bin/NT31). NT 3.1's TCP/IP has no DHCP client of its own; use
[DHCPNT31](#dhcpnt31-nt-31-dhcp-client).

### USB adapters on NT 3.1 to 4.0

1. Install [USBNT](https://github.com/ages2001/USBNT) with boot start.
2. Add an adapter with the matching Network Setup package.
3. Bind it to the USB adapter by its permanent MAC (twelve hex digits),
   then restart:

   ```text
   RLTUSB list
   RLTUSB usb RLTKLANT1 525400abcdef
   ```

Use one configured instance per USB adapter. Its TCP/IP binding survives
unplugging and plugging the adapter in again. Details are in
[docs/USBNT-NDIS-DEVELOPMENT.md](docs/USBNT-NDIS-DEVELOPMENT.md).

## DHCPNT31: NT 3.1 DHCP client

[`DHCPNT31.EXE`](DHCPNT31) gets a lease over Windows Sockets 1.1 and saves it
as the static TCP/IP settings for the next restart. With `/stay` it keeps
renewing every half lease while the user is logged on.

```text
C:\> DHCPNT31 /auto /apply /stay
DHCPDISCOVER sent (try 1)
DHCPOFFER 192.168.1.50
DHCPREQUEST sent (try 1)

=== LEASE ACQUIRED ===
IP Address : 192.168.1.50
Subnet Mask: 255.255.255.0
Gateway    : 192.168.1.1
DNS Server : 8.8.8.8
Lease Time : 86400 seconds
DHCP Server: 192.168.1.1
```

All commands and limits are in [DHCPNT31/README.MD](DHCPNT31/README.MD).

## Building

RLTKLANT builds with a free `mingw-w64` + `binutils` toolchain and needs no
DDK. [`src/ndiskit.h`](src/ndiskit.h) and [`src/ndisnt31.h`](src/ndisnt31.h)
declare the NDIS / HAL functions the driver uses. The import libraries are
generated from each target's own `ndis.sys` / `hal.dll` exports.

```sh
build/build.sh ndis3      # NT 3.5x / 4.0          -> build/ndis3/rltklant.sys
build/build.sh ndis5      # 2000 / XP x86          -> build/ndis5/rltklant.sys
build/build.sh ndis5x64   # XP x64 / 2003 x64      -> build/ndis5x64/rltklant.sys
build/build-nt31.sh       # NT 3.1 MAC driver      -> build/ndisnt31/rltklant.sys
bash DHCPNT31/build.sh    # DHCPNT31.EXE
```

The build checks that the generated sources are current:

- the firmware catalogue (`build/make-fwimages.py`)
- the USB PHY tables (`build/make-usb-phy.py`)
- the INF models (`tools/mkinfids.py`)

Native NT 3.x / 4.0 Setup media are assembled with
[`tools/package_legacy.py`](docs/LEGACY-PACKAGING.md), which writes the
exact copy-list sizes.

## Testing

```sh
bash tests/run.sh            # id tables, chip identification, INF in sync
bash USBNT/test/run.sh       # USB protocol, hot plug and QEMU-model interop
python3 tests/w5rltk.py pc   # XP x64 guest: install, DHCP and ping per adapter
```

These are the latest QEMU results. The adapters are QEMU's emulated Realtek
chips; [`qemu/`](qemu/) can give a chip another board's PCI or USB identity
for these tests.

| Guest | Adapters | Result |
|---|---|---|
| XP x64 (2026-11-03) | D-Link DGE-528T `1186:4300`, D-Link DFE-538TX `1186:1300`, `10EC:8129` (8139 XID and 8169 XID), RTL8139 with vendor `0001`, Accton `1113:1211`, RTL9151AS (extended XID), Lenovo `17EF:7205` RTL8153 (CDC first) on in-box EHCI and on USBNT xHCI | Plug and Play install, DHCP and ping on every adapter: **PASS** ([log](test-results/xp64_linux_ids_20261103.txt)) |
| NT 3.1 | RTL8029, RTL8139, RTL8169, RTL8127 (10G), USB RTL8153 over USBNT | ping 4/4 on every backend |
| NT 3.5 / 3.51 / 4.0 | RTL8139, two adapters with DHCP | DHCP and ping |
| Windows 2000 | RTL8029, RTL8139, RTL8169, RTL8125 | ping 5/5, 0 % loss |
| XP x64 | RTL8139; RTL8168 | ping 5/5, 0 % loss; installs and starts at 1000 Mbit |

![NT 3.1 pinging over an RTL8127 at 10 Gbit](test-results/nt31_rtl8127_10G_ping.png)

> [!WARNING]
> The new PCI and USB ids have not been run in an NT 3.1, NT 3.5x / 4.0 or
> Windows 2000 guest yet; those drivers use the same id table. Physical
> boards, SFP / SerDes parts and PCIe power-loss / resume have not been
> tested at all.

<details>
<summary><b>QEMU models</b></summary>

QEMU itself emulates only the RTL8139 and the RTL8029 (`ne2k_pci`).
[`qemu/hw-net/rtl8169.c`](qemu/hw-net/rtl8169.c) implements the C+
descriptor engine and the 8125+ register layout:

```text
-device rtl8169                         # RTL8169 (1 GbE, PCI)
-device rtl8125                         # also rtl8168, rtl8126, rtl8127 (PCIe)
-device rtl8169,variant=2,chip-profile=9151as   # RTL9151AS (extended XID)
-device rtl8169,x-vendor-id=0x1186,x-device-id=0x4300   # another board's id
```

[`qemu/hw-usb/dev-rtl8153.c`](qemu/hw-usb/dev-rtl8153.c) provides
`usb-rtl8153`, `usb-rtl8156`, `usb-rtl8157`, `usb-rtl8159`, `usb-rtl8150` and
`usb-rtl8153-cdcfirst`.
[`qemu/patches/rtl8139-test-identities.patch`](qemu/patches/rtl8139-test-identities.patch)
gives QEMU's `rtl8139` the same `x-vendor-id` / `x-device-id` /
`x-subsys-vendor-id` / `x-subsys-id` properties. See
[`qemu/README.txt`](qemu/README.txt).

</details>

## Technical notes

<details>
<summary><b>How NT 3.1 was made to work</b></summary>

The NT 3.1 build was brought up under QEMU (Pentium, 64 MB, IDE) on NT 3.1
build 511 with a compiled-in `0xE9` debug-console trace. Every NDIS structure
was reverse-engineered from the in-box `NE2000.SYS` / `LANCE.SYS` MAC
drivers.

- **Setup and loading.**
  - NT 3.1 network setup uses the procedural *MS Setup-script* INF. The
    `key=value` INF of NT 3.5x / 4.0 crashes `SETUP.EXE`.
  - The build uses `-march=pentium`, because GCC's `CMOV` bugchecks with
    `0x7F`.
  - The PE checksum is recomputed, because NT 3.1 checks it.
  - `KeRaiseIrql` / `KeLowerIrql` are imported from `HAL.DLL`.
- **Flat PE image.** The image is laid out like the in-box NE2000:
  section alignment equals file alignment equals `0x200`, file offset equals
  RVA, and there is no `.bss` gap.
- **`NDIS30_MAC_CHARACTERISTICS`.** It has a `UINT Reserved` at `0x04`, ten
  handlers and no `QueryProtocolStatistics`, `0x38` bytes in all. Without
  that field every handler was shifted by four bytes.
- **DISPATCH-level receive.** NT 3.1 maps a driver image in the address
  space of the process that loaded it, so a DPC or ISR in another context
  faults on the driver's own code. RLTKLANT copies its image into non-paged
  pool at `DriverEntry`, re-applies the relocations, and runs every entry
  point NDIS calls later through that copy.
- **Receive delivery.** `OID_GEN_CURRENT_PACKET_FILTER` goes through
  `EthFilterAdjust`. TX walks the whole `NDIS_BUFFER` chain.
- **C+ rings.** The ring base is aligned to 256 bytes, because the
  controller ignores the low 8 bits of `TNPDS` / `RDSAR`.

RX is polled from an NDIS timer DPC; see [docs/10G-NOTES.md](docs/10G-NOTES.md)
for the throughput this allows.

</details>

<details>
<summary><b>NDIS 5.1 registration on XP / 2003 / x64</b></summary>

A miniport registered as NDIS 5.0 on an NDIS 5.1 system gets a legacy path
without its real Plug and Play resources (*Code 10*). RLTKLANT registers as
NDIS 5.1 and falls back to 5.0 only where the OS rejects it (Windows 2000).
`NdisMoveMemory`, `NdisZeroMemory`, `NdisStallExecution`, `NdisCancelTimer`
and `NdisMGetBusData` are DDK macros, not exports, so the driver provides
them itself.

</details>

## Repository layout

```text
RLTKLANT/
├── src/                     one source tree for every target
│   ├── rtlchip.h            PCI id table, XID / HwVerId chip database
│   ├── hwdetect.c           id matching, XID resolution, speed ceiling
│   ├── init.c, driver.c     miniport (NDIS 3.0 / 5.1)
│   ├── drvnt31.c            NT 3.1 MAC driver
│   ├── backend_*.c          NE2000, RTL8139, C+ and USB backends
│   ├── pci_start.c, macseq.c, physeq.c   Linux-derived MAC / PHY start-up
│   ├── native_urb.c         2000 / XP USB (native URBs)
│   └── fwimages.c           generated firmware catalogue
├── USBNT/src/               USB NIC protocol (r8152 family, RTL8150), USB id table
├── third_party/r8169/       pinned Linux r8169 sources
├── firmware/                firmware payloads, licenses, chips.json
├── bin/                     NT31, NT35x-4 and 2KXP packages
├── DHCPNT31/                NT 3.1 DHCP client
├── tools/                   mkinfids.py, package_legacy.py, rltusb.c, rltsetup.c
├── tests/                   host checks and the XP x64 guest test
├── qemu/                    QEMU device models and patches
├── build/                   build scripts
└── docs/                    development notes
```

## Changelog

See [CHANGELOG.md](CHANGELOG.md).

## Credits

RLTKLANT by ages2001.

- **Chip data.** Chip identification, id tables and MAC / PHY sequences
  follow the Linux `r8169`, `8139too`, `8139cp`, `ne2k-pci`, `r8152` and
  `rtl8150` drivers (pinned commit `65ba8f34ec88`), the Realtek vendor
  `r8152` driver and OpenBSD `if_ure`.
- **Firmware.** The payloads keep their upstream license and provenance
  under [`firmware/`](firmware/README.md).
- **Datasheets.** NE2000 and RTL8139 register use also follows the Realtek
  datasheets.
