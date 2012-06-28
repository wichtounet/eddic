_F4timeAI:
push rbp
mov rbp, rsp
xor rax, rax
;serialize instruction stream
cpuid
;rdx:rax = timestamp
rdtsc
mov rsi, [rbp + 16]
mov [rsi - 4], eax
mov [rsi - 8], edx
leave
ret
