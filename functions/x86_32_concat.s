_F6concatSS:
push ebp
mov ebp, esp
mov ebx, [ebp + 16]
mov ecx, [ebp + 8]
add ebx, ecx
push ebx
call eddi_alloc
add esp, 8
mov edi, eax
mov ecx, [ebp + 16]
mov esi, [ebp + 20]
rep movsb
mov ecx, [ebp + 8]
mov esi, [ebp + 12]
rep movsb
leave
ret
