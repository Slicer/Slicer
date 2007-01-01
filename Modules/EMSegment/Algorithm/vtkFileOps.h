/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFileOps.h,v $
  Date:      $Date: 2006/12/08 23:28:24 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
#ifndef __vtkFileOps_h
#define __vtkFileOps_h

#include "vtkObject.h"
#include "vtkEMSegment.h"
    
class VTK_EMSEGMENT_EXPORT vtkFileOps { //; prevent man page generation
  public:
  static vtkFileOps *New() {return (new vtkFileOps);}

  // Note if varname = NULL it will just write the data in the file
  void WriteVectorMatlabFile (const char *filename, const char *name,unsigned char *vec, int xMax) const;
  void WriteVectorMatlabFile (const char *filename, const char *varname,float *vec, int xMax) const;
  void WriteVectorMatlabFile (const char *filename, const char *varname,double *vec, int xMax) const;
  void WriteMatrixMatlabFile (const char *filename, const char *varname, double **mat, int imgY, int imgX) const;

  // ----------------------------------------------
  // Kilian: Old Stuff - I think you can take all of this out  
  // GE Format
  int  WriteMRIfile(char *fname, unsigned char *header, int headersize, short *data, int npixels);
  // Transforms a double vec (with values between min and max) into an unsigned short file where min is 0 and max is MaxOutput
  // int XSize, int YSize are only important if FlagUpsideDown is set 
  void WriteDoubleToUShortToGEFile(const char* FileName, double* vec, int XSize, int YSize, int XYSize , double min, double max, unsigned short MaxOutput,  bool FlagUpsideDown) const; 
  // Does not work correclty
  int  WriteMRIfile(char *fname, double *data, int np);
  //  End of old stuff
  // ----------------------------------------------

  // -------------------------------
  // Write MRI Functions from fileops.c
  // -------------------------------
  static int makeDirectoryIfNeeded(char *fname);
  static char* pathComponent(char *fname);

protected:
  void WriteVectorMatlabFile (FILE *f,const char *name, double *vec, int xMax) const;
  void WriteMatrixMatlabFile (FILE *f,const char *name, double **mat, int imgY, int imgX) const;

  void ensureGEByteOrderForShort(short *data, int np);
  int IsMSBFirstForShort(void);
  short convertShortFromGE(short ge);
  int fileIsCompressed(const char *fname, char **newFileName);
  int uncompressedFileName(const char *fname, char **newFileName);
};

//BTX

template <class T>  
static T MaxValue(T *vec, int size) {
  T result = *vec;
  for (int i = 1; i < size; i++) {
    if (result < *vec ) result = *vec;
    vec++;
  }
  return result;
}

template <class Tin, class Tout> 
static void TransferDataFormat(Tin *vec_in, Tout *vec_out, int Size, Tout max_out) {
  Tin max_in = MaxValue(vec_in, Size);
  Tin quote  = Tin(max_out) / (max_in != 0 ? max_in : 1.0);
  for (int i = 0; i < Size; i++) {
    vec_out[i] = Tout((vec_in[i])*quote);
  }
}
 

template <class T> 
void FlipXAxis(T *invec, T *outvec, int XSize, int YSize, int XYSize) {
  // Flip around the X-Axis
  invec += XYSize;
  for (int y = 0; y < YSize; y++) {
    invec -= XSize;
    memcpy(outvec,invec,sizeof(T)*XSize);
    outvec += XSize;
  }
}

// Kilian - Fixed function in August 03 - works now correctly 
// Flip around the Y Axis
// vec += XSize;
// for (y = 0; y < YSize; y++) {
//  if (y) vec += 2*XSize;
//  for (x = 0; x < XSize; x++) *res++ = *vec--;
// }
// res -= XYSize;

template <class T> 
static void vtkFileOps_WriteToFlippedGEFile(const char *filename,T *vec, int XSize, int YSize, int XYSize) {
  T* res = new T[XYSize];
  FlipXAxis(vec,res,XSize,YSize,XYSize);
  WriteToGEFile(filename,res,XYSize);
  delete[] res;
}

// Opens up a new file and writes down result in the file
template <class T> 
static int WriteToGEFile(const char *filename,T *vec, int size) {
  int appendFlag = 0;
  // If you enter - as name => prints it on the screen
  FILE *f = (strcmp(filename,"-")) ? fopen(filename,((appendFlag)?"ab":"wb")):stdout;
  if ( f == NULL ) {
      //cerr << "Could not open file " << filename << "\n";
      fprintf(stderr, "Could not open file %s\n", filename);
    return 0;
  }
  // Cannot individually do fwrite for each element - > makes a double out of it 
  fwrite(vec, sizeof(T), size, f);
  fflush(f);
  fclose(f);
  return 1;
}
//ETX
#endif
