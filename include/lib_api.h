#ifndef __LIB__API___
#define __LIB__API___

#define MAPPER_CORE_EXPORTS 1

#if defined(_WIN32) || defined(_WIN64)
	#if defined(MAPPER_CORE_EXPORTS)
		#define LIBAPI __declspec(dllexport)
	#elif defined(MAPPER_CORE_IMPORTS)
		#define LIBAPI __declspec(dllimport)
	#else
		#define LIBAPI
	#endif
#else
	#if defined(MAPPER_CORE_EXPORTS)
		#define LIBAPI __attribute__ ((visibility ("default")))
	#else
		#define LIBAPI extern
	#endif
#endif
	
#endif
