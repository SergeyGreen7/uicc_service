#ifndef _PRINTER_H_
#define _PRINTER_H_

#ifndef PRINT_LEVEL
#define PRINT_LEVEL 3
#endif

#define SILENCE_LEVEL 0
#define CRITIC_LEVEL 1
#define ERROR_LEVEL 2
#define WARNING_LEVEL 3
#define INFO_LEVEL 4
#define DEBUG_LEVEL 5

#if PRINT_LEVEL >= DEBUG_LEVEL
#define eprint(...)     printf(__VA_ARGS__)
#define wprint(...)     printf(__VA_ARGS__)
#define iprint(...)     printf(__VA_ARGS__)
#define dprint(...)     printf(__VA_ARGS__)
#elif PRINT_LEVEL >= INFO_LEVEL
#define eprint(...)     printf(__VA_ARGS__)
#define wprint(...)     printf(__VA_ARGS__)
#define iprint(...)     printf(__VA_ARGS__)
#define dprint(...)     do{ } while(0)
#elif PRINT_LEVEL >= WARNING_LEVEL
#define eprint(...)     printf(__VA_ARGS__)
#define wprint(...)     printf(__VA_ARGS__)
#define iprint(...)     do{ } while(0)
#define dprint(...)     do{ } while(0)
#elif PRINT_LEVEL >= ERROR_LEVEL
#define eprint(...)     printf(__VA_ARGS__)
#define wprint(...)     do{ } while(0)
#define iprint(...)     do{ } while(0)
#define dprint(...)     do{ } while(0)
#else
#define eprint(...)     do{ } while(0)
#define wprint(...)     do{ } while(0)
#define iprint(...)     do{ } while(0)
#define dprint(...)     do{ } while(0)
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

// void uint16_to_

#endif // _PRINTER_H_