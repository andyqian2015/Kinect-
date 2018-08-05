// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DLPGESTURE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DLPGESTURE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DLPGESTURE_EXPORTS
#define DLPGESTURE_API __declspec(dllexport)
#else
#define DLPGESTURE_API __declspec(dllimport)
#endif

// This class is exported from the dlpGesture.dll
class DLPGESTURE_API CdlpGesture {
public:
	CdlpGesture(void);
	// TODO: add your methods here.
};

extern DLPGESTURE_API int ndlpGesture;

DLPGESTURE_API int fndlpGesture(void);
