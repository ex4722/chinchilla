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

struct MADT *madt = NULL;
struct FADT *fadt = NULL;

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

// IOREGSEL and IOREGWIN MMIO register selects to access IO APIC registers
// IOREGWIN = IOREGSEL + 0x10
// https://wiki.osdev.org/IOAPIC
void write_ioapic_register(uintptr_t apic_base, uint8_t offset, uint32_t val) {
    *(volatile uint32_t *)apic_base = offset;
    *(volatile uint32_t *)(apic_base + 0x10) = val;
}

uint32_t read_ioapic_register(uintptr_t apic_base, uint8_t offset) {
    *(volatile uint32_t *)apic_base = offset;
    return *(volatile uint32_t *)(apic_base + 0x10);
}

uint32_t get_ioapic_version(uintptr_t apic_base) {
    return read_ioapic_register(apic_base, IOAPICVER);
}

struct MADT *parse_rsdt() {
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
            madt = (struct MADT *)entry;
        }
        if (!memcmp(entry->Signature, FADT_HEADER, FADT_HEADER_LENGTH)) {
            PINFO("FOUND %s: %#lx\n", FADT_HEADER, entry);
            fadt = (struct FADT *)entry;
        }
    }
    return NULL;
}

u64 get_apic_addr() {
    parse_rsdt();
    // https://wiki.osdev.org/ACPI#Switching_to_ACPI_Mode
    ASSERT(madt == NULL, "MADT Not found");

    PINFO("Madt found @ 0x%p with length of %d\n", madt, madt->h.Length);

    // if bit 0 in flags mask all the 8259 PIC's interrupts
    // APIC replaces this???
    // if(madt->flags & 1){
    // }

    int e = 0;
    // get the end of the madt struct which is start of apic entries
    struct madt_header *madt_cursor = (struct madt_header *)(&madt->data);
    void *madt_end = (void *)((u64)madt + madt->h.Length);
    while ((void *)madt_cursor < madt_end) {
        e += 1;
        if (e > 30) {
            break;
        }
        printf("Processin type %d of length %d @ %p\n", madt_cursor->type,
               madt_cursor->length, madt_cursor);
        switch (madt_cursor->type) {
        case ET_PROCESSOR_APIC:
            struct processor_local_apic_entry *processor_apic =
                (struct processor_local_apic_entry *)madt_cursor;
            if (processor_apic->flags & APIC_CPU_ENABLED) {
                printf("Processor %d is able to be enabled\n",
                       processor_apic->acpi_processor_id);
            } else {
                if (!(processor_apic->flags & APIC_ONLINE_CAP)) {
                    printf("Processor %d is capable of being turned on\n",
                           processor_apic->apic_id);
                } else {
                    printf("Processor %d is not capable of being turned on\n",
                           processor_apic->apic_id);
                }
            }
            break;
        case ET_IO_APIC:
            // https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
            struct io_apic_entry *io_apic_entry =
                (struct io_apic_entry *)madt_cursor;
            u32 version = get_ioapic_version(io_apic_entry->io_apic_address);
            size_t number_of_inputs = ((version >> 16) & 0xFF) + 1;

            printf(
                "APIC ID %d handles interupt number %d to %d io_adder 0x%p\n",
                io_apic_entry->io_apic_id, io_apic_entry->global_interrupt_base,
                io_apic_entry->global_interrupt_base + number_of_inputs,
                io_apic_entry->io_apic_address);
            break;
        case ET_IO_INT_APIC:
            struct io_apic_interrupt_source_override_entry *io_override =
                (struct io_apic_interrupt_source_override_entry *)madt_cursor;
            printf("Bus src %d IRQ src %d Global Inter %d Flags %d\n",
                   io_override->bus_source, io_override->irq_source,
                   io_override->global_interrupt, io_override->flags);
            break;
        case ET_ADDR_OVERRIDE_APIC:
            break;
        }
        madt_cursor =
            (struct madt_header *)((u64)madt_cursor + madt_cursor->length);
    }

    return madt->apci_address;
}

bool acpi_timer_enabled() {
    parse_rsdt();
    return (fadt->PMTimerLength == 4);
}

bool setup_acpi_timer() {
    if (!acpi_timer_enabled()) {
        // acpi not enabled
        return false;
    };
    parse_rsdt();
    if (madt->h.Revision >= 2) {
        if (fadt->X_PMTimerBlock.Address != 0) {
            // use X_PMTimerBlock
            printf("Use X_PMTimerBlock\n");
            printf("addr space %d\n", fadt->X_PMTimerBlock.AddressSpace);
            printf("addr space %x\n", fadt->X_PMTimerBlock.Address);
            if (fadt->X_PMTimerBlock.AddressSpace == ACPI_GAS_SYSTEM_MEMORY) {
                printf("Timer value is in memory %x\n", *(u32*)fadt->X_PMTimerBlock.Address);
            }else{
                printf("FUCK");
            }
        };
    } else {
        // use PMTimerBlock
        printf("Use PMTimerBlock\n");
    };

    if (fadt->Flags & (1 << 8)) {
        printf("32 bit counter\n");
    }
    return true;
}
