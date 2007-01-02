/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFileOps.cxx,v $
  Date:      $Date: 2006/12/12 18:40:30 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/
#include "vtkFileOps.h"

// Just for write Function 
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <sys/stat.h>
#include <vtksys/SystemTools.hxx>

// Opens up a new file and writes down result in the file
void vtkFileOps::WriteVectorMatlabFile (const char *filename, const char *name, unsigned char *vec, int xMax) const {
  int appendFlag = 0;
  FILE *f = (strcmp(filename,"-"))?fopen(filename,((appendFlag)?"a":"w")):stdout;
  if ( f == NULL ) {
    cerr << "Could not open file " << filename << "\n";
    return;
  }
  if (name != NULL) fprintf(f,"%s = [", name);
  xMax --;
  for (int x = 0; x < xMax; x++ )
      fprintf(f,"%d ", vec[x]);
  fprintf(f,"%d", vec[xMax]);
  if (name != NULL) fprintf(f,"];\n");
  fflush(f);
  fclose(f);
}

void vtkFileOps::WriteVectorMatlabFile (const char *filename, const char *name,float *vec, int xMax) const {
  int appendFlag = 0;
  FILE *f = (strcmp(filename,"-"))?fopen(filename,((appendFlag)?"a":"w")):stdout;
  if ( f == NULL ) {
    cerr << "Could not open file " << filename << "\n";
    return;
  }
  if (name != NULL) fprintf(f,"%s = [", name);
  xMax --;
  for (int x = 0; x < xMax; x++ ) fprintf(f,"%10.6f ", vec[x]);
  fprintf(f,"%10.6f", vec[xMax]);
  if (name != NULL) fprintf(f,"];\n");
  fflush(f);
  fclose(f);
}

// Opens up a new file and writes down result in the file
void vtkFileOps::WriteVectorMatlabFile (const char *filename, const char *varname,double *vec, int xMax) const {
  int appendFlag = 0;
  FILE *f = (strcmp(filename,"-"))?fopen(filename,((appendFlag)?"a":"w")):stdout;
  if ( f == NULL ) {
    cerr << "Could not open file " << filename << "\n";
    return;
  }
  this->WriteVectorMatlabFile(f,varname,vec,xMax);
  fflush(f);
  fclose(f);
}

// Writes Vector to file in Matlab format if name is specified otherwise just 
// writes the values in the file
void vtkFileOps::WriteVectorMatlabFile (FILE *f, const char *name, double *vec, int xMax) const {
  if (name != NULL) fprintf(f,"%s = [", name);
  xMax --;
  for (int x = 0; x < xMax; x++ )
      fprintf(f,"%10.6f ", vec[x]);
  fprintf(f,"%10.6f", vec[xMax]);
  if (name != NULL) fprintf(f,"];\n");
}

// Opens up a new file and writes down result in the file
void vtkFileOps::WriteMatrixMatlabFile (const char *filename, const char *varname, double **mat, int imgY, int imgX) const {
  int appendFlag = 0;
  FILE *f = (strcmp(filename,"-"))?fopen(filename,((appendFlag)?"a":"w")):stdout;
  if ( f == NULL ) {
    cerr << "Could not open file " << filename << "\n";
    return;
  }
  this->WriteMatrixMatlabFile(f,varname,mat,imgY,imgX);
  fflush(f);
  fclose(f);
}

// Writes Matrix to file in Matlab format if name is specified otherwise just 
// writes the values in the file
void vtkFileOps::WriteMatrixMatlabFile (FILE *f, const char *name, double **mat, int imgY, int imgX) const
{
  if (name != NULL) fprintf(f,"%s = [", name);
  for (int y = 0; y < imgY; y++ ) {
    this->WriteVectorMatlabFile(f,NULL,mat[y],imgX);
    if (y < (imgY-1)) fprintf(f,";\n");
  }
  if (name != NULL) fprintf(f,"];\n");
  fprintf(f,"\n");
}
// int XSize, int YSize are only important if FlagUpsideDown is set 
void  vtkFileOps::WriteDoubleToUShortToGEFile(const char* FileName, double* vec,int XSize, int YSize, int XYSize, double min, double max, unsigned short MaxOutput, bool FlagUpsideDown) const {
  unsigned short *result = new unsigned short[XYSize];
  if (min < 0 ) min = 0; 
  double calc =  double(MaxOutput)/ (max - min);
  for (int x = 0 ; x < XYSize; x++) {
    if ((*vec) < min) (*result ++) = 0;
    else if ((*vec) > max) (*result ++) = MaxOutput ;
    else (*result ++) = (unsigned short)(calc* (*vec - min));
    vec ++;
  }
  result -= XYSize;
  if (FlagUpsideDown) vtkFileOps_WriteToFlippedGEFile(FileName,result, XSize,YSize, XYSize);
  else WriteToGEFile(FileName,result, XYSize);;
  delete[] result;
}

// ---------------------------------------------------------
// Functions to write Short Data to an MRI File
// ---------------------------------------------------------
/* Copyright (c) Simon Warfield simonw@bwh.harvard.edu */
/* $Id: vtkFileOps.cxx,v 1.2 2006/12/12 18:40:30 yumin Exp $ */
int vtkFileOps::uncompressedFileName(const char *fname, char **newFileName)
{
#ifndef _WIN32
  char *p = (char *)NULL;
  char *lastSlash = (char *)NULL;

    /* If the file name ends in .gz or .Z it is probably compressed */
    p = strrchr(fname, '.');
    lastSlash = strrchr(fname, '/');
    if ( (p != NULL) && ((lastSlash == NULL) || (lastSlash < p))  &&
        ((strcmp(p, ".gz") == 0) || (strcmp(p, ".Z") == 0)) ) {
      (*newFileName) = strdup(fname); /* Use the same name */
      assert((*newFileName) != NULL);
      p = strrchr(*newFileName,'.');
      *p = '\0';
    } else {
      (*newFileName) = strdup(fname); /* Use the same name */
      assert((*newFileName) != NULL);
    }
#endif
  return 0; /* Success */
}

/* Return -1 on error, return 1 if file is compressed, 
        return 0 if it is not compressed
 */
int vtkFileOps::fileIsCompressed(const char *fname, char **newFileName)
{
#ifndef _WIN32
  struct stat statBuf;
  vtkNotUsed(int long fileSize = 0;);

  char *p = (char *)NULL;
  char *lastSlash = (char *)NULL;
  int fileIsCompressed = 0;

  char *dontcarename = NULL;

  /* In case we don't want to name the uncompressed name */
  if (newFileName == NULL) {
    newFileName = &dontcarename;
  }

  if (stat(fname, &statBuf) == -1) {
    /* The given file name is wrong.  Lets look for a compressed version */
    if ( (*newFileName) == NULL) {
      (*newFileName) = (char *)malloc(PATH_MAX);
      assert((*newFileName) != NULL);
    }
    (*newFileName)[0] = '\0';
    strcat((*newFileName),fname);
    strcat((*newFileName),".gz");
    if (stat((*newFileName), &statBuf) == -1) {
      (*newFileName)[0] = '\0';
      strcat((*newFileName),fname);
      strcat((*newFileName),".Z");
      if (stat((*newFileName), &statBuf) == -1) {
        free((*newFileName));
        (*newFileName) = NULL;
        /* File does not exist, and no compressed version does */
        return -1; /* error - no such file */
      }
    }
    /* File does not exist, but a compressed version does */
    fileIsCompressed = 1;
  } else {
    /* File does exist : Is it compressed ? */
    /* If the file name ends in .gz or .Z it is probably compressed */
    p = strrchr(fname, '.');
    lastSlash = strrchr(fname, '/');
    if ( (p != NULL) && ((lastSlash == NULL) || (lastSlash < p))  &&
        ((strcmp(p, ".gz") == 0) || (strcmp(p, ".Z") == 0)) ) {
      fileIsCompressed = 1;
    }
    if ( (*newFileName) != NULL) {
      free(*newFileName);
    }
    (*newFileName) = strdup(fname); /* Use the same name */
    assert((*newFileName) != NULL);
  }
  if (dontcarename != NULL) {
    free(dontcarename);
  }
  return fileIsCompressed;
#endif
  return 0;
}

char* vtkFileOps::pathComponent(char *fname)
{
  int j = 0;
  int len = 0;
  char *tmp = strdup(fname);
  assert(tmp != NULL);

  len = int(strlen(tmp));

  for (j = (len-1); j >= 0; j--) {
    if (tmp[j] != '/') {
       tmp[j] = '\0';
    } else {
       tmp[j] = '\0';
       break;
    }
  }
  /* Use a NULL pointer to indicate no / was in the string */
  if (strlen(tmp) == 0) return NULL;
  return tmp;
}

/* Given a file name, find the directory this file would go in,
 * and make the directory if necessary
 *
 * There is a race condition if multiple threads call makeDirectoryIfNeeded
 * at the same time, since some may find the directory is not present,
 * and then find it has been created by the time they try to create it.
 *   I hope the EEXISTS test will catch any problems.
 */
int vtkFileOps::makeDirectoryIfNeeded(char *fname)
{
  struct stat statBuf;
  int retval = -1;
  int status = 0;
  // Kilian : change again
  char *path = vtkFileOps::pathComponent(fname);
  if (fname == NULL) {
    /* It isn't possible to make a directory for a NULL file name */
    return -1;
  }
  if (path == NULL) {
    /* The filename is going into the current directory, which is always OK */
    return 0;
  }

  /* If it doesn't exist, make it */
  if (stat(path, &statBuf) != 0) {
    /* fprintf(stdout,"stat failed - '%s' doesn't exist so need to make it\n",
    path); */
    if (vtkFileOps::makeDirectoryIfNeeded(path) != 0) {
      free(path);
      return -1; /* Some horrible failure */
    }
#ifdef _WIN32    
    status = !vtksys::SystemTools::MakeDirectory(path);  
//    status = CreateDirectory(path,NULL);  
#else
    status = mkdir(path,0777);
#endif
    if ( (status != 0) && (errno != EEXIST) ) {
        /* We don't want to treat EEXIST as a real error */
        fprintf(stderr,"mkdir failed with code %d and errno %d for path: %s\n",
          status, errno, path);
        perror("Failed creating directory");
    }
  }

  retval = stat(path, &statBuf);
  free(path);
  return retval;
}

short vtkFileOps::convertShortFromGE(short ge)
{
  char *x =(char *)&ge;
  return (x[0] * (1 << 8) + x[1]);
         /* MSB             LSB in GE format */
}

int vtkFileOps::IsMSBFirstForShort(void)
/* Return 1 if MSB of a short is at the first byte */
{
  union {
    short i;
    char c[sizeof(short)];
  } kludge;
  kludge.i = 0;
  kludge.c[0] = 1;
 
  if (kludge.i != 1) {
    return 1;
  } else {
    return 0;
  }
}

void vtkFileOps::ensureGEByteOrderForShort(short *data, int np)
{
  int i = 0;
  /* If it is not in the GE order, swap the bytes */
  if (! this->IsMSBFirstForShort()) {
    for (i=0; i < np; i++) {
       /* Flip the bytes around */
       data[i] = this->convertShortFromGE(data[i]);
    }
  }
}

int vtkFileOps::WriteMRIfile(char *fname, unsigned char *header, int headersize, short *data, int npixels)
{
    fprintf (stdout, "vtkFileOps::WriteMRIfile: ERROR:KILIAN Fix me\n");
    exit(1);
#ifndef _WIN32
  FILE *fp = NULL;

  /* To handle automatic compression */
  char *uncompressedName = NULL;
  char command[4*PATH_MAX];
  int errcode = 0;

  int writeItCompressed = 0;

  assert(data != NULL);

  /* Decide here, before opening the file, if it will be written compressed */
  if (this->fileIsCompressed(fname,(char **)NULL) == 1) {
    writeItCompressed = 1;
  }
  if (this->makeDirectoryIfNeeded(fname) != 0) {
      fprintf(stderr,"makeDirectoryIfNeeded failed for %s",fname);
      return -1;
   }

  /* open the file */
  fp = fopen(fname,"wb");
  if (fp == NULL) {
    fprintf(stderr,"Failed to open file %s for writing\n",fname);
    return -1;
  }

  if (header != NULL) {
    if (fwrite(header,sizeof(unsigned char), headersize,fp) < 
        static_cast<unsigned int>(headersize)) {
      fprintf(stderr,"Failed writing the output header to %s\n",fname);
      fclose(fp);
      exit(1);
    }
  }
  /* So we can write out the pixels in GE order on non-GE order machines */
  this->ensureGEByteOrderForShort(data,npixels);
  
  // Original: if (fwrite(data,sizeof(unsigned short), npixels,fp) < npixels) {
  if (fwrite(data,sizeof(short), npixels,fp) < 
      static_cast<unsigned int>(npixels)) {
    fprintf(stderr,"Failed writing the output data\n");
    fclose(fp);
    assert(0);
    exit(1);
  }
  fclose(fp);

  /* 
   *   I want to support automatic and transparent compression of files.
   *  If the file name does not end in .gz, I don't want to compress the file,
   * unless there is another file already present that does end in .gz.
   *  If the file name already ends in .gz, I need to be careful, since
   *  gzip will refuse to process such a file.
   */
 
  /* New behaviour: only compress files if it was already compressed */
  if (writeItCompressed) {
    /*File is currently compressed */
    if (this->uncompressedFileName(fname, &uncompressedName) != 0) {
      fprintf(stderr,"Failed attempting to determine uncompressed file name");
    }
    /* save the file using its uncompressed name, then move it to its
       compressed name
     */
    if (strcmp(fname,uncompressedName) == 0) {
      sprintf(command,"gzip --fast -f \"%s\"",fname);
    } else {
      sprintf(command,"mv \"%s\" \"%s\" ; gzip --fast -f \"%s\"",fname, 
        uncompressedName, uncompressedName);
    }
    free(uncompressedName);

    errcode = system(command);
    if (errcode != 0) {
      perror("Failed attempting to compress file");
      return -1; /* Failure */
    }
  }

  return npixels; /* Success */
#endif
  return 0;
}

int vtkFileOps::WriteMRIfile(char *fname, double *data, int np)
{
  unsigned char *header = NULL;
  int headersize = 0;
  int i = 0;
  short *newdata = NULL;

  newdata = (short *)malloc(sizeof(short)*np);
  assert(newdata != NULL);

  assert(data != NULL);

  for (i = 0; i < np; i++) {
    newdata[i] = (short)data[i];
  }
  i = WriteMRIfile(fname, header, headersize, newdata, np);
  free(newdata);
  return i;
}
