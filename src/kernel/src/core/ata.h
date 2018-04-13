/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_KERNEL_SRC_CORE_ATA_H
#define GUARD_KERNEL_SRC_CORE_ATA_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <hybrid/align.h>
#include <kos/kdev_t.h>
#include <fs/device.h>

DECL_BEGIN

/* The default and seemingly universal sector size for CD-ROMs. */
#define ATAPI_SECTOR_SIZE   2048

#define ATA_SECTOR_SIZE     512

 
/* The necessary I/O ports, indexed by "bus". */
#define ATA_DATA(bus)          (bus)
#define ATA_FEATURES(bus)     ((bus)+1)
#define ATA_SECTOR_COUNT(bus) ((bus)+2)
#define ATA_ADDRESS1(bus)     ((bus)+3)
#define ATA_ADDRESS2(bus)     ((bus)+4)
#define ATA_ADDRESS3(bus)     ((bus)+5)
#define ATA_DRIVE_SELECT(bus) ((bus)+6)
#define ATA_STATUS(bus)       ((bus)+7)
#define ATA_COMMAND(bus)      ((bus)+7)
#   define ATA_COMMAND_READ_SECTORS    0x20
#   define ATA_COMMAND_READ_PIO        0x20
#   define ATA_COMMAND_READ_PIO_EXT    0x24
#   define ATA_COMMAND_READ_DMA        0xc8
#   define ATA_COMMAND_READ_DMA_EXT    0x25
#   define ATA_COMMAND_WRITE_PIO       0x30
#   define ATA_COMMAND_WRITE_PIO_EXT   0x34
#   define ATA_COMMAND_WRITE_DMA       0xca
#   define ATA_COMMAND_WRITE_DMA_EXT   0x35
#   define ATA_COMMAND_CACHE_FLUSH     0xe7
#   define ATA_COMMAND_CACHE_FLUSH_EXT 0xea
#   define ATA_COMMAND_PACKET          0xa0
#   define ATA_COMMAND_IDENTIFY_PACKET 0xa1
#   define ATA_COMMAND_IDENTIFY        0xec
#define ATA_DCR(bus)          ((bus)+0x206)   /* device control (write) / alternate status (read) register */
#   define ATA_DCR_ERR   0x01 /* 1 << 0 */
#   define ATA_DCR_IDX   0x02 /* 1 << 1 */
#   define ATA_DCR_CORR  0x04 /* 1 << 2 */
#   define ATA_DCR_DRQ   0x08 /* 1 << 3 */
#   define ATA_DCR_DSC   0x10 /* 1 << 4 */
#   define ATA_DCR_DF    0x20 /* 1 << 5 */
#   define ATA_DCR_DRDY  0x40 /* 1 << 6 */
#   define ATA_DCR_BSY   0x80 /* 1 << 7 */
#   define ATA_CTRL_NIEN 0x02 /* Disable interrupts. */
#   define ATA_CTRL_SRST 0x04 /* Software reset bit. */
#   define ATA_CTRL_HOB  0x80 /* Read back the last high-order byte of the last LBA-48 value sent. */

/* valid values for "bus" */
#define ATA_BUS_PRIMARY     0x1f0
#define ATA_BUS_SECONDARY   0x170
#define ATA_BUS_FPRIMARY    0x080 /* BIT: When set, is primary. */

/* valid values for "drive" */
#define ATA_DRIVE_MASTER    0xa0
#define ATA_DRIVE_SLAVE     0xb0
#define ATA_DRIVE_FSLAVE    0x10 /* BIT: When set, use slave. */




/* Device numbers for ATA drives.
 * NOTE: Individual partitions can be accessed 0..63 through minor numbers. */
#define DEVNO_ATA_PRIMARY_MASTER    MKDEV(3,0)
#define DEVNO_ATA_PRIMARY_SLAVE     MKDEV(3,64)
#define DEVNO_ATA_SECONDARY_MASTER  MKDEV(22,0)
#define DEVNO_ATA_SECONDARY_SLAVE   MKDEV(22,64)
/* HINT: Find device numbers here:
 * http://www.lanana.org/docs/device-list/devices-2.6+.txt */


#ifdef __CC__
 
/* ATA specifies a 400ns delay after drive switching -- often
 * implemented as 4 Alternative Status queries. */
#define ATA_SELECT_DELAY(bus) \
  (inb(ATA_DCR(bus)),inb(ATA_DCR(bus)),inb(ATA_DCR(bus)),inb(ATA_DCR(bus)))


#define UINT unsigned int


typedef struct PACKED {
    struct PACKED {
        UINT     Reserved1          : 1;
        UINT     Retired3           : 1;
        UINT     ResponseIncomplete : 1;
        UINT     Retired2           : 3;
        UINT     FixedDevice        : 1;
        UINT     RemovableMedia     : 1;
        UINT     Retired1           : 7;
        UINT     DeviceType         : 1;
    } GeneralConfiguration;
    u16          NumCylinders;
    u16          ReservedWord2;
    u16          NumHeads;
    u16          Retired1[2];
    u16          NumSectorsPerTrack;
    u16          VendorUnique1[3];
    u8           SerialNumber[20];
    u16          Retired2[2];
    u16          Obsolete1;
    u8           FirmwareRevision[8];
    u8           ModelNumber[40];
    u8           MaximumBlockTransfer;
    u8           VendorUnique2;
    u16          ReservedWord48;
    struct PACKED {
        u8       ReservedByte49;
        UINT     DmaSupported         : 1;
        UINT     LbaSupported         : 1;
        UINT     IordyDisable         : 1;
        UINT     IordySupported       : 1;
        UINT     Reserved1            : 1;
        UINT     StandybyTimerSupport : 1;
        UINT     Reserved2            : 2;
        u16      ReservedWord50;
    } Capabilities;
    u16          ObsoleteWords51[2];
    UINT         TranslationFieldsValid : 3;
    UINT         Reserved3              : 13;
    u16          NumberOfCurrentCylinders;
    u16          NumberOfCurrentHeads;
    u16          CurrentSectorsPerTrack;
    u32          CurrentSectorCapacity;
    u8           CurrentMultiSectorSetting;
    UINT         MultiSectorSettingValid : 1;
    UINT         ReservedByte59          : 7;
    u32          UserAddressableSectors;
    u16          ObsoleteWord62;
    UINT         MultiWordDMASupport : 8;
    UINT         MultiWordDMAActive  : 8;
    UINT         AdvancedPIOModes    : 8;
    UINT         ReservedByte64      : 8;
    u16          MinimumMWXferCycleTime;
    u16          RecommendedMWXferCycleTime;
    u16          MinimumPIOCycleTime;
    u16          MinimumPIOCycleTimeIORDY;
    u16          ReservedWords69[6];
    UINT         QueueDepth     : 5;
    UINT         ReservedWord75 : 11;
    u16          ReservedWords76[4];
    u16          MajorRevision;
    u16          MinorRevision;
    struct PACKED {
        UINT     SmartCommands         : 1;
        UINT     SecurityMode          : 1;
        UINT     RemovableMediaFeature : 1;
        UINT     PowerManagement       : 1;
        UINT     Reserved1             : 1;
        UINT     WriteCache            : 1;
        UINT     LookAhead             : 1;
        UINT     ReleaseInterrupt      : 1;
        UINT     ServiceInterrupt      : 1;
        UINT     DeviceReset           : 1;
        UINT     HostProtectedArea     : 1;
        UINT     Obsolete1             : 1;
        UINT     WriteBuffer           : 1;
        UINT     ReadBuffer            : 1;
        UINT     Nop                   : 1;
        UINT     Obsolete2             : 1;
        UINT     DownloadMicrocode     : 1;
        UINT     DmaQueued             : 1;
        UINT     Cfa                   : 1;
        UINT     AdvancedPm            : 1;
        UINT     Msn                   : 1;
        UINT     PowerUpInStandby      : 1;
        UINT     ManualPowerUp         : 1;
        UINT     Reserved2             : 1;
        UINT     SetMax                : 1;
        UINT     Acoustics             : 1;
        UINT     BigLba                : 1;
        UINT     DeviceConfigOverlay   : 1;
        UINT     FlushCache            : 1;
        UINT     FlushCacheExt         : 1;
        UINT     Resrved3              : 2;
        UINT     SmartErrorLog         : 1;
        UINT     SmartSelfTest         : 1;
        UINT     MediaSerialNumber     : 1;
        UINT     MediaCardPassThrough  : 1;
        UINT     StreamingFeature      : 1;
        UINT     GpLogging             : 1;
        UINT     WriteFua              : 1;
        UINT     WriteQueuedFua        : 1;
        UINT     WWN64Bit              : 1;
        UINT     URGReadStream         : 1;
        UINT     URGWriteStream        : 1;
        UINT     ReservedForTechReport : 2;
        UINT     IdleWithUnloadFeature : 1;
        UINT     Reserved4             : 2;
    } CommandSetSupport,CommandSetActive;
    UINT         UltraDMASupport : 8;
    UINT         UltraDMAActive  : 8;
    u16          ReservedWord89[4];
    u16          HardwareResetResult;
    UINT         CurrentAcousticValue     : 8;
    UINT         RecommendedAcousticValue : 8;
    u16          ReservedWord95[5];
    union PACKED {
       u32       Max48BitLBA[2];
       u64       UserAddressableSectors48;
    };
    u16          StreamingTransferTime;
    u16          ReservedWord105;
    struct PACKED {
        UINT     LogicalSectorsPerPhysicalSector         : 4;
        UINT     Reserved0                               : 8;
        UINT     LogicalSectorLongerThan256Words         : 1;
        UINT     MultipleLogicalSectorsPerPhysicalSector : 1;
        UINT     Reserved1                               : 2;
    }            PhysicalLogicalSectorSize;
    u16          InterSeekDelay;
    u16          WorldWideName[4];
    u16          ReservedForWorldWideName128[4];
    u16          ReservedForTlcTechnicalReport;
    u16          WordsPerLogicalSector[2];
    struct PACKED {
        UINT     ReservedForDrqTechnicalReport : 1;
        UINT     WriteReadVerifyEnabled        : 1;
        UINT     Reserved01                    : 11;
        UINT     Reserved1                     : 2;
    } CommandSetSupportExt,CommandSetActiveExt;
    u16          ReservedForExpandedSupportandActive[6];
    UINT         MsnSupport       : 2;
    UINT         ReservedWord1274 : 14;
    struct PACKED {
        UINT     SecuritySupported              : 1;
        UINT     SecurityEnabled                : 1;
        UINT     SecurityLocked                 : 1;
        UINT     SecurityFrozen                 : 1;
        UINT     SecurityCountExpired           : 1;
        UINT     EnhancedSecurityEraseSupported : 1;
        UINT     Reserved0                      : 2;
        UINT     SecurityLevel                  : 1;
        UINT     Reserved1                      : 7;
    } SecurityStatus;
    u16          ReservedWord129[31];
    struct PACKED {
        UINT     MaximumCurrentInMA2   : 12;
        UINT     CfaPowerMode1Disabled : 1;
        UINT     CfaPowerMode1Required : 1;
        UINT     Reserved0             : 1;
        UINT     Word160Supported      : 1;
    } CfaPowerModel;
    u16          ReservedForCfaWord161[8];
    struct PACKED {
        UINT     SupportsTrim : 1;
        UINT     Reserved0    : 15;
    } DataSetManagementFeature;
    u16          ReservedForCfaWord170[6];
    u16          CurrentMediaSerialNumber[30];
    u16          ReservedWord206;
    u16          ReservedWord207[2];
    struct PACKED {
        UINT     AlignmentOfLogicalWithinPhysical : 14;
        UINT     Word209Supported                 : 1;
        UINT     Reserved0                        : 1;
    } BlockAlignment;
    u16          WriteReadVerifySectorCountMode3Only[2];
    u16          WriteReadVerifySectorCountMode2Only[2];
    struct PACKED {
        UINT     NVCachePowerModeEnabled  : 1;
        UINT     Reserved0                : 3;
        UINT     NVCacheFeatureSetEnabled : 1;
        UINT     Reserved1                : 3;
        UINT     NVCachePowerModeVersion  : 4;
        UINT     NVCacheFeatureSetVersion : 4;
    } NVCacheCapabilities;
    u16          NVCacheSizeLSW;
    u16          NVCacheSizeMSW;
    u16          NominalMediaRotationRate;
    u16          ReservedWord218;
    struct PACKED {
        u8       NVCacheEstimatedTimeToSpinUpInSeconds;
        u8       Reserved;
    } NVCacheOptions;
    u16          ReservedWord220[35];
    UINT         Signature : 8;
    UINT         CheckSum  : 8;
} AtaDeviceSpecifications;
#undef UINT


typedef struct {
    struct block_device a_device; /* Underlying block-device. */
    u16                 a_bus;    /* [const] The BUS of this ATA device. */
    u8                  a_drive;  /* [const] The DRIVE of this ATA device. */
    u8                  a_pad;    /* ... */
    struct PACKED {
        u16             a_cylinders;
        u8              a_sectors_per_track;
        u8              a_number_of_heads;
    }                   a_chs_info;
} AtaDevice;



#if 0
#define ATA_PRIMARY        0x1f0 /* ... 0x1f7 */
#define ATA_SECONDARY      0x170 /* ... 0x177 */

/* Device control register/Alternate status ports. */
#define ATA_CTRL_PRIMARY   0x3f6
#define ATA_CTRL_SECONDARY 0x376


/* Register offsets from `ATA_SECONDARY' */
#define ATA_PORT(x)    x
#define ATA_DATA       ATA_PORT(0x00)
#define ATA_ERROR      ATA_PORT(0x01)
#define ATA_FEATURES   ATA_PORT(0x01)
#define ATA_SECCOUNT0  ATA_PORT(0x02)
#define ATA_LBALO      ATA_PORT(0x03)
#define ATA_LBAMD      ATA_PORT(0x04)
#define ATA_LBAHI      ATA_PORT(0x05)
#define ATA_HDDEVSEL   ATA_PORT(0x06)
#   define ATA_DRIVE_MASTER  0xa0
#   define ATA_DRIVE_SLAVE   0xb0
#define ATA_CMD        ATA_PORT(0x07)
#   define ATA_CMD_READ_PIO        0x20
#   define ATA_CMD_READ_PIO_EXT    0x24
#   define ATA_CMD_READ_DMA        0xc8
#   define ATA_CMD_READ_DMA_EXT    0x25
#   define ATA_CMD_WRITE_PIO       0x30
#   define ATA_CMD_WRITE_PIO_EXT   0x34
#   define ATA_CMD_WRITE_DMA       0xca
#   define ATA_CMD_WRITE_DMA_EXT   0x35
#   define ATA_CMD_CACHE_FLUSH     0xe7
#   define ATA_CMD_CACHE_FLUSH_EXT 0xea
#   define ATA_CMD_PACKET          0xa0
#   define ATA_CMD_IDENTIFY_PACKET 0xa1
#   define ATA_CMD_IDENTIFY        0xec
#define ATA_STATUS     ATA_PORT(0x07)
#   define ATA_SR_ERR   0x01 /* 1 << 0 */
#   define ATA_SR_IDX   0x02 /* 1 << 1 */
#   define ATA_SR_CORR  0x04 /* 1 << 2 */
#   define ATA_SR_DRQ   0x08 /* 1 << 3 */
#   define ATA_SR_DSC   0x10 /* 1 << 4 */
#   define ATA_SR_DF    0x20 /* 1 << 5 */
#   define ATA_SR_DRDY  0x40 /* 1 << 6 */
#   define ATA_SR_BSY   0x80 /* 1 << 7 */
#define ATA_SECCOUNT1  ATA_PORT(0x08)
#define ATA_LBA3       ATA_PORT(0x09)
#define ATA_LBA4       ATA_PORT(0x0a)
#define ATA_LBA5       ATA_PORT(0x0b)
#define ATA_CONTROL    ATA_PORT(0x0c)
#define ATA_ALTSTATUS  ATA_PORT(0x0c)
#define ATA_DEVADDRESS ATA_PORT(0x0d)



struct atablkdev {
    struct block_device        a_device;     /* Underlying block device. */
    u16                        a_iobase;     /* ATA I/O base address. */
    u16                        a_ctrl;       /* Device control register/Alternate status port. */
    u8                         a_drive;      /* ATA Drive location. */
    u8                         a_padding[3]; /* ... */
#if defined(CONFIG_DEBUG) && 1
#define ATA_DEFAULT_IO_TIMEOUT   SEC_TO_JIFFIES(1)
#else
#define ATA_DEFAULT_IO_TIMEOUT   SEC_TO_JIFFIES(30)  /* 30 seconds spin-up timeout. */
#endif
#define ATA_DEFAULT_CM_TIMEOUT  MSEC_TO_JIFFIES(200)
    jtime_t                    a_io_timeout; /* Timeout for I/O operations. */
    jtime_t                    a_cm_timeout; /* Timeout for command acknowledgement. */
    struct ata_spec            a_spec;       /* Drive specifications. */
#ifdef __INTELLISENSE__
    u16                        a_number_of_heads;
    u16                        a_cylinders;
    u16                        a_sectors_per_track;
#else
#define a_number_of_heads   a_spec.NumHeads
#define a_cylinders         a_spec.NumCylinders
#define a_sectors_per_track a_spec.NumSectorsPerTrack
#endif
};
#endif
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_SRC_CORE_ATA_H */
