/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkGDFReader.cxx,v $
  Date:      $Date: 2006/02/28 20:43:02 $
  Version:   $Revision: 1.12 $

=========================================================================auto=*/
#include "vtkGDFReader.h"
#include "vtkObjectFactory.h"

#ifdef _WIN32
#define strncasecmp strnicmp
#endif

vtkGDFReader* vtkGDFReader::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkGDFReader");
  if(ret)
  {
      return (vtkGDFReader*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkGDFReader;
}

//------------------------------------------------------------------------------
vtkGDFReader::vtkGDFReader()
{
    // set up defaults
    this->Title = NULL;
    this->SetTitle("Title");
    this->MeasurementName = NULL;
    this->SubjectName = NULL;
    this->DataFileName = NULL;
    this->NumClasses = 0;
    this->NumVariables = 0;
    this->DefaultVariable = NULL;
    this->DefaultVariableIndex = 0;
    this->NumberOfSubjects = 0;
    
    this->Tessellation = NULL;
    this->RegistrationSubject = NULL;
    this->DesignMatFile = NULL;
    this->Creator = NULL;
    this->SmoothSteps = 0;
    this->SUBJECTS_DIR = NULL;
    this->SynthSeed = -1;
    this->gd2mtx = 0;
    this->Setgd2mtx("doss");

    this->lastID = 0;
    
    this->defaultMarker = "plus";
    this->defaultColour = "blue";
    this->defaultMarker2 = "cross";
    this->defaultColour2 = "red";
    
    this->ErrVal = 0;
    this->SetErrVal("ERROR");
}

//------------------------------------------------------------------------------
vtkGDFReader::~vtkGDFReader()
{
}

//------------------------------------------------------------------------------
void vtkGDFReader::ExecuteInformation()
{
    vtkDebugMacro(<< "vtkGDFReader: ExecuteInformation");
}
//------------------------------------------------------------------------------
void vtkGDFReader::Execute()
{
    vtkDebugMacro(<< "vtkGDFReader: Execute");

}

//------------------------------------------------------------------------------
vtkImageData *vtkGDFReader::GetImage(int ImageNumber)
{
  cerr << "vtkGDFReader::GetImage() called. uh oh." << endl;
  return NULL;
}

//------------------------------------------------------------------------------
void vtkGDFReader::Read()
{
    vtkDebugMacro(<< "Read");
    // read the data plot file
    if (this->DataFileName == NULL)
    {
        vtkErrorMacro(<<"ERROR: vtkGDFReader::Read data file name is null");
        return;
    }
    vtkDebugMacro(<< "About to try reading the data file " << this->DataFileName);
    if (strstr(this->DataFileName, ".bfloat") == NULL &&
        strstr(this->DataFileName, ".bshort") == NULL)
    {
        vtkErrorMacro(<<"ERROR:  vtkGDFReader::Read does not recognise file type (has to be bfloat or bshort) of data file " << this->DataFileName);
        return;
    }
/*
    vtkBVolumeReader dataReader = new vtkBVolumeReader();
    dataReader.SetFileName(this->DataFileName);

    dataReader.Delete();
*/  
}

//------------------------------------------------------------------------------
// Format of the file is specified here:
// http://surfer.nmr.mgh.harvard.edu/docs/fsgdf.txt
int vtkGDFReader::ReadHeader(const char *filename, int flag)
{
  int newID = 0;
  // pointer to the header file
  FILE *fp;
  // buffer to read into from the file
  char input[1024];
  // pointer into input
  char *line;
  // pointer for parsing out substrings in line
  char *subline;

  char tempString[1024];
  bool usedDefaultMarker = false;
  bool usedDefaultColour = false;
  
  vtkDebugMacro(<< "ReadHeader with filename " << filename << " and flag " << flag << "\n");

  // open the header file
  fp = fopen(filename, "r");
  if (!fp)
  {
      vtkErrorMacro(<<"Can't open GDF header file for reading: "<< filename);
      return newID;
  }

  // check that the first line is "GroupDescriptorFile 1"
  fgets(input, 1024, fp);
  if( input[strlen(input)-1] == '\n' ) {
      input[strlen(input)-1] = '\0';
  }
  line = input;
  while( isspace( (int)(*line) ) ) {
      line++;
  }
  if (strlen(line) > 0 && strncasecmp(line, "GroupDescriptorFile 1", 21) == 0)
  {
      // have a valid file
  }
  else
  {
      vtkErrorMacro(<<"ERROR: first line of file " << filename << " is not 'GroupDescriptorFile 1'\n");
      return newID;
  }

  this->NumClasses = 0;
  this->NumVariables = 0;
  this->NumberOfSubjects = 0;

  this->ClassesVec.clear();
  this->VariablesVec.clear();
  this->SubjectsVec.clear();

  
  // now read the values from the header and save them
  while (!feof(fp))
  {
      // Get a line. Strip newline and skip initial spaces.
      fgets( input, 1024, fp );
      if( input[strlen(input)-1] == '\n' ) {
          input[strlen(input)-1] = '\0';
      }
      line = input;
      while( isspace( (int)(*line) ) ) {
          line++;
      }
      // now parse the lines in the files for the header values, skipping ones
      // with only one character or less, or are comment lines
      if (strlen(line) > 1 && strncmp(line, "#", 1) != 0)
      {
          vtkDebugMacro(<<"Parsing line " << line << endl);
          if (strncasecmp(line, "Title", 5) == 0)
          {
              this->Title = (char *)malloc((strlen(line)) * sizeof(char));
              sscanf(line, "%*s %s", this->Title);
          }
          else if (strncasecmp(line, "Class", 5) == 0)
          {
              // this is a bit complex, as we may be defining lots of classes
              this->NumClasses++;
              // the class name is required, but there may be a marker and a
              // colour after it
              // this call will get 'Class', next calls will get the name, marker
              // and colour (if present)
              subline = strtok(line, " ");
              // add the values to this temp vector, then push that into the
              // classes vector
              vtkstd::vector <vtkstd::string> tmpVec;
              while ((subline = strtok(NULL, " ")) != NULL)
              {
                  // add to the classes vector
                  vtkDebugMacro( << "Classes:\n\t line: " << line << "\n\t subline: "<< subline << endl);
                  sscanf(subline, "%s", tempString);
                  if (tempString != NULL)
                  {
                      std::string classval = tempString;
                      tmpVec.push_back(classval);
                      vtkDebugMacro(<<"classes: got tempString: " << tempString << ", adding it to tmpVec ");
                  }
              }
              // check to make sure that have a marker and a colour, switch
              // between the two defaults if not
              if (tmpVec.size() < 2)
              {
                  if (!usedDefaultMarker) 
                      tmpVec.push_back(this->defaultMarker);
                  else
                      tmpVec.push_back(this->defaultMarker2);
                  usedDefaultMarker = !usedDefaultMarker;
              }
              if (tmpVec.size() < 3)
              {
                  if (!usedDefaultColour)
                      tmpVec.push_back(this->defaultColour);
                  else
                      tmpVec.push_back(this->defaultColour2);
                  usedDefaultColour = !usedDefaultColour;
              }
              this->ClassesVec.push_back(tmpVec);
          }
          else if (strncasecmp(line, "MeasurementName", 15) == 0)
          {
              this->MeasurementName = (char *)malloc((strlen(line)) * sizeof(char));
              sscanf(line, "%*s %s", this->MeasurementName);
          }
          else if (strncasecmp(line, "SubjectName", 11) == 0)
          {
              this->SubjectName = (char *)malloc((strlen(line)) * sizeof(char));
              sscanf(line, "%*s %s", this->SubjectName);
          }
          else if (strncasecmp(line, "DataFileName", 12) == 0)
          {
              this->DataFileName = (char *)malloc((strlen(line)) * sizeof(char));
              sscanf(line,  "%*s %s", this->DataFileName);
          }
          else if (strncasecmp(line, "Variables", 9) == 0)
          {
              // this is a bit complex, can have a list of variable names
              // after it
              // this call will get Variables, next calls will get the list element
              subline = strtok(line, " ");
              while ((subline = strtok(NULL, " ")) != NULL)
              {
                  vtkDebugMacro( << "Variables:\n\t line: " << line << "\n\t subline: "<< subline << endl);
                  sscanf(subline, "%s", tempString);
                  if (tempString != NULL)
                  {
                      std::string strval = tempString;
                      this->VariablesVec.push_back(strval);
                      vtkDebugMacro( << "Variables, got val, tempString = " << tempString << ", strval = " << strval.c_str() << ", variables vec size = " << this->VariablesVec.size() << ", subline = " << subline << endl);
                      //cerr << "Variables, got val " << this->VariablesVec[this->NumVariables] << ", variables vec size = " << this->VariablesVec.size() << ", subline = " << subline << endl;
                      this->NumVariables++;
                  }
                  else
                  {
                      cerr << "Variables, value is null!\n";
                  }
              }
          }
          else if (strncasecmp(line, "DefaultVariable", 15) == 0)
          {
              this->DefaultVariable = (char *)malloc((strlen(line)) * sizeof(char));
              sscanf(line, "%*s %s", this->DefaultVariable);
          }
          else if (strncasecmp(line, "Input", 5) == 0)
          {
              // complex, this is one of many subjects
              // Input subjectid class var1val var1val .. varnval
              this->NumberOfSubjects++;
              // this call will get 'Input', the next calls will get the
              // subject id, classname, and variables (should match
              // NumVariables)
              subline = strtok(line, " ");
              // add the values to this temp vector, then push that into the
              // subjects vector
              vtkstd::vector <vtkstd::string> tmpVec;
              while ((subline = strtok(NULL, " ")) != NULL)
              {
                  // can have a mix of strings and numbers
                  if (sscanf(subline, "%s", tempString) == 0)
                  {
                      /*
                      int tempInt;
                      if (sscanf(subline, "%d", tempInt) == 0)
                      {
                          float tempFloat;
                          if (sscanf(subline, "%f", tempFloat) == 0)
                          {
                              vtkErrorMacro(<<"ERROR parsing line " << line 
                              << ", unknown token type for " << subline << endl);
                          }
                          else
                          {
                              // write the float to a string
                              tempString += string.tempFloat;
                          }
                      }
                      else
                      {
                          // write the int to a string
                      }
                      */
                      vtkErrorMacro(<<"ERROR parsing line " << line 
                        << ", unknown token type for " << subline << endl);
                  }
                  if (tempString != NULL)
                  {
                      std::string subval = tempString;
                      tmpVec.push_back(subval);
                      vtkDebugMacro(<<"subjects: got tempString: " << tempString 
                        << ", adding it to tmpVec ");
                  }
              }
              if ((int)(tmpVec.size()-2) != this->NumVariables)
              {
                  vtkErrorMacro(<<"Error: reading in subject " 
                    << this->NumberOfSubjects 
                    << " and not getting expected number of variables (" 
                    << this->NumVariables << ") from line: "<< line << endl);
              }
              this->SubjectsVec.push_back(tmpVec);
          }
          else if (strncasecmp(line, "Tessellation", 12) == 0)
          {
              this->Tessellation = (char *)malloc((strlen(line)) * sizeof(char));
              sscanf(line, "%*s %s", this->Tessellation);
          }
          else if (strncasecmp(line, "RegistrationSubject", 19) == 0)
          {
              this->RegistrationSubject = (char *)malloc((strlen(line)) * sizeof(char));
              sscanf(line, "%*s %s", this->RegistrationSubject);
          }
          else if (strncasecmp(line, "PlotFile", 8) == 0)
          {
              this->DataFileName = (char *)malloc((strlen(line)) * sizeof(char));
              // use this as a synonym for the data file name
              sscanf(line, "%*s %s", this->DataFileName);
          }
          else if (strncasecmp(line, "DesignMatFile", 13) == 0)
          {
              this->DesignMatFile = (char *)malloc((strlen(line)) * sizeof(char));
              this->gd2mtx = (char *)malloc((strlen(line)) * sizeof(char));
              sscanf(line, "%*s %s %s", this->DesignMatFile, this->gd2mtx);
          }
          else if (strncasecmp(line, "Creator", 7) == 0)
          {
              this->Creator = (char *)malloc((strlen(line)) * sizeof(char));
              sscanf(line, "%*s %s", this->Creator);
          }
          else if (strncasecmp(line, "SmoothSteps", 11) == 0)
          {
              sscanf(line, "%*s %d", &this->SmoothSteps);
          }
          else if (strncasecmp(line, "SUBJECTS_DIR", 12) == 0)
          {
              this->SUBJECTS_DIR = (char *)malloc((strlen(line)) * sizeof(char));
              sscanf(line, "%*s %s", this->SUBJECTS_DIR);
          }
          else if (strncasecmp(line, "SynthSeed", 9) == 0)
          {
              sscanf(line, "%*s %d", &this->SynthSeed);
          }
          else if (strncasecmp(line, "LogY", 4) == 0)
            {
            sscanf(line, "%*s %d", &this->LogY);
            }
          else if (strncasecmp(line, "ResidualFWHM", 12) == 0)
            {
            sscanf(line, "%*s %g", &this->ResidualFWHM);
            }
          else if (strncasecmp(line, "DeMeanFlag", 10) == 0)
            {
            sscanf(line, "%*s %d", &this->DeMeanFlag);
            }
          else
          {
              vtkWarningMacro(<<"Warning: unknown tag in line, skipping:\n" << line);
          }
      }
  }
  // cheating for a bit, allocate data structures and then rewind the file to fill in the data
  fclose(fp);
  
  if (this->DefaultVariable == NULL && this->NumVariables > 0)
  {
      vtkDebugMacro(<<"Default Variable is null, setting it to first variable " 
        << this->VariablesVec[0].c_str());
      this->DefaultVariable = (char *)this->VariablesVec[0].c_str();
      this->DefaultVariableIndex = 0;
  }
  
  vtkDebugMacro(<< "Done reading header file " << filename << "\n");
  newID = this->lastID++;
  return newID;
}

//------------------------------------------------------------------------------
// Calculate the offset and the slope for this point
void vtkGDFReader::OffsetSlope(char * c, char * v, int x, int y, int z)
{
    vtkDebugMacro(<< "OffsetSlope: c=" << c << ", v=" << v << ", x=" << x << ",y=" << y << ",z=" << z);
}

//------------------------------------------------------------------------------
const char *vtkGDFReader::GetNthClassLabel(int n)
{
    vtkDebugMacro(<< "GetNthClassLabel\n");
    if (n < (int)this->ClassesVec.size())
    {
        return (char *)this->ClassesVec[n][0].c_str();
    }
    else
    {
        vtkErrorMacro(<<"ERROR: GetNthClassLabel: n " << n 
          << " is greater than classes vector size " << this->ClassesVec.size());
        return this->ErrVal;
    }
}

//------------------------------------------------------------------------------
const char *vtkGDFReader::GetNthClassMarker(int n)
{
    vtkDebugMacro(<< "GetNthClassMarker\n");
    if (n < (int)this->ClassesVec.size())
    {
        return (char *)this->ClassesVec[n][1].c_str();
    }
    else
    {
        vtkErrorMacro(<<"ERROR: GetNthClassMarker: n " << n 
          << " is greater than classes vector size " << this->ClassesVec.size());

        return this->ErrVal;
    }
}

//------------------------------------------------------------------------------
const char *vtkGDFReader::GetNthClassColor(int n)
{
    vtkDebugMacro(<< "GetNthClassColor\n");
    if (n < (int)this->ClassesVec.size())
    {
        return (char *)this->ClassesVec[n][2].c_str();
    }
    else
    {
        vtkErrorMacro(<<"ERROR: GetNthClassColour: n " << n 
          << " is greater than classes vector size " << this->ClassesVec.size());
        return this->ErrVal;
    }
}

//------------------------------------------------------------------------------
const char *vtkGDFReader::GetNthVariableLabel(int n)
{
    vtkDebugMacro(<< "GetNthVariableLabel\n");
    if (n < (int)this->VariablesVec.size())
    {
        return this->VariablesVec[n].c_str();
    }
    else
    {
        vtkErrorMacro(<<"ERROR: GetNthVariableLabeln " << n 
          << " is greater than variable vector size " << this->VariablesVec.size());
        return this->ErrVal;
    }
}

//------------------------------------------------------------------------------
// id is the first element of the vector of subject n
const char *vtkGDFReader::GetNthSubjectID(int n)
{
    vtkDebugMacro(<< "GetNthSubjectID\n");
    if (n < (int)this->SubjectsVec.size())
    {
        return this->SubjectsVec[n][0].c_str();
    }
    else
    {
        vtkErrorMacro(<<"ERROR: GetNthSubjectID " << n 
          << " is greater than subject vector size " << this->SubjectsVec.size());
        return this->ErrVal;
    }
}

//------------------------------------------------------------------------------
// class is the second element of the vector of subject n
const char *vtkGDFReader::GetNthSubjectClass(int n)
{
    vtkDebugMacro(<< "GetNthSubjectClass\n");
    if (n < (int)this->SubjectsVec.size())
    {
        return this->SubjectsVec[n][1].c_str();
    }
    else
    {
        vtkErrorMacro(<<"ERROR: GetNthSubjectClass " << n 
          << " is greater than subject vector size " << this->SubjectsVec.size());
        return this->ErrVal;
    }
}

//------------------------------------------------------------------------------
// the subject values start at the third entry in the subjects vec
// and go for NumVariables entries, so increment the n2 value by 2
const char *vtkGDFReader::GetNthSubjectNthValue(int n1, int n2)
{
    vtkDebugMacro(<< "GetNthSubjectNthValue\n");
    int nthIndex = n2 + 2;
    if ((unsigned int)n1 < this->SubjectsVec.size())
    {
        if ((unsigned int)nthIndex < this->SubjectsVec[n1].size())
        {
            return this->SubjectsVec[n1][nthIndex].c_str();
        }
        else
        {
            vtkErrorMacro(<<"ERROR: GetNthSubjectNthValue " << nthIndex 
              << " is greater than subject " << n1 << "'s vector size " 
              << this->SubjectsVec[n1].size() 
              << "(increment n2 by 2 to skip subject name and class)");
            return this->ErrVal;
        }
    }
    else
    {
        vtkErrorMacro(<<"ERROR: GetNthSubjectNthValue " << n1 << " is greater than subject vector size " << this->SubjectsVec.size());
        return this->ErrVal;
    }
}

//------------------------------------------------------------------------------
// return all the values for this subject
// need to return a point in x, y, z - do it in tcl
vtkFloatingPointType vtkGDFReader::GetNthSubjectMeasurement(int n, const char *subject, int x, int y, int z)
{
    vtkFloatingPointType retval;
    vtkDebugMacro(<< "GetNthSubjectMeasurement\n");
    retval = 0.0;

    return retval;
}

//------------------------------------------------------------------------------
void vtkGDFReader::PrintSelf(ostream& os, vtkIndent indent)
{
    // to index vectors
    unsigned int indx, subindx;
    
    if (this->Title != NULL)
    {
      os << indent << "Title: " << this->Title << endl;
    } 
    else 
    {
      os << indent << "Title:" << endl;
    }
    if (this->MeasurementName != NULL)
    {
      os << indent << "Measurement name: " << this->MeasurementName << endl;
    }
    else 
    {
      os << indent << "Measurement name:" << endl;
    }
    if (this->SubjectName != NULL)
    {
      os << indent << "Subject name: " << this->SubjectName << endl;
    } 
    else 
    {
      os << indent << "Subject name:" << endl;
    }
    if (this->DataFileName != NULL)
      {
        os << indent << "Data file name: " << this->DataFileName << endl;
      } 
    else 
    {
      os << indent << "Data file name:" << endl;
    }
    os << indent << "Number of classes: " << this->NumClasses << endl;
    if (this->NumClasses > 0)
    {
        for (indx = 0; indx < this->ClassesVec.size(); indx++)
        {
            os << indent << "Class " << indx << ":" << endl;
            for (subindx = 0; subindx < this->ClassesVec[indx].size(); subindx++)
            {
                os << indent << " " << this->ClassesVec[indx][subindx].c_str();
            }
            os << endl;
        } 
    }
    os << indent << "Number of variables: " << this->NumVariables << endl;
    if (this->NumVariables > 0)
    {
        
        for (indx = 0; indx < this->VariablesVec.size(); indx++)
        {
            os << indent << "Variable " << indx << ": " << this->VariablesVec[indx].c_str() << endl; 
        } 
    }
    if (this->DefaultVariable != NULL)
    {
        os << indent << "Default variable: " << this->DefaultVariable << endl;
    } 
    else 
    {
        os << indent << "Default variable:" << endl;
    }
    os << indent << "Default variable index: " << this->DefaultVariableIndex << endl;
    os << indent << "Number of subjects: " << this->NumberOfSubjects << endl;
    if (this->NumberOfSubjects > 0)
    {
        for (indx = 0; indx < this->SubjectsVec.size(); indx++)
        {
            os << indent << "Subject " << indx << ": " << endl;
            for (subindx = 0; subindx < this->SubjectsVec[indx].size(); subindx++)
            {
                os << indent << " " << this->SubjectsVec[indx][subindx].c_str();
            }
            os << endl;
            //os << indent << "\tsubject id: " << this->SubjectsVec[indx].subjectID.c_str() << endl; 
            // os << indent << "\tclass name: " << this->SubjectsVec[indx].className.c_str() << endl; 
//            os << indent << "\tvars: " << this->SubjectsVec[indx].vars[] << endl; 
        } 
    }
    
    if (this->Tessellation != NULL)
        os << indent << "Tessellation: " << this->Tessellation << endl;
    else
        os << indent << "Tessellation: " << endl;
    if (this->RegistrationSubject != NULL)
        os << indent << "RegistrationSubject: " << this->RegistrationSubject << endl;
    else
        os << indent << "RegistrationSubject: " << endl;
        
    if (this->DesignMatFile != NULL)
        os << indent << "DesignMatFile: " << this->DesignMatFile << endl;
    else
        os << indent << "DesignMatFile: " << endl;
    
    if (this->Creator != NULL)
        os << indent << "Creator: " << this->Creator << endl;
    else
        os << indent << "Creator: " << endl;

    os << indent << "SmoothSteps: " << this->SmoothSteps << endl;
    if (this->SUBJECTS_DIR != NULL)
        os << indent << "SUBJECTS_DIR: " << this->SUBJECTS_DIR << endl;
    else
        os << indent << "SUBJECTS_DIR: " << endl;
    os << indent << "SynthSeed: " << this->SynthSeed << endl;
    os << indent << "GD file conversion method: " << this->gd2mtx << endl;
    
}

//------------------------------------------------------------------------------
void vtkGDFReader::PrintStdout()
{
  vtkIndent indent;
  PrintSelf(cout, indent);
}
