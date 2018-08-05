// dlpGesture.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "dlpGesture.h"


// This is an example of an exported variable
DLPGESTURE_API int ndlpGesture=0;

// This is an example of an exported function.
DLPGESTURE_API int fndlpGesture(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see dlpGesture.h for the class definition
CdlpGesture::CdlpGesture()
{
	return;
}
