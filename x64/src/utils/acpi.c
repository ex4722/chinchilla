#include "utils/acpi.h"
#include "utils/assert.h"
#include "utils/log.h"

// EBDA address found here https://wiki.osdev.org/Memory_Map_(x86)#Overview
#define EBDA_START 0x80000
#define RSDP_HEADER "RSD PTR "
#define RSDP_HEADER_LENGTH (sizeof(RSDP_HEADER) - 1)
#define FADT_HEADER "FACP"
#define FADT_HEADER_LENGTH (sizeof(FADT_HEADER) - 1)
#define MADT_HEADER "APIC"
#define MADT_HEADER_LENGTH (sizeof(MADT_HEADER) - 1)

// https://wiki.osdev.org/RSDP#Checksum_validation
bool rsdp_checksum(struct RSDP_t *rsdp) {
    u8 sum = 0;
    for (u32 i = 0; i < sizeof(struct RSDP_t); i++) {
        // Add up each byte and make sure lowest byte equal zero
        sum += ((u8 *)rsdp)[i];
    }
    return sum == 0;
}

bool xsdp_checksum(struct XSDP_t *xsdp) {
    u8 sum = 0;
    for (u32 i = 0; i < sizeof(struct XSDP_t); i++) {
        // Add up each byte and make sure lowest byte equal zero
        sum += ((u8 *)xsdp)[i];
    }
    return sum == 0;
}

// Technique used https://wiki.osdev.org/RSDP#Detecting_the_RSDP
u64 find_rsdp() {
    // look in the first kb of EBDA + Header is 16 byte aligned
    for (u32 i = 0; i < 0x400; i += 0x10) {
        if (!memcmp((void *)EBDA_START + i, RSDP_HEADER, RSDP_HEADER_LENGTH)) {
            // found the string
            return EBDA_START + i;
        }
    }

    // Look in main BIOS area under 1MB
    u64 bios_start = 0xE0000;
    u64 bios_size = 128 * KILOBTYE; // 128 kb

    // look in the 128 kb of bios region + Header is 16 byte aligned
    for (u32 i = 0; i < bios_size; i += 0x10) {
        if (!memcmp((void *)bios_start + i, RSDP_HEADER, RSDP_HEADER_LENGTH)) {
            // found the string
            return bios_start + i;
        }
    }
    return 0;
}

struct MADT *find_madt() {
    struct RSDP_t *rsdp = (struct RSDP_t *)find_rsdp();
    // version 1 aka rsdp
    if (rsdp->Revision == 0) {
        ASSERT(!rsdp_checksum(rsdp), "RSDP invalid checksum");
    } else {
        // other versions, cast rsdp into xsdp
        struct XSDP_t *xsdp = (struct XSDP_t *)rsdp;
        ASSERT(!xsdp_checksum(xsdp), "XSDP invalid checksum");
    }

    struct RSDT *rsdt = (struct RSDT *)(u64)rsdp->RsdtAddress;

    u32 entries = (rsdt->header.Length - sizeof(struct ACPISDTHeader)) / 4;
    for (u32 i = 0; i < entries; i++) {
        struct ACPISDTHeader *entry =
            (struct ACPISDTHeader *)(u64)rsdt->SDT_pointers[i];
        if (!memcmp(entry->Signature, MADT_HEADER, MADT_HEADER_LENGTH)) {
            PINFO("FOUND %s: %#lx\n", MADT_HEADER, entry);
            return (struct MADT *)entry;
        }
    }
    return NULL;
}

u64 get_apic_addr() {
    // https://wiki.osdev.org/ACPI#Switching_to_ACPI_Mode
    struct MADT *madt = find_madt();
    ASSERT(madt == NULL, "MADT Not found");

    PINFO("Madt found @ 0x%p with length of %d\n", madt, madt->h.Length);

    // if bit 0 in flags mask all the 8259 PIC's interrupts
    // APIC replaces this???
    // if(madt->flags & 1){
    // } 

    int e = 0;
    // get the end of the madt struct which is start of apic entries
    struct madt_header *madt_cursor = (struct madt_header *)(&madt->data);
    void *madt_end = (void*) ((u64)madt+ madt->h.Length);
    while ((void *)madt_cursor < madt_end) {
        e += 1;
        if (e > 30){
            break;
        }
        printf("Processin type %d of length %d @ %p\n", madt_cursor->type, madt_cursor->length, madt_cursor);
        switch (madt_cursor->type) {
        case ET_PROCESSOR_APIC:
            struct processor_local_apic_entry *processor_apic =
                (struct processor_local_apic_entry *)madt_cursor;
            if (processor_apic->flags & APIC_CPU_ENABLED) {
                printf("Processor %d is enabled\n", processor_apic->acpi_processor_id);
            } 
            if (!(processor_apic->flags & APIC_ONLINE_CAP)) {
                printf("Processor %d is not capable of being turned on\n",
                        processor_apic->apic_id);
            }
            break;
        case ET_IO_APIC:
            struct io_apic_entry * apic_entry = (struct io_apic_entry *)madt_cursor;
            break;
        case ET_ADDR_OVERRIDE_APIC:
            break;
        }
        madt_cursor = (struct madt_header *)((u64)madt_cursor + madt_cursor->length);
    }

    return madt->apci_address;
}
