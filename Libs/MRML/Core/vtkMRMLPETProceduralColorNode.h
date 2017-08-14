#ifndef __vtkMRMLPETProceduralColorNode_h
#define __vtkMRMLPETProceduralColorNode_h

#include "vtkMRMLProceduralColorNode.h"

class VTK_MRML_EXPORT vtkMRMLPETProceduralColorNode : public vtkMRMLProceduralColorNode
{
public:
  static vtkMRMLPETProceduralColorNode *New();
  vtkTypeMacro(vtkMRMLPETProceduralColorNode,vtkMRMLProceduralColorNode);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance() VTK_OVERRIDE;

  ///
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts) VTK_OVERRIDE;

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) VTK_OVERRIDE;

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node) VTK_OVERRIDE;

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() VTK_OVERRIDE {return "PETProceduralColor";}

  ///
  ///
  virtual void UpdateScene(vtkMRMLScene *scene) VTK_OVERRIDE;

  ///
  /// Get/Set for Type. In SetType, set up the custom colour options for this
  /// set of colours
  virtual void SetType(int type) VTK_OVERRIDE;

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData ) VTK_OVERRIDE;

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
  int GetFirstType() VTK_OVERRIDE { return this->PETheat; }
  int GetLastType() VTK_OVERRIDE { return this->PETMIP; }

  const char *GetTypeAsString() VTK_OVERRIDE;
  void SetTypeToHeat();
  void SetTypeToRainbow();
  void SetTypeToMIP();

protected:
  vtkMRMLPETProceduralColorNode();
  ~vtkMRMLPETProceduralColorNode();
  vtkMRMLPETProceduralColorNode(const vtkMRMLPETProceduralColorNode&);
  void operator=(const vtkMRMLPETProceduralColorNode&);
};

#endif
