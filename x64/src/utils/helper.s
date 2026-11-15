section .text

global outb
outb:
	mov dx, di
	mov al, sil
	out dx, al
	ret

global inb
inb:
	mov dx, di
	in al, dx
	ret
global inw
inw:
	mov dx, di
	in ax, dx
	ret

; loads value into the lidt register
; extern void lidt(u64 idt_address, u64 size);
global lidt
lidt:
    lidt [rdi]
    ret
