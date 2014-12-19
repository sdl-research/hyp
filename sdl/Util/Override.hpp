


































*/







#if defined(_MSC_VER)
#define OVERRIDE override

#elif defined(__clang__)



#if __GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 7







#endif
#endif

#define OVERRIDE override
#else
#define OVERRIDE
#endif
#endif












