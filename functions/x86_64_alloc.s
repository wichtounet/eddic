_F5allocI:
push rbp
mov rbp, rsp

push rbx
push rcx
push rdi
push rsi

mov rbx, [Veddi_remaining]
cmp r14, rbx
jle .alloc_normal
;Get the current address
;syscall 12 = sys_brk
mov rax, 12
;get end
xor rdi, rdi
syscall
;%eax is the current address 
mov rsi, rax
;Alloc new block of 16384K from the current address
mov rdi, rax
;rdi = first parameter
add rdi, 16384
;syscall 12 = sys_brk
mov rax, 12
syscall
;zero'd the new block
;edi = start of block
mov rdi, rax
;edi points to the last DWORD available to us
sub rdi, 4
;this many DWORDs were allocated
mov rcx, 4096
;will write with zeroes
xor rax, rax
;walk backwards
std
;write all over the reserved area
rep stosb
;bring back the DF flag to normal state
cld
mov rax, rsi
mov dword [Veddi_remaining], 16384
mov [Veddi_current], rsi
.alloc_normal:
mov rax, [Veddi_current]
mov rbx, [Veddi_current]
add rbx, r14
mov [Veddi_current], rbx
mov rbx, [Veddi_remaining]
sub rbx, r14
mov [Veddi_remaining], rbx
.alloc_end:

pop rsi
pop rdi
pop rcx
pop rbx

leave
ret
