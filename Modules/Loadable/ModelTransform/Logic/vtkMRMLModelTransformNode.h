#ifndef __vtkMRMLModelTransformNode_h
#define __vtkMRMLModelTransformNode_h

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include "vtkStringArray.h"

// STL includes
#include <string>
#include <vector>

#include "vtkSlicerModelTransformModuleLogicExport.h"

class VTK_SLICER_MODELTRANSFORM_MODULE_LOGIC_EXPORT vtkMRMLModelTransformNode : public vtkMRMLNode
{
  public:
  static vtkMRMLModelTransformNode *New();
  vtkTypeMacro(vtkMRMLModelTransformNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create instance
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "ModelTransformNode"; };

  vtkGetStringMacro ( InputModelID );
  vtkSetStringMacro ( InputModelID );

  vtkGetStringMacro ( TransformNodeID );
  vtkSetStringMacro ( TransformNodeID );

  vtkGetStringMacro ( OutputModelID );
  vtkSetStringMacro ( OutputModelID );

  vtkGetStringMacro ( NewModelName );
  vtkSetStringMacro ( NewModelName );

  vtkGetMacro ( TransformNormals, int );
  vtkSetMacro ( TransformNormals, int );


 protected:
  vtkMRMLModelTransformNode();
  ~vtkMRMLModelTransformNode();
  vtkMRMLModelTransformNode(const vtkMRMLModelTransformNode&);
  void operator=(const vtkMRMLModelTransformNode&);

  int   TransformNormals;
  char *NewModelName;
  char *InputModelID;
  char *TransformNodeID;
  char *OutputModelID;
};

#endif

