_F5printI:
push ebp
mov ebp, esp
push eax
push ebx
push ecx
push edx
push esi
mov eax, ecx
xor esi, esi
cmp eax, 0
jge .loop
neg eax
push eax
push 1
push S2
call _F5printS
add esp, 8
pop eax
.loop:
mov edx, 0
mov ebx, 10
div ebx
add edx, 48
push edx
inc esi
cmp eax, 0
jz .next
jmp .loop
.next:
cmp esi, 0
jz .exit
dec esi
mov eax, 4
mov ecx, esp
mov ebx, 1
mov edx, 1
int 80h
add esp, 4
jmp .next
.exit:
pop esi
pop edx
pop ecx
pop ebx
pop eax
leave
ret
