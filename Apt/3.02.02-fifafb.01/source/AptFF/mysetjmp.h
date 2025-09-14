#ifndef _mysetjmp_h_
#define _mysetjmp_h_
typedef struct {
  unsigned long eax;
  unsigned long ebx;
  unsigned long ecx;
  unsigned long edx;
  unsigned long esi;
  unsigned long edi;
  unsigned long ebp;
  unsigned long esp;
  unsigned long eip;
} jmp_buf[1];

#define setjmp(_) (0)
#define longjmp(_,__)

#endif
