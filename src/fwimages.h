/* Immutable firmware catalogue. Selection alone does not initialize a PHY. */
#ifndef RLTK_FWIMAGES_H
#define RLTK_FWIMAGES_H
#include "fwvm.h"
typedef struct {
    const char *Name;
    const RTL_FW_U8 *Bytes;
    RTL_FW_U32 Size;
} RTL_FW_IMAGE;
typedef struct {
    RTL_FW_U16 Mask,Xid,MacVersion;
    const RTL_FW_IMAGE *Image;
} RTL_FW_CHIP;
/* xid: TxConfig bits 31..20, or RTL_FW_XID_EXTENDED+n for a chip that
   reports the extended marker and is identified by TX_CONFIG_V2. */
#define RTL_FW_XID_EXTENDED 0x1000
#define RTL_FW_XID_UNKNOWN  0xffff
#define RTL_FW_TX_CONFIG_V2 0x60b0
const RTL_FW_CHIP *RtlFwChip(RTL_FW_U16 xid,int gmii);
int RtlFwXidIsExtended(RTL_FW_U16 xid);
RTL_FW_U16 RtlFwExtendedXid(RTL_FW_U32 xid2);
#endif
