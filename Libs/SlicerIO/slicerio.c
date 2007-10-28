/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/


#include "slicerio.h"
#include "utilities.h"

#include <stdlib.h>
#include  <errno.h>
#include <stdarg.h>
#include <string.h>


#define ERR_STRLEN 128


#undef stdin
#undef stdout
#undef stderr

/* For SunOS */
#if defined(__unix__) && defined(__sun__)
#if defined(__STDC__)
extern __FILE   __iob[_NFILE];
#define stdin   (&__iob[0])
#define stdout  (&__iob[1])
#define stderr  (&__iob[2])
#else
extern __FILE    _iob[_NFILE];
#define    stdin    (&_iob[0])
#define    stdout    (&_iob[1])
#define    stderr    (&_iob[2])
#endif    /* __STDC__ */
#endif

/* For linux */
#if defined(__linux__)
#define stdin stdin
#define stdout stdout
#define stderr stderr
#endif

#ifdef __APPLE_CC__
# include <AvailabilityMacros.h>
# ifdef MAC_OS_X_VERSION_10_5 /* Not defined for pre-leopard versions */
#define stdin __stdinp
#define stdout __stdoutp
#define stderr __stderrp
# else
#define stdin   (&__sF[0])
#define stdout  (&__sF[1])
#define stderr  (&__sF[2])
#endif
#endif

/* For Windows */
#if defined(__WIN32__)
#if (_MSC_VER >= 1400)  /* Visual C++ 2005 */
#define stdin  (&__iob_func()[0])
#define stdout (&__iob_func()[1])
#define stderr (&__iob_func()[2])
#else  /* Visual C++ 2003 */
#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])
#endif
#endif


#ifdef FILE 
#undef FILE 
#endif

#undef fprintf


/* Standard input stream.  */
slicerStream sstdin = {SLICER_STDIN, NULL, NULL};
/* Standard output stream.  */
slicerStream sstdout = {SLICER_STDOUT, NULL, NULL};
/* Standard error output stream.  */
slicerStream sstderr = {SLICER_STDERR, NULL, NULL};


#ifdef fopen
#undef fopen
#endif
slicerStream *slicerFopen(const char *filename, const char *mode)
{
    char me[]="slicerFopen", err[ERR_STRLEN];
    slicerStream *stream;
    FILE *file;

    if (!(filename && mode)) {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return NULL;
    }

    stream = (slicerStream *) malloc(sizeof(slicerStream));
    if (!stream) {
        sprintf(err, "%s: couldn't alloc I/O struct", me);
        fprintf(stderr, "%s\n", err);
        return NULL;
    }

    if (filename && *filename == '|') /* Here is a socket io */
    {
        int argc, code;
        const char **argv;
        int flags;
        Tcl_Interp *interp;
        Tcl_Channel channel;

 
        interp = Tcl_CreateInterp();
        code = Tcl_SplitList(interp, filename+1, &argc, &argv);
        if (code != TCL_OK) 
        {
            sprintf(err, "%s: couldn't split(\"%s\"): %s", 
                    me, filename, interp->result);
            fprintf(stderr, "%s\n", err);
            free(stream);
            return NULL;
        }


        /* fopen takes the following modes:
           ``r'' 
           ``r+'' 
           ``w'' 
           ``w+'' 
           ``a'' 
           ``a+'' 

           Tcl_OpenCommandChannel uses these flags:
           TCL_STDIN            (1<<1)  
           TCL_STDOUT           (1<<2)
           TCL_STDERR           (1<<3)
           TCL_ENFORCE_MODE     (1<<4)

           For now, if we see "r", use TCL_STDOUT
                    if we see "w", use TCL_STDIN
                    if we see other modes, report an error.
        */
        if (strcmp(mode, "r") == 0)
        {
            flags = TCL_STDOUT;
        }
        else if (strcmp(mode, "w") == 0)
        {
            flags = TCL_STDIN;
        }
        else
        {
            sprintf(err, "%s: wrong mode for Tcl_OpenCommandChannel: %s", 
                    me, mode);
            fprintf(stderr, "%s\n", err);
            free(stream);
            return NULL;
        }
 
        channel = Tcl_OpenCommandChannel(interp, argc, argv, flags);
        if (channel == NULL) 
        {
            sprintf(err, "%s: couldn't open \"%s\": %s", 
                    me, filename, interp->result);
            fprintf(stderr, "%s\n", err);
            free(stream);
            return NULL;
        }
        Tcl_Free((char *) argv);


        stream->interp = interp;
        stream->type = SLICER_NONE;
        stream->option.channel = channel;

        return stream;
    } 
    else   /* regular file io */
    {
        if (!(file = fopen(filename, mode)))
        { 
            sprintf(err, "%s: fopen(\"%s\",\"%s\") failed", 
                    me, filename, mode);
            fprintf(stderr, "%s\n", err);
            return NULL;
        }

        stream->interp = NULL;
        stream->type = SLICER_FILE;
        stream->option.file = file;

        return stream;
    }
}
#define fopen slicerFopen


#ifdef fclose
#undef fclose
#endif
int slicerFclose(slicerStream *stream) 
{
    char me[]="slicerFclose", err[ERR_STRLEN];
    int ret = 1;
 
    if (! stream) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return ret;
    }

    if (stream->interp == NULL) 
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            ret = fclose((FILE *)stream->option.file);
            free(stream);
            break;
        default:
            ret = 0;
            break;
        } 
    }
    else  /* command pipe */
    {
        ret = Tcl_Close(stream->interp, stream->option.channel);
        if (ret != TCL_OK) 
        {
            sprintf(err, "%s: couldn't close: %s", 
                    me, stream->interp->result);
            fprintf(stderr, "%s\n", err);
        }
        Tcl_DeleteInterp(stream->interp);
        free(stream);
    }

    return ret;
}
#define fclose slicerFclose



#ifdef fread
#undef fread
#endif
int slicerFread(void *buffer, size_t size, size_t count, slicerStream *stream)
{
    char me[]="slicerFread", err[ERR_STRLEN];
    int ret = -1;
 
    if (! (buffer && stream)) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return ret;
    }

    if (stream->interp == NULL)  /* regular file */ 
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            ret = fread(buffer, size, count, (FILE *)stream->option.file);
            break;
        case SLICER_STDIN:   /* stdin */
            ret = fread(buffer, size, count, stdin);
            break;
        default:
            sprintf(err, "%s: cannot fread from stdout or stderr", me);
            fprintf(stderr, "%s\n", err);
            return -1;
            break;
        } 
    }
    else  /* command pipe */
    {
        ret = Tcl_Read(stream->option.channel, buffer, (size * count));
    }

    return ret;
}
#define fread slicerFread



#ifdef fwrite
#undef fwrite
#endif
size_t  slicerFwrite(const void *buffer, size_t size, size_t count, slicerStream *stream)
{
    char me[]="slicerFwrite", err[ERR_STRLEN];
    int ret = -1;
 
    if (! (buffer && stream)) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return ret;
    }

    if (stream->interp == NULL)  /* regular file */ 
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            ret = fwrite(buffer, size, count, (FILE *)stream->option.file);
            break;
        case SLICER_STDOUT:   /* stdout */
            ret = fwrite(buffer, size, count, stdout);
            break;
        case SLICER_STDERR:   /* stderr */
            ret = fwrite(buffer, size, count, stderr);
            break;
        default:
            sprintf(err, "%s: cannot fwrite to stdin", me);
            fprintf(stderr, "%s\n", err);
            ret = -1;
            break;
        } 
    }
    else  /* command pipe */
    {
        ret = Tcl_Write(Tcl_GetStdChannel(TCL_STDOUT), buffer, (size * count));
        //  ret = Tcl_Write(stream->option.channel, buffer, (size * count));
 
    }

    return ret;
}
#define fwrite slicerFwrite



#ifdef fgets
#undef fgets
#endif
char *slicerFgets(char *str, int num, slicerStream *stream)
{
    char me[]="slicerFgets", err[ERR_STRLEN];
    int ret;
    Tcl_DString buffer;

    if (! (str && stream)) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return NULL;
    }


    if (stream->interp == NULL)  /* regular file */ 
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            return (fgets(str, num, (FILE *)stream->option.file));
            break;
        case SLICER_STDIN:   /* stdin */
            return (fgets(str, num, stdin));
            break;
        default:
            sprintf(err, "%s: cannot fgets from stdout or stderr", me);
            fprintf(stderr, "%s\n", err);
            return NULL;
            break;
        } 
    }
    else  /* command pipe */
    {
        Tcl_DStringInit(&buffer);
        ret = Tcl_Gets(stream->option.channel, &buffer);
        strncpy(str, buffer.string, num);
        Tcl_DStringFree(&buffer);

        if (ret >= 0) /* successfully */
        {
            return str;
        }
        else /* failed */
        {
            return NULL;
        }
    }
}
#define fgets slicerFgets



#ifdef fflush
#undef fflush
#endif
int slicerFflush(slicerStream *stream)
{
    char me[]="slicerFflush", err[ERR_STRLEN];
    int retVal = -1;
 
    if (! stream) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return retVal;
    }


    if (stream->interp == NULL)  
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            retVal = fflush((FILE *)stream->option.file);
            break;
        case SLICER_STDOUT:   /* stdout */
            retVal = fflush(stdout);
            break;
        case SLICER_STDERR:   /* stderr */
            retVal = fflush(stderr);
            break;
        default:
            sprintf(err, "%s: fflushing stdin is undefined", me);
            fprintf(stderr, "%s\n", err);
            retVal = -1;
            break;
        } 
    }
    else  /* command pipe */
    {
        retVal = Tcl_Flush(stream->option.channel);
    }

    return retVal;
}
#define fflush slicerFflush



#ifdef fgetc
#undef fgetc
#endif
int slicerFgetc(slicerStream *stream)
{
    char me[]="slicerFgetc", err[ERR_STRLEN];
    char buffer[2];
    int retVal = EOF;
    int num = 0;
 
    if (! stream) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return retVal;
    }


    if (stream->interp == NULL)  
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            retVal = fgetc((FILE *)stream->option.file);
            break;
        case SLICER_STDIN:   /* stdin */
            retVal = fgetc(stdin);
            break;
        default:
            sprintf(err, "%s: can't fgetc from stdout or stderr", me);
            fprintf(stderr, "%s\n", err);
            retVal = EOF;
            break;
        } 
    }
    else  /* command pipe */
    {
        num = Tcl_Read(stream->option.channel, buffer, (sizeof(char) * 1));
        /* num == -1 for error */
        retVal = (num == -1 ? num : buffer[0]);
    }

    return retVal;
}
#define fgetc slicerFgetc



#ifdef fputc
#undef fputc
#endif
int slicerFputc(int ch, slicerStream *stream)
{
    char me[]="slicerFputc", err[ERR_STRLEN];
    char buffer[2];
    int retVal = EOF;
    int num = 0;
 
    if (! stream) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return retVal;
    }

    if (stream->interp == NULL)  
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            retVal = fputc(ch, (FILE *)stream->option.file);
            break;
        case SLICER_STDOUT:   /* stdout */
            retVal = fputc(ch, stdout);
            break;
        case SLICER_STDERR:   /* stderr */
            retVal = fputc(ch, stderr);
            break;
        default:
            sprintf(err, "%s: can't fputc on stdin", me);
            fprintf(stderr, "%s\n", err);
            retVal = EOF;
            break;
        } 
    }
    else  /* command pipe */
    {
        buffer[0] = ch;
        num = Tcl_Write(stream->option.channel, buffer, (1 * sizeof(char)));
 
        /* num == -1 for error */
        retVal = (num <= 0 ? num : ch);
    }

    return retVal;
}
#define fputc slicerFputc



#ifdef feof
#undef feof
#endif
int slicerFeof(slicerStream *stream)
{
    char me[]="slicerFeof", err[ERR_STRLEN];
    int retVal = -1;
 
    if (! stream) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return -1;
    }

    if (stream->interp == NULL)  
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            retVal = feof((FILE *)stream->option.file);
            break;
        case SLICER_STDIN:   /* stdin */
            retVal = feof(stdin);
            break;
        default:
            sprintf(err, "%s: can't feof on stdout or stderr", me);
            fprintf(stderr, "%s\n", err);
            retVal = -1;
            break;
        } 
    }
    else  /* command pipe */
    {
        retVal = Tcl_Eof(stream->option.channel);
    }

    return retVal;
}
#define feof slicerFeof



#ifdef fseek
#undef fseek
#endif
int slicerFseek(slicerStream *stream, long offset, int whence)
{
    char me[]="slicerFseek", err[ERR_STRLEN];
    Tcl_WideInt retVal = -1;
 
    if (! stream) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return -1;
    }

    if (stream->interp == NULL)  
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            retVal = fseek((FILE *)stream->option.file, offset, whence);
            break;
        case SLICER_STDIN:   /* stdin */
            retVal = fseek(stdin, offset, whence);
            break;
        case SLICER_STDOUT:   /* stdout */
            retVal = fseek(stdout, offset, whence);
            break;
        case SLICER_STDERR:   /* stderr */
            retVal = fseek(stderr, offset, whence);
            break;
        default:
            sprintf(err, "%s: can't fseek on an unknown stream", me);
            fprintf(stderr, "%s\n", err);
            retVal = -1;
            break;
        } 
    }
    else  /* command pipe */
    {
        retVal = Tcl_Seek(stream->option.channel, offset, whence);
        if (retVal == -1)
        {
            sprintf(err, "%s: %s", me, Tcl_ErrnoMsg(Tcl_GetErrno()));
            fprintf(stderr, "%s\n", err);
        }
    }

    return (int) retVal;
}
#define fseek slicerFseek



#ifdef ftell
#undef ftell
#endif
int slicerFtell(slicerStream *stream)
{
    char me[]="slicerFtell", err[ERR_STRLEN];
    Tcl_WideInt retVal = -1;
 
    if (! stream) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return -1;
    }

    if (stream->interp == NULL)  
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            retVal = ftell((FILE *)stream->option.file);
            break;
        case SLICER_STDIN:   /* stdin */
            retVal = ftell(stdin);
            break;
        case SLICER_STDOUT:   /* stdout */
            retVal = ftell(stdout);
            break;
        case SLICER_STDERR:   /* stderr */
            retVal = ftell(stderr);
            break;
        default:
            sprintf(err, "%s: can't ftell on an unknown stream", me);
            fprintf(stderr, "%s\n", err);
            retVal = -1;
            break;
        } 
    }
    else  /* command pipe */
    {
        retVal = Tcl_Tell(stream->option.channel);
        if (retVal == -1)
        {
            sprintf(err, "%s: %s", me, Tcl_ErrnoMsg(Tcl_GetErrno()));
            fprintf(stderr, "%s\n", err);
        }
    }

    return (int) retVal;
}
#define ftell slicerFtell



#ifdef ungetc 
#undef ungetc 
#endif
int slicerUngetc(int c, slicerStream *stream)
{
    char me[]="slicerUngetc", err[ERR_STRLEN];
    char buffer[2];
    int retVal = EOF;
 
    if (! stream) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return retVal;
    }

    if (stream->interp == NULL)  
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            retVal = ungetc(c, (FILE *)stream->option.file);
            break;
        case SLICER_STDIN:   /* stdin */
            retVal = ungetc(c, stdin);
            break;
        default:
            sprintf(err, "%s: can't ungetc on stdout or stderr", me);
            fprintf(stderr, "%s\n", err);
            retVal = EOF;
            break;
        } 
    }
    else  /* command pipe */
    {
        buffer[0] = c;
        retVal = Tcl_Ungets(stream->option.channel, buffer, 1, 0);
        if (retVal == -1)
        {
            sprintf(err, "%s: %s", me, Tcl_ErrnoMsg(Tcl_GetErrno()));
            fprintf(stderr, "%s\n", err);
        }
    }

    return retVal;
}
#define ungetc slicerUngetc



int slicerFprintf(slicerStream *stream, const char *format, ...)
{
    char me[]="slicerFprint", err[ERR_STRLEN];
    int retVal = -1;
 
    int size;
    va_list ap;
    char *p;

    if (! (format && stream)) 
    {
        sprintf(err, "%s: got NULL pointer", me);
        fprintf(stderr, "%s\n", err);
        return retVal;
    }


    va_start(ap, format);
    size = vaFormatStringLength(format, ap);

    p = (char *)ckalloc(size*sizeof(char));
    if (p == NULL) 
    {
        sprintf(err, "%s: failed to allocate memory for string", me);
        fprintf(stderr, "%s\n", err);
        return retVal;
    }

    if (stream->interp == NULL)  /* regular file */ 
    {
        switch (stream->type)
        {
        case SLICER_FILE:   /* regular file */
            retVal = vfprintf((FILE *)stream->option.file, format, ap);
            break;
        case SLICER_STDOUT:   /* stdout */
            retVal = vfprintf(stdout, format, ap);
            break;
        case SLICER_STDERR:   /* stderr */
            retVal = vfprintf(stderr, format, ap);
            break;
        default:
            sprintf(err, "%s: cannot fprintf to stdin", me);
            fprintf(stderr, "%s\n", err);
            retVal = 0;
            break;
        } 
    }
    else  /* command pipe */
    {
        vsprintf(p, format, ap);
        retVal = Tcl_Write(stream->option.channel, p, (size * sizeof(char)));
    }

    va_end(ap);
    Tcl_Free(p);
    return retVal;
}
#define fprintf slicerFprintf



#undef FILE
#define FILE slicerStream

#undef stdin
#undef stdout
#undef stderr

#define slicerStdin stdin
#define slicerStdout stdout
#define slicerStderr stderr



