_F5allocI:
push ebp
mov ebp, esp

push ebx
push ecx
push edx
push esi
push edi

mov ebx, [Veddi_remaining]
cmp ecx, ebx
jle .alloc_normal
mov eax, 45
xor ebx, ebx
int 80h
mov esi, eax
mov ebx, eax
add ebx, 16384
mov eax, 45
int 80h
mov edi, eax
sub edi, 4
mov ecx, 4096
xor eax, eax
std
rep stosb
cld
mov eax, esi
mov dword [Veddi_remaining], 16384
mov [Veddi_current], esi
.alloc_normal:
mov eax, [Veddi_current]
mov ebx, [Veddi_current]
add ebx, ecx
mov [Veddi_current], ebx
mov ebx, [Veddi_remaining]
sub ebx, ecx
mov [Veddi_remaining], ebx
.alloc_end:

pop edi
pop esi
pop edx
pop ecx
pop ebx

leave
ret
