/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFSSurfaceAnnotationReader.cxx,v $
  Date:      $Date: 2006/08/17 17:31:27 $
  Version:   $Revision: 1.14 $

=========================================================================auto=*/
#include <stdio.h>
#include "vtkFSSurfaceAnnotationReader.h"
#include "vtkObjectFactory.h"
#include "vtkFSIO.h"
#include "vtkByteSwap.h"
#include "vtkFloatArray.h"

//-------------------------------------------------------------------------
vtkFSSurfaceAnnotationReader* vtkFSSurfaceAnnotationReader::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkFSSurfaceAnnotationReader");
  if(ret)
  {
      return (vtkFSSurfaceAnnotationReader*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkFSSurfaceAnnotationReader;
}

vtkFSSurfaceAnnotationReader::vtkFSSurfaceAnnotationReader()
{
  this->Labels = NULL;
  this->Colors = NULL;
  this->NamesList = NULL;
  this->NumColorTableEntries = -1;
  this->UseExternalColorTableFile = 0; 
  strcpy (this->ColorTableFileName, "");
}

vtkFSSurfaceAnnotationReader::~vtkFSSurfaceAnnotationReader()
{
  if (NULL != this->NamesList)
  {
      free (this->NamesList);
  }
}

void vtkFSSurfaceAnnotationReader::SetColorTableFileName (char* name)
{
  if (NULL != name)
  {
      strcpy (this->ColorTableFileName, name);
  }
}

//void vtkFSSurfaceAnnotationReader::DoUseExternalColorTableFile ()
//{
//  this->UseExternalColorTableFile = true;
//}

//void vtkFSSurfaceAnnotationReader::DontUseExternalColorTableFile ()
//{
//  this->UseExternalColorTableFile = false;
//}



int vtkFSSurfaceAnnotationReader::ReadFSAnnotation()
{
  vtkIntArray *olabels = this->Labels;
  vtkLookupTable *ocolors = this->Colors;
  int result = 0;
  FILE* annotFile = NULL;
  int read;
  int numLabels;
  int* rgbs;
  int* labels;
  int labelIndex;
  int vertexIndex;
  int rgb;
  int error;
  int** colorTableRGBs;
  char** colorTableNames;
  int colorTableEntryIndex;
  int numColorTableEntries = 0;
  int r;
  int g;
  int b;
  bool found;
  bool unassignedEntry;
  int stringLength;

  int thisStep = 0;
  int totalSteps = 1;
  
  vtkDebugMacro( << "starting ReadFSAnnotation\n");
  if (olabels == NULL)
  {
      vtkErrorMacro(<< "ReadFSAnnotation: output is null");
      return -1;
  }
  if (ocolors == NULL)
  {
      vtkErrorMacro(<< "ReadFSAnnotation: color table output is null");
      return -1;
  }
  
  if (NULL == this->FileName) 
  {
      vtkErrorMacro(<< "ReadFSAnnotation: fileName not specified.");
      return -1;
  }
  
  vtkDebugMacro( << "ReadFSAnnotation: Reading surface annotation data... from " << this->FileName << "\n");
  
  // Try to open the file.
  annotFile = fopen (this->FileName, "rb");
  if (NULL == annotFile) 
  {
      vtkErrorMacro (<< "\nReadFSAnnotation: could not open file\n " 
                     << this->FileName);
      return vtkFSSurfaceAnnotationReader::FS_ERROR_LOADING_ANNOTATION;
  }  
  
  // Read the number of label elements in the file.
  read = vtkFSIO::ReadInt (annotFile, numLabels);
  vtkDebugMacro( << "ReadFSAnnotation:  after ReadInt, numLabels = " << numLabels << endl);
  
  if (read != 1)
  {
      vtkErrorMacro(<< "ReadFSAnnotation: error reading number of labels");
      fclose (annotFile);
      return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_ANNOTATION;
  }
  if (numLabels <= 0) 
  {
      vtkErrorMacro (<< "\nReadFSAnnotation: number of labels is "
                     << "0 or negative, can't process file.");
      fclose (annotFile);
      return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_ANNOTATION;
  }
  
  // Allocate our arrays to hold the rgb values from the annotation
  // file and the label indices into which we'll transform them.
  vtkDebugMacro( << "nReadFSAnnotation: Callocing rgbs and labels to be numlabels ints " << numLabels << endl);
  rgbs = (int*) calloc (numLabels, sizeof(int));
  labels = (int*) calloc (numLabels, sizeof(int));
  if (NULL == labels || NULL == rgbs)
  {
      vtkErrorMacro (<< "\nReadFSAnnotation: couldn't allocate array with\n "
                     << numLabels << " ints.");
      fclose (annotFile);
      return -1;
  }
  
  // Read in all the label rgb values.
  vtkDebugMacro( << "ReadFSAnnotation:  About to read in all the label rgb values, up to " << numLabels << ".\n");

  // this calc will be wrong, as it doesn't count the setting up of the colour
  // table stuff.
  totalSteps = numLabels*2;
      
  for (labelIndex = 0; labelIndex < numLabels; labelIndex ++ )
  {    
      if (feof (annotFile) ) 
      {
          vtkErrorMacro (<< "\nReadFSAnnotation: unexpected EOF after\n " 
                         << numLabels << " values read.");
          fclose (annotFile);
          free (rgbs);
          free (labels);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_ANNOTATION;
      }
      
      // Read a vertex index and an rgb value. Set the appropriate
      // value in the rgb array.
      read = vtkFSIO::ReadInt (annotFile, vertexIndex);
      if (read != 1)
      {
          vtkErrorMacro (<< "\nReadFSAnnotation: error reading vertex index at\n"
                         << labelIndex);
          fclose (annotFile);
          free (rgbs);
          free (labels);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_ANNOTATION;
      }
      read = vtkFSIO::ReadInt (annotFile, rgb);
      if (read != 1)
      {
          vtkErrorMacro (<< "\nReadFSAnnotation: error reading rgb value at\n"
                         << labelIndex);
          fclose (annotFile);
          free (rgbs);
          free (labels);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_ANNOTATION;
      }
      if (labelIndex < 100)
      {
          vtkDebugMacro(<< "ReadFSAnnotation: Read vertex # " << vertexIndex << " rgb = " << rgb << endl);
      }
      rgbs[vertexIndex] = rgb;
    thisStep++;
    if (thisStep % 1000 == 0)
    {
        this->UpdateProgress(1.0*thisStep/totalSteps);
    }
  }
  
  
  // Are we using an embedded or an external color table?
  vtkDebugMacro( << "ReadFSAnnotation: Are we using an external color table?\n\t" << this->UseExternalColorTableFile << endl);
  
  if (this->UseExternalColorTableFile != 0)
  {
      vtkDebugMacro(<< "ReadFSAnnotation: Using external color table file\n");
      error = ReadExternalColorTable (this->ColorTableFileName,
                      &numColorTableEntries,
                      &colorTableRGBs,
                      &colorTableNames);
      if (0 != error)
      {
          vtkErrorMacro ("ReadFSAnnotation: Got an error on reading external colour table " << error << endl);
          fclose (annotFile);
          free (rgbs);
          free (labels);
          return error;
      }
  } 
  else 
  {
      vtkDebugMacro( << "ReadFSAnnotation: Not using external color table file\n");
      error = ReadEmbeddedColorTable (annotFile,
                                      &numColorTableEntries,
                                      &colorTableRGBs,
                                      &colorTableNames);
      if (0 != error)
      {
          vtkDebugMacro( << "ReadFSAnnotation: Got an error on reading embeded colour table " << error << endl);
          // Return vtkFSSurfaceAnnotationReader::FS_NO_COLOR_TABLE here so that the caller can
          // see that while the annotation file was loaded
          // correctly, there was no embedded color table, so the
          // user must specify one.
          fclose (annotFile);
          //free (rgbs);
          //free (labels);
          vtkDebugMacro( << "ReadFSAnnotation: Returning fs no color table: " << vtkFSSurfaceAnnotationReader::FS_NO_COLOR_TABLE << endl);
          return vtkFSSurfaceAnnotationReader::FS_NO_COLOR_TABLE;
      }
  }

  
      
  // Now match up rgb values with table entries to find the label
  // indices for each vertex.
  vtkDebugMacro( << "ReadFSAnnotation: Now match up rgb values with table entries to find the label indices for each vertex, numLabels = " << numLabels << ", numColorTableEntries = " << numColorTableEntries << endl);
  
  unassignedEntry = false;
  for (labelIndex = 0; labelIndex < numLabels; labelIndex++)
  {
    if (labelIndex % 1000 == 0) {
      vtkDebugMacro( << "ReadFSAnnotation: rgbs[" << labelIndex << "] = " << rgbs[labelIndex] << " (numLabels = " << numLabels << ")" << endl);
    }
      // Expand the rgb value into separate values.
      r = rgbs[labelIndex] & 0xff;
      g = (rgbs[labelIndex] >> 8) & 0xff;
      b = (rgbs[labelIndex] >> 16) & 0xff;
/*     
if (labelIndex % 1000 == 0) {
    cerr << "\t r " << r;
    cerr <<  " g " << g;
    cerr << " b " << b << endl;
}
*/
      // Look for this rgb value in the table.
      found = false;
      for (colorTableEntryIndex = 0; 
           (found == false) && (colorTableEntryIndex < numColorTableEntries); 
           colorTableEntryIndex++)
      {
          if (colorTableRGBs[colorTableEntryIndex] != NULL &&
              r == colorTableRGBs[colorTableEntryIndex][0] &&
              g == colorTableRGBs[colorTableEntryIndex][1] &&
              b == colorTableRGBs[colorTableEntryIndex][2])
          {
              labels[labelIndex] = colorTableEntryIndex;
              found = true;
              continue;
          }
          else
          {
              if ( colorTableRGBs[colorTableEntryIndex] == NULL)
              {
                  // let's fail silently for now, as the scan through the
                  // colour table may encounter an index where the colour
                  // hasn't been initialised
                  vtkDebugMacro(<<"ReadFSAnnotation ERROR: null entry at " << colorTableEntryIndex << " of the color table (label index " << labelIndex << ")\n");
                  //fclose (annotFile);
                  //return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_ANNOTATION;
              }
          }
          
      }
      thisStep++;
      if (thisStep % 1000 == 0)
      {
          this->UpdateProgress(1.0*thisStep/totalSteps);
      }
      
      // Didn't find an entry so just set it to -1.
      if (!found)
      {
          vtkDebugMacro(<< "ReadFSAnnotation: Not found, returning a 0 in labels[" << labelIndex << "]\n");
          unassignedEntry = true;
          labels[labelIndex] = 0;
      }
  }
  
  // reset total steps, as have to do stuff for the colour table entries
  vtkDebugMacro(<<"ReadFSAnnotation: increasing totalSteps " << totalSteps << " by 3 times the number of colour table entries : " << 3*numColorTableEntries << ", thsi step is currently " << thisStep);

  totalSteps += 3*numColorTableEntries;
  
  // Copy the names as a list into a new string. First find the
  // length that the string should be.
  vtkDebugMacro(<< "ReadFSAnnotation: Copy the names as a list into a new string\n");
  stringLength = 0;
  for (colorTableEntryIndex = 0; 
       colorTableEntryIndex < numColorTableEntries; 
       colorTableEntryIndex++)
  {
      if (colorTableNames[colorTableEntryIndex] != NULL)
      {
          stringLength += strlen (colorTableNames[colorTableEntryIndex]);
          // for the other chars in the name: "nnn {} " where nn is an index
          stringLength += 7;
      }
      else
      { 
        vtkWarningMacro( "WARNING: null colour table names entry at index " << colorTableEntryIndex << endl); 
      }
      thisStep++;
      if (thisStep % 1000 == 0)
      {
          this->UpdateProgress(1.0*thisStep/totalSteps);
      }
  }
  // Allocate the string and copy all the names in, along with
  // their index. This makes it possible to use the string to
  // allocate a tcl array.
  if (NULL != this->NamesList)
  {
      free (this->NamesList);
  }
  vtkDebugMacro(<<"Allocating one big string for the names list, of size " << stringLength + 1 << "chars.\n");
  this->NamesList = (char*) calloc (stringLength+1, sizeof(char));
  if (NULL == this->NamesList) 
    {
      vtkErrorMacro(<<"ERROR: ReadFSAnnotation: could not calloc a pointer to a string of characters, of size " << stringLength + 1 );
      fclose(annotFile);
      return -1;
    }
  for (colorTableEntryIndex = 0; 
       colorTableEntryIndex < numColorTableEntries; 
       colorTableEntryIndex++)
  {
      if (colorTableNames[colorTableEntryIndex] != NULL)
      {
          sprintf (this->NamesList, "%s%3d {%s} ", this->NamesList,
                   colorTableEntryIndex, 
                   colorTableNames[colorTableEntryIndex]);
      }
      thisStep++;
      if (thisStep % 1000 == 0)
      {
          this->UpdateProgress(1.0*thisStep/totalSteps);
      }
  }
  this->NumColorTableEntries = numColorTableEntries;
  
  
  // We're done, so now start setting the values in our output objects.
  
  // Set all the values in the output array.
  olabels->SetArray (labels, numLabels, 0);
  
  // Now convert the color table arrays to a real lookup table.
  ocolors->SetNumberOfColors (numColorTableEntries);
  ocolors->SetTableRange (0.0, 255.0);
  for (colorTableEntryIndex = 0; 
       colorTableEntryIndex < numColorTableEntries; 
       colorTableEntryIndex++)
  {
      if (colorTableRGBs[colorTableEntryIndex] != NULL)
      {
          
          ocolors->SetTableValue (colorTableEntryIndex, 
                                  (colorTableRGBs[colorTableEntryIndex][0] / 255.0),
                                  (colorTableRGBs[colorTableEntryIndex][1] / 255.0),
                                  (colorTableRGBs[colorTableEntryIndex][2] / 255.0),
                                  1.0);
      }
      else
      {
          // use a default value
          vtkWarningMacro ("WARNING: colorTableRGBs null at entry index " <<  colorTableEntryIndex << ", using default value of 0 0 0" << endl);
          ocolors->SetTableValue(colorTableEntryIndex, 0, 0, 0, 1.0);
      }
      thisStep++;
      if (thisStep % 1000 == 0)
      {
          this->UpdateProgress(1.0*thisStep/totalSteps);
      }
  }
  
  if (unassignedEntry)
  {
      result = vtkFSSurfaceAnnotationReader::FS_WARNING_UNASSIGNED_LABELS;
  }

  this->SetProgressText("");
  this->UpdateProgress(0.0);
  
  // Close the file.
  fclose (annotFile);
  
  // Delete the stuff we allocated. This causes a crash in Slicer, so comment
  // it out for now
  /*
  vtkDebugMacro(<< "ReadFSAnnotation: Freeing rgbs, colorTableRGBs, colorTableNames\n");
  free (rgbs);
  vtkDebugMacro( << "done rgbs\n");
  for (colorTableEntryIndex = 0; 
       colorTableEntryIndex < numColorTableEntries; 
       colorTableEntryIndex++)
  {
      // both loops are causing bus errors/seg faults
      vtkDebugMacro( << colorTableEntryIndex << "\t");
      if (colorTableNames[colorTableEntryIndex] != NULL)
      {
          free (colorTableNames[colorTableEntryIndex]);
      }
      else
      {
          vtkDebugMacro( << "WARNING: tried to free null entry in colorTableNames at index " << colorTableEntryIndex << endl);
      }
     
      if (colorTableRGBs[colorTableEntryIndex] != NULL)
      {
          free (colorTableRGBs[colorTableEntryIndex]);
      }
      else
      {
          vtkDebugMacro( << "WARNING: tried to free null entry in colorTableRGBs at index " << colorTableEntryIndex << endl);
      }
      vtkDebugMacro( << "rgbs\n");
  }
  //vtkDebugMacro( << "done looping\n");
  free (colorTableNames);
  vtkDebugMacro( << " done colortablenames\n");
  free (colorTableRGBs);
  vtkDebugMacro( << " done colortablergbs\n");
  */
  vtkDebugMacro( << "ReadFSAnnotation: Returning " << result << endl);
  return result;
}

int vtkFSSurfaceAnnotationReader::ReadEmbeddedColorTable (FILE* annotFile,
                              int* onumEntries, 
                              int*** orgbValues, 
                              char*** onames)
{
  int read;
  int tag;
  int numColorTableEntries;
  int nameLength;
  char tableName[vtkFSSurfaceAnnotationReader::FS_COLOR_TABLE_NAME_LENGTH];
  int entryIndex;
  int** rgbValues;
  char** names;
  int flag;
  int deleteIndex;

  *onumEntries = 0;
  //  vtkDebugMacro( << "Starting ReadEmbeddedColorTable\n");
  // Embedded color tables are identified by a tag at the end of
  // the normal data. If there is no embedded table, we'll get
  // an eof error here.
  read = vtkFSIO::ReadInt (annotFile, tag);
  if (read != 1)
    {
    return vtkFSSurfaceAnnotationReader::FS_NO_COLOR_TABLE;
    }
  // check the value of the tag, should be 1
  
  if (vtkFSSurfaceAnnotationReader::FS_COLOR_TABLE_TAG == tag)
    {
    // Read the number of entries.
    read = vtkFSIO::ReadInt (annotFile, numColorTableEntries);
    if (read != 1)
      {
      vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading "
                     << "number of entries");
      return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
      }
    // check the next int, if it's >0 it's the old format, 
    // and this int is the number of entries, if it's <0 it's the new format, 
    // and it's the negative version number 
    if (numColorTableEntries > 0)
      {
      // old version
      // Read the table name.
      read = vtkFSIO::ReadInt (annotFile, nameLength);
      if (read != 1)
        {
        vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading "
                       << "table name length");
        return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
        }
      read = fread (tableName, sizeof(char), nameLength, annotFile);
      if (read != nameLength)
        {
        vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading "
                       << "table name");
        return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
        }
      
      // Allocate arrays for our r/g/b values and for our names.
      rgbValues = (int**) calloc (numColorTableEntries, sizeof(int*) );
      names = (char**) calloc (numColorTableEntries, sizeof(char*) );
      if (NULL == rgbValues || NULL == names)
        {
        vtkErrorMacro (<< "\nReadEmbeddedColorTable: error allocating "
                       << "rgb or name arrays with\n " 
                       << numColorTableEntries << " entries.");
        return -1;
        }
      
      // Read all the entries.
      for (entryIndex = 0; entryIndex < numColorTableEntries; entryIndex++)
        {
        
        // Allocate the name.
        names[entryIndex] = 
          (char*) calloc (vtkFSSurfaceAnnotationReader::FS_COLOR_TABLE_ENTRY_NAME_LENGTH,sizeof(char));
        if (NULL == names[entryIndex])
          {
          vtkErrorMacro (<< "\nReadEmbeddedColorTable: error allocating\n "
                         << "entry name " << entryIndex);
          free (rgbValues);
          for (deleteIndex = 0; deleteIndex < entryIndex; deleteIndex++)
            {
            free (names[deleteIndex]);
            }
          free (names);
          return -1;
          }
        
        // Allocate rgb holders.
        //cerr << "\nReadEmbeddedColorTable: allocating "
        //         << "rgb entry " << entryIndex << endl;
        rgbValues[entryIndex] = (int*) calloc (3, sizeof(int));
        if (NULL == rgbValues[entryIndex])
          {
          vtkErrorMacro (<< "\nReadEmbeddedColorTable: error allocating\n "
                         << "rgb entry " << entryIndex);
          free (rgbValues);
          for (deleteIndex = 0; deleteIndex <= entryIndex; deleteIndex++)
            {
            free (names[deleteIndex]);
            }
          free (names);
          return -1;
          }
        
        // Read the name length and name.
        read = vtkFSIO::ReadInt (annotFile, nameLength);
        if (read != 1)
          {
          vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading\n"
                         << "entry name length for entry " 
                         << entryIndex);
          free (rgbValues);
          for (deleteIndex = 0; deleteIndex <= entryIndex; deleteIndex++)
            {
            free (names[deleteIndex]);
            }
          free (names);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
          }
        read = fread (names[entryIndex], sizeof(char), 
                      nameLength, annotFile);
        if (read != nameLength)
          {
          vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading\n"
                         << "entry name for entry "
                         << entryIndex);
          free (rgbValues);
          for (deleteIndex = 0; deleteIndex <= entryIndex; deleteIndex++)
            {
            free (names[deleteIndex]);
            }
          free (names);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
          }
        
        // Read the in the rgb values.
        read = vtkFSIO::ReadInt (annotFile, rgbValues[entryIndex][0]);
        if (read != 1)
          {
          vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading\n "
                         << "red value for entry " << entryIndex);
          free (rgbValues);
          for (deleteIndex = 0; deleteIndex <= entryIndex; deleteIndex++)
            {
            free (names[deleteIndex]);
            }
          free (names);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
          }
        read = vtkFSIO::ReadInt (annotFile, rgbValues[entryIndex][1]);
        if (read != 1)
          {
          vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading "
                         << "green value for entry " << entryIndex);
          free (rgbValues);
          for (deleteIndex = 0; deleteIndex <= entryIndex; deleteIndex++)
            {
            free (names[deleteIndex]);
            }
          free (names);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
          }
        read = vtkFSIO::ReadInt (annotFile, rgbValues  [entryIndex][2]);
        if (read != 1)
          {
          vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading "
                         << "blue value for entry " << entryIndex);
          free (rgbValues);
          for (deleteIndex = 0; deleteIndex <= entryIndex; deleteIndex++)
            {
            free (names[deleteIndex]);
            }
          free (names);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
          }
        if (entryIndex < 100) {
        vtkDebugMacro( << "ReadEmbeddedColorTable: index " << entryIndex << " r = " << rgbValues[entryIndex][0] << ", g = " << rgbValues[entryIndex][1] << ", b = " << rgbValues[entryIndex][2]<< endl);
        }
        // A flag value, but we'll just ignore it.
        read = vtkFSIO::ReadInt (annotFile, flag);
        if (read != 1)
          {
          vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading "
                         << "flag value for entry " << entryIndex);
          free (rgbValues);
          for (deleteIndex = 0; deleteIndex <= entryIndex; deleteIndex++)
            {
            free (names[deleteIndex]);
            }
          free (names);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
          }
        }
      }
    else
      {
      //new version
      int version = -numColorTableEntries;
      vtkDebugMacro ("ReadEmbeddedColorTable: color table has "
                     << numColorTableEntries << " entries, this means a new style file and the version number is " << version);
      if (version == 2)
        {
        int num_entries_to_read, i = 0;
        int structure, len;
        char *name;
        int t;
        // read the number of entries
        read = vtkFSIO::ReadInt(annotFile, numColorTableEntries);
        if (read != 1 || numColorTableEntries < 0)
          {
          vtkErrorMacro("ReadEmbeddedColorTable: error getting number of colour table entries: " << numColorTableEntries);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
          }
        // allocate arrays for our r/g/b values and for our names.
        rgbValues = (int**) calloc (numColorTableEntries, sizeof(int*) );
        names = (char**) calloc (numColorTableEntries, sizeof(char*) );
        if (NULL == rgbValues || NULL == names)
          {
          vtkErrorMacro (<< "\nReadEmbeddedColorTable: error allocating "
                         << "rgb or name arrays with\n " 
                         << numColorTableEntries << " entries.");
          return -1;
          }
        // read the file name
        read = vtkFSIO::ReadInt(annotFile, len);
        if (read != 1 || len < 0)
          {
          vtkErrorMacro("ReadEmbeddedColorTable: error getting length of file name: " << len);
          free (rgbValues);
          free (names);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
          }
        name = (char*) malloc (len+1);
        fread (name, sizeof(char), len, annotFile);
        vtkDebugMacro("ReadEmbeddedColorTable: got name: " << name);
        // read the number of entries to read
        read = vtkFSIO::ReadInt(annotFile, num_entries_to_read);
        if (read != 1 || num_entries_to_read < 0)
          {
          vtkErrorMacro("ReadEmbeddedColorTable: error getting num entries to read: " << num_entries_to_read);
          free (rgbValues);
          free (names);
          return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
          }
        // for each entry, read in the info
        for (i = 0; i < num_entries_to_read; i++)
          {
          // read structure number first
          read =  vtkFSIO::ReadInt(annotFile, structure);
          if (read != 1 || structure < 0)
            {
            vtkErrorMacro("ReadEmbeddedColorTable: error in structure " << structure << " at entry " << i);
            free(rgbValues); free(names); 
            return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
            }
          // check if we already have an entry here
          if (rgbValues[structure] != NULL)
            {
            vtkErrorMacro("ReadEmbeddedColorTable: duplicate structure " << structure << " at index " << i);
            free(rgbValues); free(names); 
            return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
            }
          // create the entry
          rgbValues[structure] = (int*) calloc (3, sizeof(int));
          if (NULL == rgbValues[structure])
            {
            vtkErrorMacro (<< "\nReadEmbeddedColorTable: error allocating\n "
                           << "rgb entry " << structure << " at index " << i);
            free (rgbValues);
            for (deleteIndex = 0; deleteIndex <= i; deleteIndex++)
              {
              free (names[deleteIndex]);
              }
            free (names);
            return -1;
            }
          // Read the name length and name.
          read = vtkFSIO::ReadInt (annotFile, nameLength);
          if (read != 1)
            {
            vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading\n"
                           << "entry name length for entry " 
                           << structure);
            free (rgbValues);
            for (deleteIndex = 0; deleteIndex <= i; deleteIndex++)
              {
              free (names[deleteIndex]);
              }
            free (names);
            return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
            }
          names[structure] = (char*) malloc (nameLength+1);
          read = fread (names[structure], sizeof(char), 
                        nameLength, annotFile);
          if (read != nameLength)
            {
            vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading\n"
                           << "entry name for entry "
                           << structure);
            free (rgbValues);
            for (deleteIndex = 0; deleteIndex <= i; deleteIndex++)
              {
              free (names[deleteIndex]);
              }
            free (names);
            return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
            }
          // read in the color
          read  = vtkFSIO::ReadInt (annotFile, rgbValues[structure][0]);
          read += vtkFSIO::ReadInt (annotFile, rgbValues[structure][1]);
          read += vtkFSIO::ReadInt (annotFile, rgbValues[structure][2]);
          read += vtkFSIO::ReadInt (annotFile, t);
          if (read != 4)
            { 
            vtkErrorMacro (<< "\nReadEmbeddedColorTable: error reading RGBT "
                           << "value for entry structure " << structure << " at index " << i);
            free (rgbValues);
            for (deleteIndex = 0; deleteIndex <= i; deleteIndex++)
              {
              free (names[deleteIndex]);
              }
            free (names);
            return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
            }
          // alpha = 255 - transparency
          }
        // clean up
        free(name);
        }
      else
        {
        vtkErrorMacro("ReadEmbeddedColorTable: version of embedded color table not supported: " << version);
        return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
        }
      
      }
    
    }
  else
    {
    return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
    }
  
  *onumEntries = numColorTableEntries;
  *orgbValues = rgbValues;
  *onames = names;
  
  return 0;
}

int vtkFSSurfaceAnnotationReader::ReadExternalColorTable (char* fileName,
                              int* onumEntries, 
                              int*** orgbValues, 
                              char*** onames)
{
  
  FILE* file = NULL;
  int highestIndex;
  int lineNumber;
  char lineText[1024];
  int read;
  char* got;
  int numColorTableEntries;
  int entryIndex;
  int** rgbValues;
  char** names;
  char name[1024];
  int r;
  int g;
  int b;
  int deleteIndex;

  vtkDebugMacro(<< "Starting ReadExternalColorTable with file " << fileName << endl);
  file = fopen (fileName, "r");
  if (NULL == file)
  {
      vtkErrorMacro (<< "\nReadExternalColorTable: could not open file\n "
                     << fileName);
      return vtkFSSurfaceAnnotationReader::FS_ERROR_LOADING_COLOR_TABLE;
  }
  
  highestIndex = 0;
  lineNumber = 0;
  while (!feof (file)) 
  {
      got = fgets (lineText, 1024, file);
      if (got)
      {
          read = sscanf (lineText, "%d %*s %d %d %d %*s",
                         &entryIndex, &r, &g, &b);
          if (4 != read && -1 != read) 
          {
              vtkWarningMacro(<< "ReadExternalColorTable: error parsing "
                              << fileName << ": Malformed line "
                              << lineNumber << "\n\t" << lineText );
              return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
          }  
          
          if (entryIndex > highestIndex)
          {
              highestIndex = entryIndex;
          }
          
          lineNumber++;
      }
  }
  
  fclose (file);
  file = NULL;
  
  // We got the highest zero-based index, so our number of entries is
  // that plus one.
  numColorTableEntries = highestIndex + 1;
  
  // Allocate our arrays.
  vtkDebugMacro(<< "\nReadExternalColorTable: callocing rgbValues as int** x " << numColorTableEntries);
  
  rgbValues = (int**) calloc (numColorTableEntries, sizeof(int*) );
  names = (char**) calloc (numColorTableEntries, sizeof(char*) );
  if (NULL == rgbValues || NULL == names)
  {
      vtkErrorMacro (<< "\nReadExternalColorTable: error allocating "
                     << "rgb or name arrays with " 
                     << numColorTableEntries << " entries.");
      return -1;
  }
  
  // Read all the entries.
  file = fopen (fileName, "r" );
  if (NULL == file)
  {
      vtkErrorMacro (<< "\nReadExternalColorTable: could not open file\n " 
                     << fileName);
      free (rgbValues);
      free (names);
      return vtkFSSurfaceAnnotationReader::FS_ERROR_LOADING_ANNOTATION;
  }
  
  lineNumber = 0;
  while (!feof (file) ) 
  {
      
      got = fgets (lineText, 1024, file);
      if (got)
      {
          read = sscanf (lineText, "%d %s %d %d %d %*s",
                         &entryIndex, name, &r, &g, &b);
          if (5 != read && -1 != read) 
          {
              vtkWarningMacro(<< "ReadExternalColorTable: error parsing "
                              << fileName << ": Malformed line "
                              << lineNumber );
              free (rgbValues);
              free (names);
          fclose(file);
              return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_COLOR_TABLE;
          }  
          
          // Allocate the name.
          names[entryIndex] = 
              (char*) calloc (vtkFSSurfaceAnnotationReader::FS_COLOR_TABLE_ENTRY_NAME_LENGTH,sizeof(char));
          if (NULL == names[entryIndex])
          {
              vtkErrorMacro (<< "\nReadExternalColorTable: error allocating "
                             << "entry name " << entryIndex);
              free (rgbValues);
              for (deleteIndex = 0; deleteIndex < entryIndex; deleteIndex++)
              {
                  free (names[deleteIndex]);
              }
              free (names);
          fclose(file);
              return -1;
          }
          
          // Allocate rgb holders.
          rgbValues[entryIndex] = (int*) calloc (3, sizeof(int));
          if (rgbValues[entryIndex] == NULL)
          {
              vtkErrorMacro (<< "\nReadExternalColorTable: error allocating "
                             << "rgb entry " << entryIndex);
              free (rgbValues);
              for (deleteIndex = 0; deleteIndex <= entryIndex; deleteIndex++)
              {
                  free (names[deleteIndex]);
              }
              free (names);
          fclose(file);
              return -1;
          }
          
          // Set values.
          rgbValues[entryIndex][0] = r;
          rgbValues[entryIndex][1] = g;
          rgbValues[entryIndex][2] = b;
          
          // Copy the name.
          strcpy (names[entryIndex], name);
          lineNumber++;
      }
      
  }
  fclose(file);
  vtkDebugMacro(<< "ReadExternalColorTable: got num color table entries = " << numColorTableEntries << ", and line number = " << lineNumber );
  if (lineNumber != numColorTableEntries) {
    vtkErrorMacro(<<"ReadExternalColorTable: number of lines in the colour table file " << lineNumber << ", do not match expected number of entries " << numColorTableEntries << endl);
    return -1;
  }
  *onumEntries = numColorTableEntries;
  *orgbValues = rgbValues;
  *onames = names;

  return 0;
}

char* vtkFSSurfaceAnnotationReader::GetColorTableNames()
{
  if (NULL == this->NamesList || this->NumColorTableEntries < 0)
  {
      return NULL;
  }
  
  return this->NamesList;
}


void vtkFSSurfaceAnnotationReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataReader::PrintSelf(os,indent);
  os << indent << "Labels: ";
  if (this->Labels != NULL)
  {
      this->Labels->PrintSelf(os,indent);
  }
  else
  {
      os << "(None)\n";
  }
  os << indent << "Colors: ";
  if (this->Colors != NULL)
  {
      this->Colors->PrintSelf(os, indent);
  }
  else
  {
      os << "(None)\n";
  }
  os << indent << "Names List: ";
  if (this->NamesList != NULL)
  {
      os << this->NamesList << endl;
  }
  else
  {
      os << "(None)\n";
  }
  os << indent << "Number of Color Table Entries: " << this->NumColorTableEntries << endl;
  os << indent << "Use External Color Table File: " << this->UseExternalColorTableFile << endl;
  os << indent << "Color Table File Name: ";
  if (this->ColorTableFileName != NULL)
  {
      os << this->ColorTableFileName << endl;
  }
  else
  {
      os << "(None)\n";
  }
  
}

// write out the annotation file, using an internal color table
int vtkFSSurfaceAnnotationReader::WriteFSAnnotation()
{
    FILE* annotFile = NULL;
    vtkIntArray *olabels = this->Labels;
    vtkLookupTable *ocolors = this->Colors;
    int numLabels;
    int write;
    int labelIndex, colorTableIndex;
    double *colours;
    int rgb;
    
    vtkDebugMacro( << "starting WriteFSAnnotation\n");
    if (olabels == NULL)
    {
        vtkErrorMacro(<< "WriteFSAnnotation: labels are null");
        return -1;
    }
    if (ocolors == NULL)
    {
        vtkErrorMacro(<< "WriteFSAnnotation: color table is null");
        return -1;
    }
    if (NULL == this->FileName) 
    {
      vtkErrorMacro(<< "WriteFSAnnotation: fileName not specified.");
      return -1;
    }

    vtkDebugMacro( << "WriteFSAnnotation: Writing surface annotation data to: " << this->FileName << "\n");
    
    // Try to open the file.
    annotFile = fopen(this->FileName, "rb");
    if (annotFile != NULL)
    {
        fclose(annotFile);
    }
    annotFile = fopen (this->FileName, "wb");
    if (NULL == annotFile) 
    {
        vtkErrorMacro (<< "\nWriteFSAnnotation: could not open file\n " 
                       << this->FileName);
        return vtkFSSurfaceAnnotationReader::FS_ERROR_LOADING_ANNOTATION;
    }

    // write the number of label elements in the file.
    numLabels = olabels->GetNumberOfTuples();
    vtkDebugMacro(<<"numLabels = " << numLabels);
    
    write = vtkFSIO::WriteInt(annotFile, numLabels);
    if (write != 1)
    {
        vtkErrorMacro(<< "WriteFSAnnotation: error writing number of labels");
        fclose (annotFile);
        return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_ANNOTATION;
    }

    // write the label rgb values
    for (labelIndex = 0; labelIndex < numLabels; labelIndex ++ )
    {
        // get the index into the colour lookup table
        colorTableIndex = olabels->GetValue(labelIndex);
        vtkDebugMacro(<<"label index " << labelIndex << ", colorTableIndex " << colorTableIndex);

        if (colorTableIndex < 0 || colorTableIndex >= ocolors->GetNumberOfTableValues())
        {
            vtkErrorMacro("Colour table index " << colorTableIndex << " is out of bounds 0-" << ocolors->GetNumberOfTableValues());
            continue;
        }
        // get the label rgb
        colours = ocolors->GetTableValue(colorTableIndex);
        if (colours != NULL)
        {
            // the r, g, b were taken out of the value in the file by shifting
            // them down and anding with 0xff, so just shift them back into
            // place and or them together
            rgb = (int)(colours[0]) | ((int)(colours[1]) << 8) | ((int)(colours[2]) << 16);
        
            vtkDebugMacro(<<"label index " << labelIndex << ", colorTableIndex " << colorTableIndex << ", rgb " << rgb);
        } else {
            vtkErrorMacro(<<"Null colours at label index " << labelIndex << ", colorTableIndex " << colorTableIndex);
            rgb = 0;
        }
        // write a vertex index and an rgb value
        write = vtkFSIO::WriteInt (annotFile, labelIndex);
        if (write != 1)
        {
            vtkErrorMacro (<< "\nWriteFSAnnotation: error writing vertex index at\n"
                           << labelIndex);
            fclose (annotFile);
            return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_ANNOTATION;
        }

        // write the rgb
        write = vtkFSIO::WriteInt (annotFile, rgb);
        if (write != 1)
        {
            vtkErrorMacro (<< "\nWriteFSAnnotation: error writing rgb value at\n"
                           << labelIndex);
            fclose (annotFile);
            return vtkFSSurfaceAnnotationReader::FS_ERROR_PARSING_ANNOTATION;
        }
        if (labelIndex % 1000 == 0)
        {
            vtkDebugMacro(<< "Write: label index = " << labelIndex << ", colorTableIndex =  " << colorTableIndex);
            if (colours != NULL) {
                vtkDebugMacro(<< ", colours = " << colours[0] << "," << colours[1] << "," << colours[2] << ", rgb = " << rgb << endl);
            }
        }
    }
    // close the file and return
    fclose(annotFile);

    return 0;
}
