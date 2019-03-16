#ifndef __vtkMRMLPETProceduralColorNode_h
#define __vtkMRMLPETProceduralColorNode_h

#include "vtkMRMLProceduralColorNode.h"

class VTK_MRML_EXPORT vtkMRMLPETProceduralColorNode : public vtkMRMLProceduralColorNode
{
public:
  static vtkMRMLPETProceduralColorNode *New();
  vtkTypeMacro(vtkMRMLPETProceduralColorNode,vtkMRMLProceduralColorNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "PETProceduralColor";}

  ///
  ///
  void UpdateScene(vtkMRMLScene *scene) override;

  ///
  /// Get/Set for Type. In SetType, set up the custom colour options for this
  /// set of colours
  void SetType(int type) override;

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData ) override;

  /// The list of valid procedural types
  //enum
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
  /// PETMIP to display PET overlay  with white to black.
  enum
  {
    PETheat = 0,
    PETrainbow = 1,
    PETMIP = 2
  };

  /// Return the lowest and the highest integers, for use in looping
  int GetFirstType() override { return this->PETheat; }
  int GetLastType() override { return this->PETMIP; }

  const char *GetTypeAsString() override;
  void SetTypeToHeat();
  void SetTypeToRainbow();
  void SetTypeToMIP();

protected:
  vtkMRMLPETProceduralColorNode();
  ~vtkMRMLPETProceduralColorNode() override;
  vtkMRMLPETProceduralColorNode(const vtkMRMLPETProceduralColorNode&);
  void operator=(const vtkMRMLPETProceduralColorNode&);
};

#endif
