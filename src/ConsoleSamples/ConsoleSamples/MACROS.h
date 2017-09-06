
#include <string>

#define __MY_A(V)				#V
#define __MY_W(V)				L##V

#define STRING	std::string
#define WSTRING std::wstring

#if !defined(_UNICODE) && !defined(UNICODE)
#define __MY_T(V)				#V
#define TSTRING std::string
#else
#define TSTRING std::wstring
#define __MY_T(V)				L###V
#endif

#define _tstring TSTRING
#define tstring TSTRING