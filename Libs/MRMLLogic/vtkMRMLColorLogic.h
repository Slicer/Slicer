/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLColorLogic.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkMRMLColorLogic - MRML logic class for color manipulation
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the colors

#ifndef __vtkMRMLColorLogic_h
#define __vtkMRMLColorLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"
#include "vtkMRMLLogicWin32Header.h"

// MRML includes
class vtkMRMLColorNode;

// STD includes
#include <stdlib.h>

class VTK_MRML_LOGIC_EXPORT vtkMRMLColorLogic : public vtkMRMLAbstractLogic
{
public:
  /// The Usual vtk class functions
  static vtkMRMLColorLogic *New();
  vtkTypeRevisionMacro(vtkMRMLColorLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Update logic state when MRML scene chenges
  void ProcessMRMLEvents(vtkObject * caller,
                         unsigned long event,
                         void * callData);

  ///
  /// Add a series of color nodes, setting the types to the defaults, so that
  /// they're accessible to the rest of Slicer
  virtual void AddDefaultColorNodes();

  ///
  /// Remove the colour nodes that were added
  virtual void RemoveDefaultColorNodes();

  ///
  /// Return the default color table node id for a given type
  static const char * GetColorTableNodeID(int type);

  ///
  /// Return the default freesurfer color node id for a given type
  static const char * GetFreeSurferColorNodeID(int type);

  ///
  /// Return the default dGEMRIC color node id for a given type
  static const char * GetdGEMRICColorNodeID(int type);

  ///
  /// Return the default PET color node id for a given type
  static const char * GetPETColorNodeID(int type);

  ///
  /// return a default color node id for a procedural color node
  /// Delete the returned char* to avoid memory leak
  static const char * GetProceduralColorNodeID(const char *name);

  ///
  /// return a default color node id for a file based node, based on the file name
  /// Delete the returned char* to avoid memory leak
  static const char * GetFileColorNodeID(const char *fileName);

  ///
  /// Return a default color node id for a freesurfer label map volume
  virtual const char * GetDefaultFreeSurferLabelMapColorNodeID();

  ///
  /// Return a default color node id for a volume
  virtual const char * GetDefaultVolumeColorNodeID();

  ///
  /// Return a default color node id for a label map
  virtual const char * GetDefaultLabelMapColorNodeID();

  ///
  /// Return a default color node id for the editor
  virtual const char * GetDefaultEditorColorNodeID();

  ///
  /// Return a default color node id for a model
  virtual const char * GetDefaultModelColorNodeID();

  ///
  /// look for color files in the Base/Logic/Resources/ColorFiles directory and
  /// put their names in the ColorFiles list. Look in any user defined color
  /// files paths and put them in the UserColorFiles list.
  virtual void FindColorFiles();

  ///
  /// Add a file to the input list list, checking first for null, duplicates
//BTX
  void AddColorFile(const char *fileName, std::vector<std::string> *Files);
//ETX
  ///
  /// load in a color file, creating a storage node. Returns 1 on success,
  /// 0 on failure.
  vtkMRMLColorNode* LoadColorFile(const char *fileName, const char *nodeName = NULL);

  ///
  /// Get/Set the user defined paths where to look for extra colour files
  vtkGetStringMacro(UserColorFilePaths);
  vtkSetStringMacro(UserColorFilePaths);

protected:
  vtkMRMLColorLogic();
  virtual ~vtkMRMLColorLogic();
  // disable copy constructor and operator
  vtkMRMLColorLogic(const vtkMRMLColorLogic&);
  void operator=(const vtkMRMLColorLogic&);

  /// Reimplemented to listen to specific scene events
  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  ///
  /// a vector holding discovered default colour files, found in the
  /// Resources/ColorFiles directory, white space separated with:
  /// int name r g b a
  /// with rgba in the range 0-255
  //BTX
  std::vector<std::string> ColorFiles;
  /// a vector holding discovered user defined colour files, found in the
  /// UserColorFilesPath directories.
  std::vector<std::string> UserColorFiles;
  //ETX
  ///
  /// a string holding delimiter separated (; on win32, : else) paths where to
  /// look for extra colour files, set from the return value of
  /// vtkMRMLApplication::GetColorFilePaths
  char *UserColorFilePaths;
};

#endif

