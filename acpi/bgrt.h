#ifndef _BGRT_H_
#define _BGRT_H_

#include <unostype.h>
#include "acpi.h"

typedef struct {
  ACPI_HEADER                     Header;
  ///
  /// 2-bytes (16 bit) version ID. This value must be 1.
  ///
  USINT16                         Version;
  ///
  /// 1-byte status field indicating current status about the table.
  ///     Bits[7:1] = Reserved (must be zero)
  ///     Bit [0] = Valid. A one indicates the boot image graphic is valid.
  ///
  USINT8                          Status;
  ///
  /// 1-byte enumerated type field indicating format of the image.
  ///     0 = Bitmap
  ///     1 - 255  Reserved (for future use)
  ///
  USINT8                          ImageType;
  ///
  /// 8-byte (64 bit) physical address pointing to the firmware's in-memory copy
  /// of the image bitmap.
  ///
  USINT64                         ImageAddress;
  ///
  /// A 4-byte (32-bit) unsigned long describing the display X-offset of the boot image.
  /// (X, Y) display offset of the top left corner of the boot image.
  /// The top left corner of the display is at offset (0, 0).
  ///
  USINT32                         ImageOffsetX;
  ///
  /// A 4-byte (32-bit) unsigned long describing the display Y-offset of the boot image.
  /// (X, Y) display offset of the top left corner of the boot image.
  /// The top left corner of the display is at offset (0, 0).
  ///
  USINT32                         ImageOffsetY;
} ACPI_BGRT;

FUNCWITHSTATUS ShowBGRT(IN ACPI_BGRT *Bgrt);

#endif


