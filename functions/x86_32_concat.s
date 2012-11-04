_F6concatSS:
push ebp
mov ebp, esp

push ecx
push esi
push edi

mov ebx, [ebp + 16]
mov ecx, [ebp + 8]
add ebx, ecx

mov ecx, ebx
call _F5allocI
mov edi, eax

mov ecx, [ebp + 16]
mov esi, [ebp + 20]
rep movsb
mov ecx, [ebp + 8]
mov esi, [ebp + 12]
rep movsb

pop edi
pop esi
pop ecx

leave
ret
