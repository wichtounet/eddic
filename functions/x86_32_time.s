_F4timeAI:
push ebp
mov ebp, esp
xor eax, eax
cpuid
rdtsc
mov esi, [ebp + 8]
mov [esi - 4], eax
mov [esi - 8], edx
leave
ret
