_F4timeAI:
push rbp
mov rbp, rsp
;xor rax, rax
;serialize instruction stream
cpuid
;rdx:rax = timestamp
rdtsc
mov rsi, [rbp + 16]
mov [rsi - 8], rax
mov [rsi - 16], rdx
leave
ret
