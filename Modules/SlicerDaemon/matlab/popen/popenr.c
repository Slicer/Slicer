/*
 * popenr.c
 *
 * Attempt to provide popen for matlab, to allow reading from a process.
 * 
 * 2004-09-28 dpwe@ee.columbia.edu  after PlayOn
 * 
 * 2007-01-29 kquintus@bwh.harvard.edu modified
 *
 * ToDo:
 - handle multidimensional reads i.e. 2nd arg = [2 1000]
 - at EOF, return matrix sized according to what was actually read
 *
 * $Header: /homes/dpwe/src/SoundMex5/RCS/PlaySound.c,v 1.1 1997/06/12 00:53:28 dpwe Exp dpwe $
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

/*kquintus: TODO: for UINT16, INT32, and UINT32 discrimination between 
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
    mexPrintf("popenr     Y=popenr(X[,N[,F]])  Open and read an external process\n");
    mexPrintf("           When X is a string, that string is executed as a new process\n");
    mexPrintf("           and Y is returned as a handle (integer) to that stream.\n");
    mexPrintf("           Subsequent calls to popenr(Y,N) with that handle return\n");
    mexPrintf("           the next N values read from the standard ouptut of\n");
    mexPrintf("           converted to Matlab values according to the format\n");
    mexPrintf("           string F (default: char).  A call with N set to -1\n");
    mexPrintf("           means to close the stream.\n");
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
      f = popen(cmd, "r");
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
    mexErrMsgTxt("apparently accessing handle, but no N argument");
    return;
  }
    
  /* get the handle */
  {
    int ix, rows, cols, npts, ngot; 
    int fmt = MXPO_INT16N;
    int sz = 2;
    FILE *f = NULL;
    double *pd;
    mxArray *rslt;

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
    /* how many items required? */
    if (mxGetN(prhs[1]) == 0) {
      mexErrMsgTxt("length argument is empty");
      return;
    } else if (mxGetN(prhs[1]) == 1) {
      rows = (int)*mxGetPr(prhs[1]);
      cols = 1;
    } else {
      double *pd = mxGetPr(prhs[1]);
      rows = (int)pd[0];
      cols = (int)pd[1];
    }

    /* maybe close */
    if (rows < 0) {
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
        
    /* do the read */
    rslt = mxCreateDoubleMatrix(rows, cols, mxREAL); 
    npts = rows*cols;
    ngot = fread(mxGetPr(rslt), sz, npts, f);
    
    /* format conversion */
   
    if (fmt == MXPO_CHAR) {          
      int i;
      double *pd = mxGetPr(rslt);
      signed char *pc = (signed char *)pd;
      pd = pd + npts - 1;
      pc = pc + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pd-- = (double)*pc--;
      }
    }
    else if (fmt == MXPO_UINT8) {          
      int i;
      double *pd = mxGetPr(rslt);
      unsigned char *pc = (unsigned char *)pd;
      pd = pd + npts - 1;
      pc = pc + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pd-- = (double)*pc--;
      }
    }
    else if (fmt == MXPO_INT16N) {
      int i;
      double *pd = mxGetPr(rslt);
      short *ps = (short *)pd;
      pd = pd + npts - 1;
      ps = ps + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pd-- = (double)*ps--;
      }
    }
    else if (fmt == MXPO_INT16R) {
      int i;
      double *pd = mxGetPr(rslt);
      short *ps = (short *)pd;
      pd = pd + npts - 1;
      ps = ps + npts - 1;
      short v;
      for(i = npts-1; i >= 0; --i) {
        v = (short)*pd--;
        *ps -- = (0xFF & (v >> 8)) + (0xFF00 & (v << 8));
      }
    }
    else if (fmt == MXPO_UINT16) {          
      int i;
      double *pd = mxGetPr(rslt);
      unsigned short *pus = (unsigned short *)pd;
      pd = pd + npts - 1;
      pus = pus + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pd-- = (double)*pus--;
      }
    }
    else if (fmt == MXPO_INT32) {
      int i;
      double *pd = mxGetPr(rslt);
      int *pi = (int *)pd;
      pd = pd + npts - 1;
      pi = pi + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pd-- = (double)*pi--;
      }
    }
    else if (fmt == MXPO_UINT32) {
      int i;
      double *pd = mxGetPr(rslt);
      unsigned int *pui = (unsigned int *)pd;
      pd = pd + npts - 1;
      pui = pui + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pd-- = (double)*pui--;
      }
    }
    else if (fmt == MXPO_INT64) {
      int i;
      double *pd = mxGetPr(rslt);
      long long int *pi = (long long int *)pd;
      pd = pd + npts - 1;
      pi = pi + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pd-- = (double)*pi--;
      }
    }
    else if (fmt == MXPO_UINT64) {
      int i;
      double *pd = mxGetPr(rslt);
      unsigned long long int *pui = (unsigned long long int *)pd;
      pd = pd + npts - 1;
      pui = pui + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pd-- = (double)*pui--;
      }
    }
    else if (fmt == MXPO_FLOAT) {
      int i; 
      double *pd = mxGetPr(rslt);
      float *pf = (float *)pd;
      pd = pd + npts - 1;
      pf = pf + npts - 1;
      for(i = npts-1; i >= 0; --i) {
        *pd-- = (double)*pf--;
      }          
    }
    else if (fmt == MXPO_DOUBLE) {
      /* for fmt == MXPO_DOUBLE no conversion needed */  
    }
    else {
      mexErrMsgTxt("couldn't handle datatype");
    }
    
    
    /* did we get all the points we asked for */
    if (ngot < npts) {
      /*kquintus: added error msg in next line: in this case we don't want to
        continue*/
      mexErrMsgTxt("Error *** We read less data than we have expected!\n One possible reason: Slicerdaemon is not running.");
      /* allocate a smaller array and copy to that */
      /* but only chop down by whole columns */
      int gotcols,gotrows;
      int i;
      double *pd, *ps;
      mxArray *newarr;
      if (cols == 1) {
        gotrows = ngot;
        gotcols = 1;
      } else {
        gotrows = rows;
        gotcols = (ngot+rows-1)/rows;
      }
      newarr = mxCreateDoubleMatrix(gotrows, gotcols, mxREAL);
      pd = mxGetPr(newarr);
      ps = mxGetPr(rslt);
      for (i = 0; i < ngot; ++i) {
        *pd++ = *ps++;
      }
      for (i = ngot; i < gotrows*gotcols; ++i) {
        *pd++ = 0;
      }
      mxDestroyArray(rslt);
      rslt = newarr;
    }

    if (nlhs > 0) {
      plhs[0] = rslt;
    } else {
      mxDestroyArray(rslt);
    }
    return;
  }
}
