/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLIA_FEMeshNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLIA_FEMeshNode_h
#define __vtkMRMLIA_FEMeshNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h" 

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

#include "vtkIA_FEMesh.h"

// *** added in move of application variables to MRML node for module
//#include "vtkKWApplication.h"
#include "vtkKWRegistryHelper.h"
#include "vtkIA_FEMeshWin32Header.h"


class vtkImageData;

class VTK_IA_FEMesh_EXPORT vtkMRMLIA_FEMeshNode : public vtkMRMLNode
{
  public:
  static vtkMRMLIA_FEMeshNode *New();
  vtkTypeMacro(vtkMRMLIA_FEMeshNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "IA_FEMesh";};


  // Description:
    // Set/Get the application Auto Save Flag
//    bool GetAutoSaveFlag( );
//    void SetAutoSaveFlag(bool saveFlag);
    
    // Description:
    // Set/Get the application Auto Save Frequency
//    int  GetAutoSaveTime( );
//    void SetAutoSaveTime(int saveTime);
    
    // Description:
    // Set/Get the Auto Save Working Directory Flag
//    bool GetAutoSaveWorkDirFlag( );
 //   void SetAutoSaveWorkDirFlag(bool saveFlag);
    
    // Description:
    // Set/Get the Auto Save Directory
//    const char *GetAutoSaveDirectory() const;
 //   void SetAutoSaveDirectory(const char *dirName);
    
    // Description:
    // Set/Get the Current Render Window Text Color
//          void SetTextColor(double color[3]);
//          double *GetTextColor();
          
          // Description:
    // Set/Get the Current Render Window Background Color
//          void SetBackgroundColor(double color[3]);
//          double *GetBackgroundColor();
          
          // Description:
    // Set/Get the Auto Save Directory
//    const char *GetWorkingDirectory() const;
//    void SetWorkingDirectory(const char *dirName);
//    void InitializeWorkingDirectory(const char *dirName);
          
          // Description:
    // Get the directory to be used for automated backups of the work
//    const char *GetSaveDirectory ( );
    
    // Description:
    // Set/Get the default average edge length for building blocks when created
//    double GetAverageElementLength( );
//    void SetAverageElementLength(double length);
    
    // Description:
    // Set/Get the default precision for material properties when written to ABAQUS
//    int GetABAQUSPrecision( );
//    void SetABAQUSPrecision(int precision);
         

 
protected:
  vtkMRMLIA_FEMeshNode();
  ~vtkMRMLIA_FEMeshNode();
  vtkMRMLIA_FEMeshNode(const vtkMRMLIA_FEMeshNode&);
  void operator=(const vtkMRMLIA_FEMeshNode&);

  bool AutoSaveFlag;
  int  AutoSaveTime;
  char AutoSaveDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  bool AutoSaveWorkDirFlag; 
  char WorkingDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char DateTimeString[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char SaveDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  double AverageElementLength;
  int ABAQUSPrecision; 

};

#endif

