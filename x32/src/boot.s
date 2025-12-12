; Declare constants for the multiboot header.
MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; checksum of above, to prove we are multiboot


; Constants for serial 
SERIAL_PORT equ 0x3f8 ; COM1 

; Contants for segment descriptor access bytes  
; https://wiki.osdev.org/Global_Descriptor_Table
GDT_PRESENT equ        1 << 7      ; Must be 1 for valid segment
GDT_PRIV_LEVEL equ     1 << 5      ; 2 bits for CPU Priv level 0-> Kernel 3-> Userspace
GDT_TYPE equ           1 << 4      ; 0 is segment reg, 1 is code/data
GDT_EXEC equ           1 << 3      ; 0 is data segment, 1 is code segment 
GDT_DIRECTION equ      1 << 2      ; For data: Grow up or down, For Code: Priv level to execute
GDT_RW equ             1 << 1      ; For data: 0 -> no write, for code: 0 -> no read
GDT_ACCESS equ         1 << 0      ; Changed to 1 as access flag, keep as 1 unless otherwise needed


; Segment descriptor Flags bits, adding 4 as flags and limit share a start
; https://wiki.osdev.org/Global_Descriptor_Table
GDT_GRAN_4K       equ 1 << (3 + 4)
GDT_SZ_32         equ 1 << (2 + 4)
GDT_LONG_MODE     equ 1 << (1 + 4)


; Random bits 
PAE_BIT equ        5
LME_BIT equ        8 ; long mode enable
PG_BIT equ         31 

; MSR
EFER_MSR equ       0xC0000080


KERNEL64_ENTRY equ 0x200000

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .data
extern stack_top
extern main 
section .text
global _start
_start:
    mov esp, stack_top
    push ebx ; this is a multiboot_info_t structure referenced in documentation for multiboot
    call main
repeat:
    jmp repeat

global inb:
inb:
    mov dx, [esp + 4]
    in al, dx 
    ret

global outb; meaning of this?
outb:
    mov dx, [esp + 4] ; how does stack look?
    mov al, [esp + 8] ; how does stack look?
    ; esp is ret ad4dr 
    ; esp + 4 is value?
    out dx, al
    ret
global enter_long_mode 
; Function will be called with a pointer to pgdir 
enter_long_mode: 
	mov eax, [esp+4] ; get the value of pgdir
	mov edi, [esp+8] ;get the value of the multiboot information structure

    ; Load CR3 with the physical address of the PML4 (Level 4 Page Map)
    mov eax, [esp+4]
    mov cr3, eax

    ; Disable paging
    mov eax, cr0
    and eax, 0x7FFFFFFF
    mov cr0, eax

    ;call enable_pae 
    mov eax, cr4
    or eax, 1 << PAE_BIT
    mov cr4, eax

    ; Enable long mode by setting the LME flag (bit 8) in MSR 0xC0000080 (aka EFER)
    mov ecx, EFER_MSR 
    rdmsr
    or eax, (1 << LME_BIT)    ; eax has lower 32
    wrmsr
    
    ;; Enable paging
    mov eax, cr0
    or eax, (1 << PG_BIT) 
    mov cr0, eax

    ; Setup GDT Table and CS Register 
    lgdt [GDT64.Pointer]         ; Load the 64-bit global descriptor table.
    jmp GDT64.Code:Realm64       ; Set the code segment and enter 64-bit long mode.


global enable_pae
enable_pae:
    ; Set the PAE enable bit in CR4(Bit 5)
    mov eax, cr4
    or eax, 1 << PAE_BIT
    mov cr4, eax
    ret

; Can use full registers at this point now
[BITS 64]
Realm64:
    cli                           ; Clear the interrupt flag.
    mov ax, GDT64.Data            ; Set the A-register to the data descriptor.
    mov ds, ax                    ; Set the data segment to the A-register.
    mov es, ax                    ; Set the extra segment to the A-register.
    mov fs, ax                    ; Set the F-segment to the A-register.
    mov gs, ax                    ; Set the G-segment to the A-register.
    mov ss, ax                    ; Set the stack segment to the A-register.
    mov rax, KERNEL64_ENTRY       ; Load the 64 bit kernel entry and jmp
    jmp rax


global die
die:
    hlt

section .gdt
; Creates the data region for the GDT
; https://wiki.osdev.org/Setting_Up_Long_Mode#Entering_Long_Mode 
GDT64:
    .Null: equ $ - GDT64
        dq 0
    .Code: equ $ - GDT64
        dd 0xFFFF                                      ; Limit & Base (low, bits 0-15)
        db 0                                           ; Base (mid, bits 16-23)
        db GDT_PRESENT | GDT_TYPE | GDT_EXEC | GDT_RW  ; Access
        db GDT_GRAN_4K | GDT_LONG_MODE | 0xF           ; Flags & Limit (high, bits 16-19)
        db 0                                           ; Base (high, bits 24-31)
    .Data: equ $ - GDT64
        dd 0xFFFF                                      ; Limit & Base (low, bits 0-15)
        db 0                                           ; Base (mid, bits 16-23)
        db GDT_PRESENT | GDT_TYPE | GDT_RW             ; Access
        db GDT_GRAN_4K | GDT_LONG_MODE | 0xF               ; Flags & Limit (high, bits 16-19)
        db 0                                           ; Base (high, bits 24-31)
    .TSS: equ $ - GDT64
        dd 0x00000068
        dd 0x00CF8900
    .Pointer:
        dw $ - GDT64 - 1
        dq GDT64
