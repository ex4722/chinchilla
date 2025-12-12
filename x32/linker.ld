ENTRY(_start)

SECTIONS {
    . = 1M;
	kernel32_start = .;

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }
    
        /* Read-only data. */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /* Creates a stack */ 
    . += 0x4000;
    stack_top = .;

    kernel32_reserved_start = ALIGN(4k);
    /*kernel_pgd = ALIGN(4K);*/
    kernel_pgd = kernel32_reserved_start;
	. += 0x1000;
    kernel_pud = ALIGN(4K);
	. += 0x1000;

    .gdt BLOCK(4k) : ALIGN(4k) {
        *(.gdt)
    }
    kernel32_reserved_end = ALIGN(4k);
    kernel32_end = .;
}
