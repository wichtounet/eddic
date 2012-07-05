_F8durationAIAI:
push ebp
mov ebp, esp
mov esi, [ebp + 12]
mov edi, [ebp + 8]
mov eax, [esi - 8]
mov ebx, [edi - 8]
sub eax, ebx
cmp eax, 0
jz .second
cmp eax, 0
jge .push_first
neg eax
.push_first:
mov ecx, eax
call _F5printI
.second:
mov eax, [esi - 4]
mov ebx, [edi - 4]
sub eax, ebx
cmp eax, 0
jge .push_second
neg eax
.push_second:
mov ecx, eax
call _F5printI
leave
ret
