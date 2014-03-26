#ifndef __vtkMRMLPETProceduralColorNode_h
#define __vtkMRMLPETProceduralColorNode_h

#include "vtkMRMLProceduralColorNode.h"

class VTK_MRML_EXPORT vtkMRMLPETProceduralColorNode : public vtkMRMLProceduralColorNode
{
public:
  static vtkMRMLPETProceduralColorNode *New();
  vtkTypeMacro(vtkMRMLPETProceduralColorNode,vtkMRMLProceduralColorNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "PETProceduralColor";};

  ///
  ///
  virtual void UpdateScene(vtkMRMLScene *scene);

  ///
  /// Get/Set for Type. In SetType, set up the custom colour options for this
  /// set of colours
  virtual void SetType(int type);

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

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
  int GetFirstType() { return this->PETheat; };
  int GetLastType() { return this->PETMIP; };

  const char *GetTypeAsString();
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
