/*=========================================================================

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
  static vtkMRMLParser* New();
  vtkTypeMacro(vtkMRMLParser, vtkXMLParser);

  vtkMRMLScene* GetMRMLScene() { return this->MRMLScene; };
  void SetMRMLScene(vtkMRMLScene* scene) { this->MRMLScene = scene; };

  vtkCollection* GetNodeCollection() { return this->NodeCollection; };
  void SetNodeCollection(vtkCollection* scene) { this->NodeCollection = scene; };

protected:
  vtkMRMLParser() = default;
  ;
  ~vtkMRMLParser() override = default;
  vtkMRMLParser(const vtkMRMLParser&);
  void operator=(const vtkMRMLParser&);

  void StartElement(const char* name, const char** atts) override;
  void EndElement(const char* name) override;

  /// Report progress of parsing to the scene (ProgressImportEvent) when the scene is being imported.
  void ReportParsingProgress();

private:
  vtkMRMLScene* MRMLScene{ nullptr };
  vtkCollection* NodeCollection{ nullptr };
  std::stack<vtkMRMLNode*> NodeStack;
  /// Size of the parsed file or string in bytes (-1 if not computed yet)
  long long TotalInputSize{ -1 };
  int LastReportedProgress{ -1 };
};

#endif
