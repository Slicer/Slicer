/*
 * popenw.c
 *
 * popen with writing to pipes for matlab
 * 
 * 2005-11-11 dpwe@ee.columbia.edu  after popen
 * 2007-01-29 kquintus@bwh.harvard.edu modified
 *
 * $Header: /homes/drspeech/src/matlabpopen/RCS/popenw.c,v 1.2 2007/01/14 04:10:18 dpwe Exp $
 */
 
#include    <stdio.h>
#include    <math.h>
#include    <ctype.h>

#include    "mex.h"

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BYTE_ORDER
#ifdef  __DARWIN_UNIX03
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#endif

/* check if flags are working
   #if BYTE_ORDER == BIG_ENDIAN
   #error "byte order big endian"
   #endif

   #if BYTE_ORDER == LITTLE_ENDIAN
   #error "byte order little endian"
   #endif
*/

/*TODO: for UINT16, INT32, and UINT32 discrimination between 
  little and big endian*/

/*kquintus: added data types uint16, int32, and uint32*/
enum {
  MXPO_CHAR,    
  MXPO_UINT8,
  MXPO_INT16N,
  MXPO_INT16R,
  MXPO_UINT16,
  MXPO_INT32,
  MXPO_UINT32,
  MXPO_INT64,
  MXPO_UINT64,
  MXPO_FLOAT,
  MXPO_DOUBLE,
};

#define FILETABSZ 16
static FILE *filetab[FILETABSZ];
static int filetabix = 0;

int findfreetab() {
  /* find an open slot in the file table */
  int i;
  for (i = 0; i < filetabix; ++i) {
    if ( filetab[i] == NULL ) {
      /* NULL entries are currently unused */
      return i;
    }
  }
  if (filetabix < FILETABSZ) {
    i = filetabix;
    /* initialize it */
    filetab[i] = NULL;
    ++filetabix;
    return i;
  }
  /* out of space */
  return -1;
}

void
mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  int i, err, len;
  long pvl, pvb[16];

  if (nrhs < 1){
    mexPrintf("popenw     Y=popenw(X[,D[,F]])  Open and write an external process\n");
    mexPrintf("           When X is a string, that string is executed as a new process\n");
    mexPrintf("           and Y is returned as a handle (integer) to that stream.\n");
    mexPrintf("           Subsequent calls to popen(Y,D[,F]) with that handle write\n");
    mexPrintf("           the data in D to the the process, converted to binary\n");
    mexPrintf("           according to the format string F (default: big endian int16).\n");
    mexPrintf("           A call with D empty means to close the stream.\n");
    return;
  }
  /* look at the data */
  /* Check to be sure input argument is a string. */
  if ((mxIsChar(prhs[0]))){
    /* first argument is string - opening a new command */
    FILE *f;
    char *cmd;
    int tabix = findfreetab();

    if (tabix < 0) {
      mexErrMsgTxt("Out of file table slots.");
    } else {
      cmd = mxArrayToString(prhs[0]);
      /* fprintf(stderr, "cmd=%s\n", cmd); */
      f = popen(cmd, "w");
      mxFree(cmd);
      if ( f == NULL ) {
        mexErrMsgTxt("Error running external command.");
        return;
      }
      /* else have a new command path - save the handle */
      filetab[tabix] = f;
      /* return the index */
      if (nlhs > 0) {
        double *pd;
        mxArray *rslt = mxCreateDoubleMatrix(1,1, mxREAL);
        plhs[0] = rslt;
        pd = mxGetPr(rslt);
        *pd = (double)tabix;
      }
    }
    return;
  }

  if (nrhs < 2) {
    mexErrMsgTxt("apparently accessing handle, but no D argument");
    return;
  }
    
  /* get the handle */
  {
    int ix, rows, cols, npts, ngot; 
    int fmt = MXPO_INT16N;
    int sz = 2;
    FILE *f = NULL;
    double *pd;
    char *data;

    if (mxGetN(prhs[0]) == 0) {
      mexErrMsgTxt("handle argument is empty");
      return;
    }

    pd = mxGetPr(prhs[0]);
    ix = (int)*pd;
    if (ix < filetabix) {
      f = filetab[ix];
    }
    if (f == NULL) {
      mexErrMsgTxt("invalid handle");
      return;
    }

    /* how many values to write */
    npts = mxGetN(prhs[1]) * mxGetM(prhs[1]); 
    if (npts == 0) {
      /* close */
      pclose(f);
      filetab[ix] = NULL;
      return;
    }

    /* what is the format? */
    if ( nrhs > 2 ) {
      char *fmtstr;

      if (!mxIsChar(prhs[2])) {
        mexErrMsgTxt("format arg must be a string");
        return;
      }
      fmtstr = mxArrayToString(prhs[2]);
      if (strcmp(fmtstr, "int16n")==0 || strcmp(fmtstr, "int16") == 0) {
        fmt = MXPO_INT16N;
      } else if (strcmp(fmtstr, "int16r")==0) {
        fmt = MXPO_INT16R;
      } else if (strcmp(fmtstr, "int16be")==0) {
#if BYTE_ORDER == BIG_ENDIAN
        fmt = MXPO_INT16N;
#else
        fmt = MXPO_INT16R;
#endif
      } else if (strcmp(fmtstr, "int16le")==0) {
#if BYTE_ORDER == BIG_ENDIAN
        fmt = MXPO_INT16R;
#else
        fmt = MXPO_INT16N;
#endif
     
      } else if (strcmp(fmtstr, "uint16")==0) {
        fmt =  MXPO_UINT16;
        sz = 2;
      } else if (strcmp(fmtstr, "int32")==0) {
        fmt = MXPO_INT32;
        sz = 4;
      } else if (strcmp(fmtstr, "uint32")==0) {
        fmt =  MXPO_UINT32;
        sz = 4;
      } else if (strcmp(fmtstr, "int64")==0) {
        fmt = MXPO_INT64;
        sz = 8;
      } else if (strcmp(fmtstr, "uint64")==0) {
        fmt =  MXPO_UINT32;
        sz = 8;
      } else if (strcmp(fmtstr, "float")==0) {
        fmt = MXPO_FLOAT;
        sz = 4;
      } else if (strcmp(fmtstr, "double")==0) {
        fmt = MXPO_DOUBLE;
        sz = 8;
      } else if (strcmp(fmtstr, "uint8")==0) {
        fmt = MXPO_UINT8;
        sz = 1;
      } else if (strcmp(fmtstr, "char")==0) {
        fmt = MXPO_CHAR;
        sz = 1;
      } else {
        mexErrMsgTxt("unrecognized format");
      }
      mxFree(fmtstr);
    }

    data = (char *)malloc(sz*npts+1);

    /* format conversion */
    /* kquintus: reorganized section. Added cases for float, double, uint16, 
       int32, uint32, and uint8 */
    if (fmt == MXPO_CHAR) {
      if (mxIsChar(prhs[1])) {
        if (mxGetString(prhs[1], data, npts+1) != 0) {
          mxErrMsgTxt("Problems copying string data");
        }
      } else {
        int i;
        double *pd = mxGetPr(prhs[1]);
        signed char *pc = (signed char *)data;
        pd = pd + npts - 1;
        pc = pc + npts - 1;
        for(i = npts-1; i >= 0; --i) {
          *pc-- = (signed char)*pd--;
        }
      }
    }
    else if (fmt == MXPO_UINT8) {
      if (mxIsChar(prhs[1])) {
        if (mxGetString(prhs[1], data, npts+1) != 0) {
          mxErrMsgTxt("Problems copying string data");
        }
      } else {
        int i;
        double *pd = mxGetPr(prhs[1]);
        unsigned char *pc = (unsigned char *)data;
        pd = pd + npts - 1;
        pc = pc + npts - 1;
        for(i = npts-1; i >= 0; --i) {
          *pc-- = (unsigned char)*pd--;
        }
      }
    }
    else if (fmt == MXPO_INT16N) {
      int i;
      double *pd = mxGetPr(prhs[1]);
      short *ps = (short *)data;
      pd = pd + npts - 1;
      ps = ps + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *ps-- = (short)*pd--;
      }
    }
    else if (fmt == MXPO_INT16R) {
      int i;
      double *pd = mxGetPr(prhs[1]);
      short *ps = (short *)data;
      short v;
      pd = pd + npts - 1;
      ps = ps + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        v = (short)*pd--;
        *ps -- = (0xFF & (v >> 8)) + (0xFF00 & (v << 8));
      }
    }
    else if (fmt == MXPO_UINT16) {
      int i;
      double *pd = mxGetPr(prhs[1]);
      unsigned short *pus = (unsigned short *)data;
      pd = pd + npts - 1;
      pus = pus + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pus-- = (unsigned short)*pd--;
      }
    }  
    else if (fmt == MXPO_INT32) {
      int i;
      double *pd = mxGetPr(prhs[1]);
      int *p = (int *)data;
      pd = pd + npts - 1;
      p = p + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *p-- = (int)*pd--;
      }
    }  
    else if (fmt == MXPO_UINT32) {
      int i;
      double *pd = mxGetPr(prhs[1]);
      unsigned int *pui = (unsigned int *)data;
      pd = pd + npts - 1;
      pui = pui + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pui-- = (unsigned int)*pd--;
      }
    }  
    else if (fmt == MXPO_INT64) {
      int i;
      double *pd = mxGetPr(prhs[1]);
      long long int *p = (long long int *)data;
      pd = pd + npts - 1;
      p = p + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *p-- = (long long int)*pd--;
      }
    }  
    else if (fmt == MXPO_UINT64) {
      int i;
      double *pd = mxGetPr(prhs[1]);
      unsigned long long int *pui = (unsigned long long int *)data;
      pd = pd + npts - 1;
      pui = pui + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pui-- = (unsigned long long int)*pd--;
      }
    }  
    else if (fmt == MXPO_FLOAT) {
      int i;
      double *pd = mxGetPr(prhs[1]);
      float *pf = (float *)data;
      pd = pd + npts - 1;
      pf = pf + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pf-- = (float)*pd--;
      } 
    } 
    else if (fmt == MXPO_DOUBLE) {
      free(data);
      ngot = fwrite(mxGetPr(prhs[1]), sz, npts, f);
    }
    else {
      mexErrMsgTxt("Couldn't handle datatype");
    }

    if (fmt != MXPO_DOUBLE) {
      /* write the data */
      ngot = fwrite(data, sz, npts, f);
    
      free(data);
    } 
    /* return # bytes written */
    if (nlhs > 0) {
      double *pd;
      mxArray *rslt = mxCreateDoubleMatrix(1,1, mxREAL);
      plhs[0] = rslt;
      pd = mxGetPr(rslt);
      *pd = (double)ngot;
    }

    return;
  }
}

