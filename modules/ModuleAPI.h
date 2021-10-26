#ifndef MODULE_API_H
#define MODULE_API_H

#ifdef C2E_MODULE_NO
	#define C2E_MODULE_API
#else
	#ifdef _MSC_VER
		// VC++
		#ifdef C2E_MODULE
			// Module
			#define C2E_MODULE_API __declspec(dllimport)
		#else
			// Main program
			#define C2E_MODULE_API __declspec(dllexport)
		#endif
	#else
		// Other compilers - so far none need anything special
		#define C2E_MODULE_API
	#endif
#endif

#endif

