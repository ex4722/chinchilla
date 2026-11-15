#ifndef __ACPI_H
#define __ACPI_H

#include "utils/assert.h"
#include "utils/printf.h"
#include "utils/string.h"
#include "utils/types.h"

struct RSDP_t {
    char Signature[8];
    u8 Checksum;
    char OEMID[6];
    u8 Revision;
    u32 RsdtAddress;
} __attribute__((packed));

struct XSDP_t {
    char Signature[8];
    u8 Checksum;
    char OEMID[6];
    u8 Revision;
    u32 RsdtAddress; // deprecated since version 2.0

    u32 Length;
    u64 XsdtAddress;
    u8 ExtendedChecksum;
    u8 reserved[3];
} __attribute__((packed));

struct ACPISDTHeader {
    char Signature[4];
    u32 Length;
    u8 Revision;
    u8 Checksum;
    char OEMID[6];
    char OEMTableID[8];
    u32 OEMRevision;
    u32 CreatorID;
    u32 CreatorRevision;
};

struct RSDT {
    struct ACPISDTHeader header;
    u32 SDT_pointers[];
};

struct lame_address_struct {
    u8 AddressSpace;
    u8 BitWidth;
    u8 BitOffset;
    u8 AccessSize;
    u64 Address;
};

struct FADT {
    struct ACPISDTHeader h;
    u32 FirmwareCtrl;
    u32 Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    u8 Reserved;

    u8 PreferredPowerManagementProfile;
    u16 SCI_Interrupt;
    u32 SMI_CommandPort;
    u8 AcpiEnable;
    u8 AcpiDisable;
    u8 S4BIOS_REQ;
    u8 PSTATE_Control;
    u32 PM1aEventBlock;
    u32 PM1bEventBlock;
    u32 PM1aControlBlock;
    u32 PM1bControlBlock;
    u32 PM2ControlBlock;
    u32 PMTimerBlock;
    u32 GPE0Block;
    u32 GPE1Block;
    u8 PM1EventLength;
    u8 PM1ControlLength;
    u8 PM2ControlLength;
    u8 PMTimerLength;
    u8 GPE0Length;
    u8 GPE1Length;
    u8 GPE1Base;
    u8 CStateControl;
    u16 WorstC2Latency;
    u16 WorstC3Latency;
    u16 FlushSize;
    u16 FlushStride;
    u8 DutyOffset;
    u8 DutyWidth;
    u8 DayAlarm;
    u8 MonthAlarm;
    u8 Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    u16 BootArchitectureFlags;

    u8 Reserved2;
    u32 Flags;

    // 12 byte structure; see below for details
    struct lame_address_struct ResetReg;

    u8 ResetValue;
    u8 Reserved3[3];

    // 64bit pointers - Available on ACPI 2.0+
    uint64_t X_FirmwareControl;
    uint64_t X_Dsdt;

    struct lame_address_struct X_PM1aEventBlock;
    struct lame_address_struct X_PM1bEventBlock;
    struct lame_address_struct X_PM1aControlBlock;
    struct lame_address_struct X_PM1bControlBlock;
    struct lame_address_struct X_PM2ControlBlock;
    struct lame_address_struct X_PMTimerBlock;
    struct lame_address_struct X_GPE0Block;
    struct lame_address_struct X_GPE1Block;
};

struct MADT{
    struct ACPISDTHeader h;
    u32 apci_address;
    u32 flags;
};

// Entry type's in APIC 
#define ET_PROCESSOR_APIC 0
#define ET_IO_APIC 1 
#define ET_IO_INT_APIC 2 
#define ET_IO_NON_MASK_INT_SRC_APIC 3 
#define ET_IO_NON_MASK_INT_APIC 4
#define ET_ADDR_OVERRIDE_APIC 5 
#define ET_x2APIC 9

// Flags for the various APIC entires
#define APIC_CPU_ENABLED (1)
#define APIC_ONLINE_CAP (1<<1)

// Common header structure for all MADT entries
struct madt_header {
    u8 type;   // Entry type
    u8 length; // Length of the entry
} __attribute__((packed));

// Processor Local APIC Entry
struct processor_local_apic_entry {
    struct madt_header header; // Common header
    u8 acpi_processor_id;      // ACPI Processor ID
    u8 apic_id;                // APIC ID
    u32 flags;                 // Flags: bit 0 = Enabled, bit 1 = Online Capable
} __attribute__((packed));

// I/O APIC Entry
struct io_apic_entry {
    struct madt_header header; // Common header
    u8 io_apic_id;             // I/O APIC ID
    u8 reserved;               // Reserved (0)
    u32 io_apic_address;       // I/O APIC Address
    u32 global_interrupt_base; // Global System Interrupt Base
} __attribute__((packed));

// I/O APIC Interrupt Source Override Entry
struct io_apic_interrupt_source_override_entry {
    struct madt_header header; // Common header
    u8 bus_source;             // Bus Source
    u8 irq_source;             // IRQ Source
    u32 global_interrupt;      // Global System Interrupt
    u16 flags;                 // Flags: trigger mode, polarity
} __attribute__((packed));

// I/O APIC Non-Maskable Interrupt Source Entry
struct io_apic_nmi_source_entry {
    struct madt_header header; // Common header
    u8 nmi_source;             // NMI Source
    u8 reserved;               // Reserved
    u16 flags;                 // Flags: trigger mode, polarity
    u32 global_interrupt;      // Global System Interrupt
} __attribute__((packed));

// Local APIC Non-Maskable Interrupts Entry
struct local_apic_nmi_entry {
    struct madt_header header; // Common header
    u8 acpi_processor_id;      // ACPI Processor ID (0xFF for all processors)
    u16 flags;                 // Flags: trigger mode, polarity
    u8 lint_number;            // LINT# (0 or 1)
} __attribute__((packed));

// Local APIC Address Override Entry
struct local_apic_address_override_entry {
    struct madt_header header; // Common header
    u16 reserved;              // Reserved
    u64 local_apic_address;    // 64-bit physical address of Local APIC
} __attribute__((packed));

// Processor Local x2APIC Entry
struct processor_local_x2apic_entry {
    struct madt_header header; // Common header
    u16 reserved;              // Reserved
    u32 x2apic_id;             // Processor's Local x2APIC ID
    u32 flags;                 // Flags: bit 0 = Enabled, bit 1 = Online Capable
    u32 acpi_processor_id;     // ACPI Processor ID
} __attribute__((packed));




u64 get_apic_addr();
extern void outb(u8, u8);
extern u32 inw(u32);
#endif
