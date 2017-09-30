
#include <string>

#define __MY_A(V)				#V
#define __MY_W(V)				L##V

#define STRING	std::string
#define WSTRING std::wstring

#if !defined(_UNICODE) && !defined(UNICODE)
#define TSTRING std::string
#define __MY_T(V)				#V
#define __MY__TEXT(quote)		quote
#else
#define TSTRING std::wstring
#define __MY_T(V)				L###V
#define __MY__TEXT(quote)		L##quote
#endif

#define _MY_TEXT(x)	__MY__TEXT(x)
#define	_MY_T(x)    __MY__TEXT(x)
#define _tstring TSTRING
#define tstring TSTRING