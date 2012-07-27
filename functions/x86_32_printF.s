_F5printF:
push ebp
mov ebp, esp
push eax
push ebx
push ecx
sub esp, 4
movd [esp], xmm0
sub esp, 4
movd [esp], xmm1
sub esp, 4
movd [esp], xmm2
cvttss2si ebx, xmm7
cvtsi2ss xmm1, ebx
mov ecx, ebx
call _F5printI
push 1
push S4
call _F5printS
add esp, 8
or ebx, ebx
jge .pos
mov ebx, __float32__(-1.0)
movd xmm2, ebx
mulss xmm7, xmm2
mulss xmm1, xmm2
.pos:
subss xmm7, xmm1
mov ecx, __float32__(10000.0)
movd xmm2, ecx
mulss xmm7, xmm2
cvttss2si ebx, xmm7
mov eax, ebx
or eax, eax
je .end
xor ecx, ecx
.start:
cmp eax, 1000
jge .end
call _F5printI
imul eax, 10
jmp .start
.end:
mov ecx, ebx
call _F5printI
movd xmm2, [esp]
add esp, 4
movd xmm1, [esp]
add esp, 4
movd xmm0, [esp]
add esp, 4
pop ecx
pop ebx
pop eax
leave
ret
