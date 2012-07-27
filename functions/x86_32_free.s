_F4freePII:
push ebp
mov ebp, esp

;block->available = 1
mov dword [ecx - 8], 1

leave
ret
