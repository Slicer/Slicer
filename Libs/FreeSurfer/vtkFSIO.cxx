/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFSIO.cxx,v $
  Date:      $Date: 2006/08/08 19:37:40 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
#include "vtkFSIO.h"
#include "vtkByteSwap.h"

int vtkFSIO::ReadShort (FILE* iFile, short& oShort) {

  short s = 0;
  int result ;

  // Read an short. Swap if we need to. Return the value.
  result = fread (&s, sizeof(short), 1, iFile);
  vtkByteSwap::Swap2BE (&s);
  oShort = s;

  return result;
}

int vtkFSIO::ReadShortZ (gzFile iFile, short& oShort) {

  short s = 0;
  int result ;

  // Read an short. Swap if we need to. Return the value.
  result = gzread (iFile, &s, sizeof(short));
  vtkByteSwap::Swap2BE (&s);
  oShort = s;

  return result;
}

int vtkFSIO::ReadInt (FILE* iFile, int& oInt) {

  int i = 0;
  int result ;

  // Read an int. Swap if we need to. Return the value.
  result = fread (&i, sizeof(int), 1, iFile);
  vtkByteSwap::Swap4BE (&i);
  oInt = i;

  return result;
}

int vtkFSIO::ReadIntZ (gzFile iFile, int& oInt) {
  int i = 0;
  int result;

  result = gzread(iFile, &i, sizeof(int));
  vtkByteSwap::Swap4BE(&i);
  oInt = i;
  return result;
}

int vtkFSIO::ReadInt3 (FILE* iFile, int& oInt) {

  int i = 0;
  int result ;

  // Read three bytes. Swap if we need to. Stuff into a full sized int
  // and return.
  result = fread (&i, 3, 1, iFile);
  vtkByteSwap::Swap4BE (&i);
  oInt = ((i>>8) & 0xffffff);

  return result;
}

int vtkFSIO::ReadInt3Z (gzFile iFile, int& oInt) {

  int i = 0;
  int result ;

  // Read three bytes. Swap if we need to. Stuff into a full sized int
  // and return.
  result = gzread (iFile, &i, 3*sizeof(int));
  vtkByteSwap::Swap4BE (&i);
  oInt = ((i>>8) & 0xffffff);

  return result;
}


int vtkFSIO::ReadInt2 (FILE* iFile, int& oInt) {

  int i = 0;
  int result ;

  // Read two bytes. Swap if we need to. Return the value
  result = fread (&i, 2, 1, iFile);
  vtkByteSwap::Swap4BE (&i);
  oInt = i;
  
  return result;
}

int vtkFSIO::ReadInt2Z (gzFile iFile, int& oInt) {

  int i = 0;
  int result ;

  // Read two bytes. Swap if we need to. Return the value
  result = gzread (iFile, &i, 2*sizeof(int));
  vtkByteSwap::Swap4BE (&i);
  oInt = i;
  
  return result;
}

int vtkFSIO::ReadFloat (FILE* iFile, float& oFloat) {

  float f = 0;
  int result ;

  // Read a float. Swap if we need to. Return the value
  result = fread (&f, 4, 1, iFile);
  vtkByteSwap::Swap4BE (&f);
  oFloat = f;
  
  return result;
}

int vtkFSIO::ReadFloatZ (gzFile iFile, float& oFloat) {

  float f = 0;
  int result ;

  // Read a float. Swap if we need to. Return the value
  result = gzread (iFile, &f, sizeof(float));
  vtkByteSwap::Swap4BE (&f);
  oFloat = f;
  
  return result;
}

//
// Utility methods for writing test files
//
int vtkFSIO::WriteInt (FILE* iFile, int iInt)
{
    int i = iInt;
    int result;

    // swap if we need to, write an int, return the result
    vtkByteSwap::Swap4BE(&i);
    result = fwrite(&i, sizeof(int), 1, iFile);

    return result;
}

int vtkFSIO::WriteInt3 (FILE* oFile, int iInt)
{
    // swap if we need to, then write three bytes
    int i = iInt;
    int result;

    i = ((i>>8) & 0xffffff);
    vtkByteSwap::Swap4BE(&i);
    
    result = fwrite(&i, 3, 1, oFile);
    return result;
}

int vtkFSIO::WriteInt2 (FILE* oFile, int iInt)
{

    int i;
    int result;

    // swap if we need to, write two bytes, return the result
    i = iInt;
    vtkByteSwap::Swap4BE(&i);
    result = fread(&i, 2, 1, oFile);

    return result;
}
