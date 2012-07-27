_F5allocI:
push rbp
mov rbp, rsp

push rdi
push r10
push r11
push r12
push r13
push r14

;Add the size of the header
add r14, 16

;r12 = start, r13 = last
mov r12, [V_mem_start]
mov r13, [V_mem_last]

.start:

;Check if we are at the end
cmp r12, r13
je .alloc

;r10 = availability of the block
;r11 = size of the block
mov r10, [r12]
mov r11, [r12 + 8]
cmp r10, 1
jne .move

cmp r11, r14
jl .move

;The block is available and of the good size
;The block is of the good size

;make it unavailable
mov qword [r12], 0

;The pointer is past the header
lea rax, [r12 + 16]

leave
ret

.move:

;switch to the next block
add r12, r11

jmp .start

.alloc:

;alloc new block of the good size
lea rdi, [r12 + r14]
mov rax, 12
syscall

mov [V_mem_last], rdi

mov qword [r12], 0
mov qword [r12 + 8], r14

;the pointer is past the header
lea rax, [r12 + 16]

pop r14
pop r13
pop r12
pop r11
pop r10
pop rdi

leave
ret
