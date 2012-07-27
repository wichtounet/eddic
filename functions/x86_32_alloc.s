_F5allocI:
push ebp
mov ebp, esp

push ebx
push ecx
push edx
push edi
push esi

;Add the size of the header
add ecx, 8

;ebx = start, edx = last
mov eax, [V_mem_start]
mov edx, [V_mem_last]

.start:

;Check if we are at the end
cmp eax, edx
je .alloc

;edi = availability of the block
;esi = size of the block
mov edi, [eax]
mov esi, [eax + 4]
cmp edi, 1
jne .move

cmp esi, ecx
jl .move

;The block is available and of the good size
;The block is of the good size

;make it unavailable
mov dword [eax], 0

;The pointer is past the header
add eax, 8

leave
ret

.move:

;switch to the next block
add eax, esi

jmp .start

.alloc:

mov edi, eax

;alloc new block of the good size
lea ebx, [eax + ecx]
mov eax, 45
int 80h

mov [V_mem_last], ebx

mov dword [edi], 0
mov dword [edi + 4], ecx

;the pointer is past the header
lea eax, [edi + 8]

pop esi
pop edi
pop edx
pop ecx
pop ebx

leave
ret
