#ifndef __vtkMRMLModelMirrorNode_h
#define __vtkMRMLModelMirrorNode_h

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLModelNode.h"

// VTK includes
#include "vtkStringArray.h"

// STD includes
#include <string>
#include <vector>

#include "vtkSlicerModelMirrorModuleLogicExport.h"

class VTK_SLICER_MODELMIRROR_MODULE_LOGIC_EXPORT vtkMRMLModelMirrorNode : public vtkMRMLNode
{
  public:
  static vtkMRMLModelMirrorNode *New();
  vtkTypeMacro(vtkMRMLModelMirrorNode, vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "ModelMirrorNode"; };

  vtkGetObjectMacro ( InputModel, vtkMRMLModelNode );
  vtkSetObjectMacro ( InputModel, vtkMRMLModelNode );

  vtkGetObjectMacro ( OutputModel, vtkMRMLModelNode );
  vtkSetObjectMacro ( OutputModel, vtkMRMLModelNode );

  vtkGetMacro ( MirrorPlane, int );
  vtkSetMacro ( MirrorPlane, int );

  vtkGetStringMacro ( NewModelName );
  vtkSetStringMacro ( NewModelName );

  enum
    {
      ModelAddedEvent = 11000,
    };
  enum
    {
      AxialMirror = 0,
      SaggitalMirror,
      CoronalMirror,
    };
  vtkMRMLModelNode *InputModel;
  vtkMRMLModelNode *OutputModel;
  char *NewModelName;
  int MirrorPlane;


 protected:
  vtkMRMLModelMirrorNode();
  ~vtkMRMLModelMirrorNode();
  vtkMRMLModelMirrorNode(const vtkMRMLModelMirrorNode&);
  void operator=(const vtkMRMLModelMirrorNode&);

};

#endif

