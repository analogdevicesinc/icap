
#ifndef _ICAP_COMPILER_H_
#define _ICAP_COMPILER_H_

#if defined(__CCESVERSION__)
/* Cross Code Embeded Studio project */
#define ICAP_PACKED_BEGIN _Pragma("pack(1)")
#define ICAP_PACKED_END _Pragma("pack()")

#else
#define ICAP_PACKED_BEGIN
#define ICAP_PACKED_END __attribute__((packed))
#endif


#endif /* _ICAP_COMPILER_H_ */
