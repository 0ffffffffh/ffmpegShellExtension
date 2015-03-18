
#if defined (__x86_64__) || defined (__x86_64) || defined (__amd64) || defined (__amd64__) || defined (_AMD64_) || defined (_M_X64)
#define ARCH_X64
#elif defined(__i386__) || defined (i386) || defined(_M_IX86) || defined(_X86_) || defined(__THW_INTEL)
#define ARCH_X86
#else
#error "Not supported CPU Arch."
#endif
