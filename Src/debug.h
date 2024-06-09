#ifndef __DEBUG__
#define __DEBUG__

#ifdef DEBUG
#define debug(x) (std::cerr << (x))
#else
#define debug(x) ((void)0)
#endif
#endif


