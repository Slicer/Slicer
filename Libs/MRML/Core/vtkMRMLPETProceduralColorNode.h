#ifndef __vtkMRMLPETProceduralColorNode_h
#define __vtkMRMLPETProceduralColorNode_h

#include "vtkMRMLProceduralColorNode.h"

class VTK_MRML_EXPORT vtkMRMLPETProceduralColorNode : public vtkMRMLProceduralColorNode
{
public:
  static vtkMRMLPETProceduralColorNode* New();
  vtkTypeMacro(vtkMRMLPETProceduralColorNode, vtkMRMLProceduralColorNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes(const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode* node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "PETProceduralColor"; }

  ///
  ///
  void UpdateScene(vtkMRMLScene* scene) override;

  ///
  /// Get/Set for Type. In SetType, set up the custom color options for this
  /// set of colors
  void SetType(int type) override;

  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData) override;

  /// The list of valid procedural types
  // enum
  //{
  ///
  //};

  /// DisplayModifiedEvent is generated when display node parameters is changed
  enum
  {
    DisplayModifiedEvent = 20000
  };

  /// The list of valid types
  /// PETheat to display PET overlay with red to orange to yellow to white.
  /// PETrainbow to display PET overlay with cool to warm to white.
  /// PETrainbow2 is based on the PET lookup-table of Fiji PET-CT plugin
  /// (https://sourceforge.net/projects/bifijiplugins/files/extraLUT/). PETDICOM is based on the DICOM standard PET
  /// color palette (https://dicom.nema.org/medical/dicom/current/output/html/part06.html#sect_B.1.2). PEThotMetalBlue
  /// is based on the DICOM standard Hot Metal Blue color palette
  /// (https://dicom.nema.org/medical/dicom/current/output/html/part06.html#sect_B.1.3).
  /// PETMIP to display PET overlay  with white to black.
  enum
  {
    PETheat = 0,
    PETrainbow = 1,
    PETrainbow2 = 2,
    PETDICOM = 3,
    PEThotMetalBlue = 4,
    PETMIP = 5
  };

  /// Return the lowest and the highest integers, for use in looping
  int GetFirstType() override { return this->PETheat; }
  int GetLastType() override { return this->PETMIP; }

  const char* GetTypeAsString() override;
  void SetTypeToHeat();
  void SetTypeToRainbow();
  void SetTypeToRainbow2();
  void SetTypeToMIP();
  void SetTypeToDICOM();
  void SetTypeToHotMetalBlue();

protected:
  vtkMRMLPETProceduralColorNode();
  ~vtkMRMLPETProceduralColorNode() override;
  vtkMRMLPETProceduralColorNode(const vtkMRMLPETProceduralColorNode&);
  void operator=(const vtkMRMLPETProceduralColorNode&);
};

#endif
