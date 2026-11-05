/*
 * test_idtables.c - host check of the PCI / USB id tables and the chip
 * identification against the Linux drivers they follow (pinned Linux commit
 * 65ba8f34ec886ec72b2c07ddc8cf3d815c32869d):
 *   drivers/net/ethernet/realtek/r8169_main.c  rtl8169_pci_tbl[], rtl_chip_infos[]
 *   drivers/net/ethernet/realtek/8139too.c     rtl8139_pci_tbl[], rtl_chip_info[]
 *   drivers/net/ethernet/realtek/8139cp.c      cp_pci_tbl[]
 *   drivers/net/ethernet/8390/ne2k-pci.c       ne2k_pci_tbl[] (Realtek row)
 *   drivers/net/usb/r8152.c                    rtl8152_table[]
 *   drivers/net/usb/rtl8150.c                  rtl8150_table[]
 * Build: tests/run.sh
 */
#include <stdio.h>
#include "../src/rtlchip.h"
#include "../USBNT/src/usbchip.h"
#include "../src/fwimages.h"

static int failures;
#define CHECK(c, ...) do { if (c) { printf("ok  : "); } else { printf("FAIL: "); failures++; } \
                           printf(__VA_ARGS__); printf("\n"); } while (0)
#define ANY 0xFFFF

struct pci { unsigned short v, d, sv, sd; int backend; };
static const struct pci linux_pci[] = {
    /* r8169 */
    {0x10EC,0x2502,ANY,ANY,RTL_BACKEND_CPLUS},{0x10EC,0x2600,ANY,ANY,RTL_BACKEND_CPLUS},
    {0x10EC,0x8129,ANY,ANY,-1},{0x10EC,0x8136,ANY,ANY,RTL_BACKEND_CPLUS},
    {0x10EC,0x8161,ANY,ANY,RTL_BACKEND_CPLUS},{0x10EC,0x8162,ANY,ANY,RTL_BACKEND_CPLUS},
    {0x10EC,0x8167,ANY,ANY,RTL_BACKEND_CPLUS},{0x10EC,0x8168,ANY,ANY,RTL_BACKEND_CPLUS},
    {0x10FF,0x8168,ANY,ANY,RTL_BACKEND_CPLUS},{0x10EC,0x8169,ANY,ANY,RTL_BACKEND_CPLUS},
    {0x1186,0x4300,ANY,ANY,RTL_BACKEND_CPLUS},{0x1186,0x4302,ANY,ANY,RTL_BACKEND_CPLUS},
    {0x1259,0xC107,ANY,ANY,RTL_BACKEND_CPLUS},{0x16EC,0x0116,ANY,ANY,RTL_BACKEND_CPLUS},
    {0x1737,0x1032,ANY,0x0024,RTL_BACKEND_CPLUS},{0x0001,0x8168,ANY,0x2410,RTL_BACKEND_CPLUS},
    {0x10EC,0x8125,ANY,ANY,RTL_BACKEND_CPLUS},{0x10EC,0x8126,ANY,ANY,RTL_BACKEND_CPLUS},
    {0x10EC,0x8127,ANY,ANY,RTL_BACKEND_CPLUS},{0x10EC,0x3000,ANY,ANY,RTL_BACKEND_CPLUS},
    {0x10EC,0x5000,ANY,ANY,RTL_BACKEND_CPLUS},{0x10EC,0x0E10,ANY,ANY,RTL_BACKEND_CPLUS},
    /* 8139too */
    {0x10EC,0x8139,ANY,ANY,RTL_BACKEND_8139},{0x10EC,0x8138,ANY,ANY,RTL_BACKEND_8139},
    {0x1113,0x1211,ANY,ANY,RTL_BACKEND_8139},{0x1500,0x1360,ANY,ANY,RTL_BACKEND_8139},
    {0x4033,0x1360,ANY,ANY,RTL_BACKEND_8139},{0x1186,0x1300,ANY,ANY,RTL_BACKEND_8139},
    {0x1186,0x1340,ANY,ANY,RTL_BACKEND_8139},{0x13D1,0xAB06,ANY,ANY,RTL_BACKEND_8139},
    {0x1259,0xA117,ANY,ANY,RTL_BACKEND_8139},{0x1259,0xA11E,ANY,ANY,RTL_BACKEND_8139},
    {0x14EA,0xAB06,ANY,ANY,RTL_BACKEND_8139},{0x14EA,0xAB07,ANY,ANY,RTL_BACKEND_8139},
    {0x11DB,0x1234,ANY,ANY,RTL_BACKEND_8139},{0x1432,0x9130,ANY,ANY,RTL_BACKEND_8139},
    {0x02AC,0x1012,ANY,ANY,RTL_BACKEND_8139},{0x018A,0x0106,ANY,ANY,RTL_BACKEND_8139},
    {0x126C,0x1211,ANY,ANY,RTL_BACKEND_8139},{0x1743,0x8139,ANY,ANY,RTL_BACKEND_8139},
    {0x021B,0x8139,ANY,ANY,RTL_BACKEND_8139},{0x16EC,0xAB06,ANY,ANY,RTL_BACKEND_8139},
    {ANY,0x8139,0x10EC,0x8139,RTL_BACKEND_8139},{ANY,0x8139,0x1186,0x1300,RTL_BACKEND_8139},
    {ANY,0x8139,0x13D1,0xAB06,RTL_BACKEND_8139},
    /* 8139cp */
    {0x0357,0x000A,ANY,ANY,RTL_BACKEND_8139},
    /* ne2k-pci */
    {0x10EC,0x8029,ANY,ANY,RTL_BACKEND_NE2000},
};

struct usb { unsigned short v, p; int proto; };
static const struct usb linux_usb[] = {
    /* r8152 */
    {0x0BDA,0x8050,RTLU_PROTO_8152},{0x0BDA,0x8053,RTLU_PROTO_8152},{0x0BDA,0x8152,RTLU_PROTO_8152},
    {0x0BDA,0x8153,RTLU_PROTO_8152},{0x0BDA,0x8155,RTLU_PROTO_8152},{0x0BDA,0x8156,RTLU_PROTO_8152},
    {0x0BDA,0x8157,RTLU_PROTO_8152},{0x0BDA,0x815A,RTLU_PROTO_8152},
    {0x045E,0x07AB,RTLU_PROTO_8152},{0x045E,0x07C6,RTLU_PROTO_8152},{0x045E,0x0927,RTLU_PROTO_8152},
    {0x045E,0x0C5E,RTLU_PROTO_8152},{0x04E8,0xA101,RTLU_PROTO_8152},
    {0x17EF,0x304F,RTLU_PROTO_8152},{0x17EF,0x3054,RTLU_PROTO_8152},{0x17EF,0x3062,RTLU_PROTO_8152},
    {0x17EF,0x3069,RTLU_PROTO_8152},{0x17EF,0x3082,RTLU_PROTO_8152},{0x17EF,0x3098,RTLU_PROTO_8152},
    {0x17EF,0x7205,RTLU_PROTO_8152},{0x17EF,0x720C,RTLU_PROTO_8152},{0x17EF,0x7214,RTLU_PROTO_8152},
    {0x17EF,0x721E,RTLU_PROTO_8152},{0x17EF,0xA359,RTLU_PROTO_8152},{0x17EF,0xA387,RTLU_PROTO_8152},
    {0x13B1,0x0041,RTLU_PROTO_8152},{0x0955,0x09FF,RTLU_PROTO_8152},{0x2357,0x0601,RTLU_PROTO_8152},
    {0x2357,0x0602,RTLU_PROTO_8152},{0x2001,0xB301,RTLU_PROTO_8152},{0x413C,0xB097,RTLU_PROTO_8152},
    {0x0B05,0x1976,RTLU_PROTO_8152},{0x20F4,0xE02B,RTLU_PROTO_8152},{0x20F4,0xE02C,RTLU_PROTO_8152},
    /* rtl8150 */
    {0x0BDA,0x8150,RTLU_PROTO_8150},{0x0411,0x0012,RTLU_PROTO_8150},{0x3980,0x0003,RTLU_PROTO_8150},
    {0x07B8,0x401A,RTLU_PROTO_8150},{0x1557,0x8150,RTLU_PROTO_8150},{0x0586,0x401A,RTLU_PROTO_8150},
};

static const RTL_USB_ID *usb_match(unsigned short v, unsigned short p)
{
    const RTL_USB_ID *id;
    for (id = RtlUsbIds; id->Name; id++)
        if (id->VendorId == v && id->DeviceId == p) return id;
    return 0;
}

/* 8139too HW_REVID values (TxConfig) and 8139cp's RTL8139C+. */
static const unsigned long rev8139[] = {
    0x40000000UL, 0x60000000UL, 0x70000000UL, 0x70800000UL, 0x78000000UL,
    0x7C000000UL, 0x74000000UL, 0x78800000UL, 0x74400000UL, 0x74C00000UL, 0x74800000UL
};

int main(void)
{
    unsigned i, n;

    /* every Linux PCI row binds with the backend of its Linux driver */
    for (i = n = 0; i < sizeof(linux_pci) / sizeof(linux_pci[0]); i++) {
        const struct pci *p = &linux_pci[i];
        unsigned short v = p->v == ANY ? 0x0001 : p->v;
        unsigned short sv = p->sv == ANY ? 0x1234 : p->sv, sd = p->sd == ANY ? 0x5678 : p->sd;
        const RTL_PCI_ID *r = RtlPciMatch(v, p->d, sv, sd);
        if (!r || (p->backend >= 0 && r->Backend != p->backend)) {
            printf("FAIL: PCI %04X:%04X sub %04X:%04X\n", v, p->d, sv, sd); failures++;
        } else n++;
    }
    CHECK(n == sizeof(linux_pci) / sizeof(linux_pci[0]), "%u Linux PCI ids bind with their backend", n);
    CHECK(RtlPciMatch(0x1737, 0x1032, 0x1737, 0x0025) == 0, "Linksys 1737:1032 needs subsystem 0024 (r8169)");
    CHECK(RtlPciMatch(0x0001, 0x8168, 0x1234, 0x2411) == 0, "0001:8168 needs subsystem 2410 (r8169)");
    CHECK(RtlPciMatch(0x0001, 0x8139, 0x1234, 0x5678) == 0, "x:8139 with an unknown subsystem is not taken");
    CHECK(RtlPciMatch(0x8086, 0x100E, 0x8086, 0x001E) == 0, "foreign device (Intel e1000) not taken");
    CHECK(RtlPciMatch(0xFFFF, 0x8139, 0x10EC, 0x8139) == 0, "absent function (FFFF) not taken");
    {
        const RTL_PCI_ID *r = RtlPciMatch(0x10EC, 0x8129, 0, 0);
        CHECK(r && (r->Flags & RTL_PCIF_XIDPROBE), "10EC:8129 is decided by the XID");
    }

    /* XID decisions for 10EC:8129 */
    {
        static const unsigned x8169[] = { 0x008, 0x040, 0x100, 0x180, 0x980 };
        for (i = n = 0; i < 5; i++) n += RtlXidIsPci8169(x8169[i]) != 0;
        CHECK(n == 5, "the five RTL8169/8110 XIDs select the C+ backend");
        for (i = n = 0; i < sizeof(rev8139) / sizeof(rev8139[0]); i++)
            n += RtlXidIsPci8169((unsigned)((rev8139[i] >> 20) & 0xfff)) == 0;
        CHECK(n == sizeof(rev8139) / sizeof(rev8139[0]), "no RTL8129/8139 HW_REVID looks like an RTL8169 XID");
        /* Only the five RTL8169 XIDs may decide: RTL8139A rev G (0x708) is
           also the RTL8125CP XID of the full C+ catalogue. */
        for (i = n = 0; i < sizeof(rev8139) / sizeof(rev8139[0]); i++)
            if (RtlFwChip((RTL_FW_U16)((rev8139[i] >> 20) & 0xfff), 1)) n = (unsigned)(rev8139[i] >> 20);
        CHECK(n == 0x708, "the full C+ catalogue overlaps RTL8139A rev G (so the 8129 probe uses the 8169 set only)");
    }

    /* 8139 names follow 8139too */
    for (i = n = 0; i < sizeof(rev8139) / sizeof(rev8139[0]); i++) {
        unsigned j;
        for (j = 0; j < RTL_8139_VER_COUNT; j++) if (Rtl8139Versions[j].HwVerId == rev8139[i]) { n++; break; }
    }
    CHECK(n == sizeof(rev8139) / sizeof(rev8139[0]), "every 8139too revision has a name");

    /* extended XID (RTL9151AS) */
    {
        const RTL_FW_CHIP *c;
        CHECK(RtlFwXidIsExtended(0x7c8) && RtlFwXidIsExtended(0xfc8), "XID 0x7c8 is the extended marker");
        CHECK(!RtlFwXidIsExtended(0x7c9) && !RtlFwXidIsExtended(0x688), "other XIDs are not");
        CHECK(RtlFwChip(0x7c8, 1) == 0, "the marker itself is no chip");
        CHECK(RtlFwExtendedXid(0) == RTL_FW_XID_EXTENDED, "TX_CONFIG_V2 0 -> RTL9151AS");
        CHECK(RtlFwExtendedXid(0x80000000UL) == RTL_FW_XID_EXTENDED, "TX_CONFIG_V2 bit 31 is masked (0x7fffffff)");
        CHECK(RtlFwExtendedXid(1) == RTL_FW_XID_UNKNOWN, "other TX_CONFIG_V2 values are unknown");
        c = RtlFwChip(RTL_FW_XID_EXTENDED, 1);
        CHECK(c && c->MacVersion == 64 && c->Image && c->Image->Name &&
              !strcmp(c->Image->Name, "rtl_nic/rtl9151a-1.fw"), "RTL9151AS = VER_64 with rtl9151a-1.fw");
        CHECK(RtlFwChip(RTL_FW_XID_UNKNOWN, 1) == 0, "unknown XID has no chip");
        for (i = n = 0; i < RTL_XID_COUNT; i++)
            if ((RTL_XID_EXTENDED & RtlXidTable[i].Mask) == RtlXidTable[i].Id) { n = i + 1; break; }
        CHECK(n && !strcmp(RtlXidTable[n - 1].Name, "Realtek RTL9151AS 2.5GbE Controller") &&
              (RtlXidTable[n - 1].Flags & RTL_F_8125REGS), "RTL9151AS name row uses the 8125 register layout");
        CHECK(RTL_XID_EXTENDED == RTL_FW_XID_EXTENDED, "name table and firmware catalogue agree on the code");
    }

    /* USB */
    for (i = n = 0; i < sizeof(linux_usb) / sizeof(linux_usb[0]); i++) {
        const RTL_USB_ID *r = usb_match(linux_usb[i].v, linux_usb[i].p);
        if (!r || r->Protocol != linux_usb[i].proto) {
            printf("FAIL: USB %04X:%04X\n", linux_usb[i].v, linux_usb[i].p); failures++;
        } else n++;
    }
    CHECK(n == sizeof(linux_usb) / sizeof(linux_usb[0]), "%u Linux USB ids bind with their protocol", n);
    CHECK(usb_match(0x17EF, 0x8153) == 0, "vendor is part of the USB match");
    CHECK(usb_match(0x0B95, 0x1790) == 0, "foreign USB NIC (ASIX AX88179) not taken");

    printf(failures ? "ID TABLE TESTS FAILED (%d failures)\n" : "ID TABLE TESTS PASSED (%d failures)\n", failures);
    return failures != 0;
}
