// .NAME ModuleDescriptionParserWin32Header - manage Windows system differences
// .SECTION Description
// The ModuleDescriptionParserWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __ModuleDescriptionParserWin32Header_h
#define __ModuleDescriptionParserWin32Header_h

//#include <ModuleDescriptionParserConfigure.h>

#if defined(WIN32) && !defined(ModuleDescriptionParser_STATIC)
#if defined(ModuleDescriptionParser_EXPORTS)
#define ModuleDescriptionParser_EXPORT __declspec( dllexport ) 
#else
#define ModuleDescriptionParser_EXPORT __declspec( dllimport ) 
#endif
#else
#define ModuleDescriptionParser_EXPORT 
#endif

#endif
