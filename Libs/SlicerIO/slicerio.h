/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/


#ifndef _SLICERIO_H
#define _SLICERIO_H


#include <stdio.h>
#include "vtkTcl.h"


#ifdef __cplusplus
extern "C" {
#endif

enum stypes {SLICER_NONE = 1, SLICER_FILE, SLICER_STDIN, SLICER_STDOUT, SLICER_STDERR};

typedef struct {
    enum stypes type;
    Tcl_Interp *interp;
    union {
        FILE *file;
        Tcl_Channel channel;
    } option;
} slicerStream;



/* These functions have been defined in stdio.h. 
   We un-define them first. 
 */
#undef fopen
#undef fclose
#undef fread
#undef fwrite
#undef fgets
#undef fprintf
#undef fflush
#undef fgetc
#undef fputc
#undef feof
#undef fseek
#undef ftell
#undef ungetc 


/* Re-define the functions. */
#define fopen slicerFopen
#define fclose slicerFclose
#define fread slicerFread
#define fwrite slicerFwrite
#define fgets slicerFgets
#define fprintf slicerFprintf
#define fflush slicerFflush
#define fgetc slicerFgetc
#define fputc slicerFputc
#define feof slicerFeof
#define fseek slicerFseek
#define ftell slicerFtell
#define ungetc slicerUngetc


/* Our functions */
slicerStream *slicerFopen (const char * filename, const char *mode); 
int     slicerFclose (slicerStream * stream); 
int     slicerFread (void * buffer, size_t size, size_t count, slicerStream *stream);
size_t  slicerFwrite (const void *buffer, size_t size, size_t count, slicerStream *stream);
char   *slicerFgets(char *str, int num, slicerStream *stream);
int slicerFprintf(slicerStream *stream, const char *format, ...);
int slicerFflush(slicerStream *stream);
int slicerFgetc(slicerStream *stream);
int slicerFputc(int ch, slicerStream *stream);
int slicerFeof(slicerStream *stream);
int slicerFseek(slicerStream *stream, long offset, int whence);
int slicerFtell(slicerStream *stream);
int slicerUngetc(int c, slicerStream *stream);


#undef FILE
#define FILE slicerStream

#undef stdin
#undef stdout
#undef stderr
/* Standard input stream.  */
extern slicerStream sstdin;
/* Standard output stream.  */
extern slicerStream sstdout;
/* Standard error output stream.  */
extern slicerStream sstderr;
#define stdin  (&sstdin)
#define stdout (&sstdout)
#define stderr (&sstderr)


#ifdef __cplusplus
}
#endif


#endif /* _SLICERIO_H */


