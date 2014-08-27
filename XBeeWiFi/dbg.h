//#define DEBUG

#ifdef DEBUG 
#define DBG(...) Serial1.print(__VA_ARGS__) 
#else 
#define DBG(...) 
#endif 
