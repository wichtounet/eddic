_F4freePII:
push ebp
mov ebp, esp

push edx
push eax

;eax = size
mov eax, [ebp + 8]
lea edx, [ecx - 4]
sub edx, eax

;block->available = 1
mov dword [edx], 1

pop eax
pop ecx

leave
ret
