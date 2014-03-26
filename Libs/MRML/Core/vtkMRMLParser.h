/*=========================================================================

  Program:   Slicer
  Language:  C++
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Applications/GUI/Slicer3.cxx $
  Date:      $Date: 2009-04-15 06:29:13 -0400 (Wed, 15 Apr 2009) $
  Version:   $Revision: 9206 $

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#ifndef __vtkMRMLParser_h
#define __vtkMRMLParser_h

// MRML includes
#include "vtkMRML.h"
class vtkMRMLNode;
class vtkMRMLScene;

// VTK includes
#include "vtkXMLParser.h"
class vtkCollection;

// STD includes
#include <stack>

/// \brief Parse XML scene file.
class VTK_MRML_EXPORT vtkMRMLParser : public vtkXMLParser
{
public:
  static vtkMRMLParser *New();
  vtkTypeMacro(vtkMRMLParser,vtkXMLParser);

  vtkMRMLScene* GetMRMLScene() {return this->MRMLScene;};
  void SetMRMLScene(vtkMRMLScene* scene) {this->MRMLScene = scene;};

  vtkCollection* GetNodeCollection() {return this->NodeCollection;};
  void SetNodeCollection(vtkCollection* scene) {this->NodeCollection = scene;};

protected:
  vtkMRMLParser() : MRMLScene(NULL),NodeCollection(NULL){};
  ~vtkMRMLParser() {};
  vtkMRMLParser(const vtkMRMLParser&);
  void operator=(const vtkMRMLParser&);

  virtual void StartElement(const char* name, const char** atts);
  virtual void EndElement (const char *name);

private:
  vtkMRMLScene* MRMLScene;
  vtkCollection* NodeCollection;
  std::stack< vtkMRMLNode *> NodeStack;
};

#endif
