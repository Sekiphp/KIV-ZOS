#define DEBUG 1

#if defined(DEBUG) && DEBUG > 0
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif