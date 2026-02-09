#include "soes/esc_coe.hpp"
#include "EtherCAT/utypes.hpp"
#include <stddef.h>

#ifndef HW_REV
#define HW_REV "1.0"
#endif

#ifndef SW_REV
#define SW_REV "1.0"
#endif

static const char board_name[] = "ethercat_test_board";

static const char acName1000[] = "Device Type";
static const char acName1008[] = "Device Name";
static const char acName1009[] = "Hardware Version";
static const char acName100A[] = "Software Version";
static const char acName1018[] = "Identity Object";
static const char acName1018_00[] = "Max SubIndex";
static const char acName1018_01[] = "Vendor ID";
static const char acName1018_02[] = "Product Code";
static const char acName1018_03[] = "Revision Number";
static const char acName1018_04[] = "Serial Number";
static const char acName1600[] = "LEDs";
static const char acName1600_00[] = "Max SubIndex";
static const char acName1600_01[] = "LED1";
static const char acName1600_02[] = "LED2";
static const char acName1600_03[] = "LED3";
static const char acName1601[] = "LEDs test map";
static const char acName1601_00[] = "Max SubIndex";
static const char acName1601_01[] = "LED1";
static const char acName1602_02[] = "LED2";
static const char acName1A00[] = "Buttons";
static const char acName1A00_00[] = "Max SubIndex";
static const char acName1A00_01[] = "Button1";
static const char acName1C00[] = "Sync Manager Communication Type";
static const char acName1C00_00[] = "Max SubIndex";
static const char acName1C00_01[] = "Communications Type SM0";
static const char acName1C00_02[] = "Communications Type SM1";
static const char acName1C00_03[] = "Communications Type SM2";
static const char acName1C00_04[] = "Communications Type SM3";
static const char acName1C12[] = "Sync Manager 2 PDO Assignment";
static const char acName1C12_00[] = "Max SubIndex";
static const char acName1C12_01[] = "PDO Mapping";
static const char acName1C13[] = "Sync Manager 3 PDO Assignment";
static const char acName1C13_00[] = "Max SubIndex";
static const char acName1C13_01[] = "PDO Mapping";
static const char acName6000[] = "Buttons";
static const char acName6000_00[] = "Max SubIndex";
static const char acName6000_01[] = "Button1";
static const char acName7000[] = "LEDs";
static const char acName7000_00[] = "Max SubIndex";
static const char acName7000_01[] = "LED1";
static const char acName7000_02[] = "LED2";
static const char acName7000_03[] = "LED3";
static const char acName8000[] = "Parameters";
static const char acName8000_00[] = "Max SubIndex";
static const char acName8000_01[] = "Multiplier";

_objd SDO1000[] =
{
  {0x0, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1000, 0x01901389, NULL},
};
_objd SDO1008[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 88, ATYPE_RO, acName1008, 0, const_cast<void*>(static_cast<const void*>(board_name))},
};
_objd SDO1009[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 0, ATYPE_RO, acName1009, 0, const_cast<void*>(static_cast<const void*>(HW_REV))},
};
_objd SDO100A[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 0, ATYPE_RO, acName100A, 0, const_cast<void*>(static_cast<const void*>(SW_REV))},
};
_objd SDO1018[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1018_00, 4, 0},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, acName1018_01, 0x1337, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_02, 1234, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_03, 0, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_04, 0x00000001, NULL},
};

//mapping割り当て　32bitで割り当て先指定(第3引数)、割当先(第6引数)
//0x1600~何種類も作作れる。どれを使うかは0X1C12で選択
_objd SDO1600[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_00, 3, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_01, 0x70000108, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_02, 0x70000208, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_03, 0x70000308, NULL},
};

_objd SDO1601[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_00, 2, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_01, 0x70000108, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_02, 0x70000208, NULL},
};

//mapping割り当て　32bitで割り当て先指定(第3引数)、割当先(第6引数)
//0x1A00~何種類も作れる。どれを使うかは0X1C13で選択
_objd SDO1A00[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A00_01, 0x60000108, NULL},
};
_objd SDO1C00[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_00, 4, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_01, 1, NULL},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_02, 2, NULL},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_03, 3, NULL},
  {0x04, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_04, 4, NULL},
};
_objd SDO1C12[] =
{
  //0x16~~のからどれ使うか。
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1C12_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RW, acName1C12_01, 0x1600, NULL},
};
_objd SDO1C13[] =
{
  //0x1A~~からどれ使うか
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RW, acName1C13_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RW, acName1C13_01, 0x1A00, NULL},
};
_objd SDO6000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName6000_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName6000_01, 0, &Obj.Buttons.Button1},
};
_objd SDO7000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName7000_00, 3, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName7000_01, 0, &Obj.LEDs.LED1},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName7000_02, 0, &Obj.LEDs.LED2},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName7000_03, 0, &Obj.LEDs.LED3}
};
_objd SDO8000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName8000_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, acName8000_01, 0, &Obj.Parameters.Multiplier},
};

_objectlist SDOobjects[] =
{
  {0x1000, OTYPE_VAR, 0, 0, acName1000, SDO1000},
  {0x1008, OTYPE_VAR, 0, 0, acName1008, SDO1008},
  {0x1009, OTYPE_VAR, 0, 0, acName1009, SDO1009},
  {0x100A, OTYPE_VAR, 0, 0, acName100A, SDO100A},
  {0x1018, OTYPE_RECORD, 4, 0, acName1018, SDO1018},
  {0x1600, OTYPE_RECORD, 3, 0, acName1600, SDO1600},
  {0x1601, OTYPE_RECORD, 2, 0, acName1601, SDO1601},
  {0x1A00, OTYPE_RECORD, 1, 0, acName1A00, SDO1A00},
  {0x1C00, OTYPE_ARRAY, 4, 0, acName1C00, SDO1C00},
  {0x1C12, OTYPE_ARRAY, 1, 0, acName1C12, SDO1C12},
  {0x1C13, OTYPE_ARRAY, 1, 0, acName1C13, SDO1C13},
  {0x6000, OTYPE_RECORD, 1, 0, acName6000, SDO6000},
  {0x7000, OTYPE_RECORD, 3, 0, acName7000, SDO7000},
  {0x8000, OTYPE_RECORD, 1, 0, acName8000, SDO8000},
  {0xffff, 0xff, 0xff, 0xff, NULL, NULL}
};
