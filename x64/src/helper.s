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