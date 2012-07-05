_F8durationAIAI:
push rbp
mov rbp, rsp
;start timestamp
mov rsi, [rbp + 24]
;end timestamp
mov rdi, [rbp + 16]
;print the high order bytes
mov rax, [rsi - 16]
mov rbx, [rdi - 16]
sub rax, rbx
;if the first diff is 0, do not print 0
cmp rax, 0
jz .second
;if it's negative, we print the positive only
cmp rax, 0
jge .push_first
neg rax
.push_first:
mov r14, rax
call _F5printI
;print the low order bytes
.second:
mov rax, [rsi - 8]
mov rbx, [rdi - 8]
sub rax, rbx
;If it's negative, we print the positive only 
cmp rax, 0
jge .push_second
neg rax
.push_second:
mov r14, rax
call _F5printI
leave
ret
