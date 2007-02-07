#ifndef __vtkMRMLInteractionNode_h
#define __vtkMRMLInteractionNode_h

#include <string>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

class VTK_MRML_EXPORT vtkMRMLInteractionNode : public vtkMRMLNode
{
public:
  static vtkMRMLInteractionNode *New();
  vtkTypeMacro(vtkMRMLInteractionNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Interaction";};
  
  // Description:
  // Get/Set Current and Last mouse mode.
  vtkGetMacro (CurrentInteractionMode, int );
  void SetCurrentInteractionMode ( int mode );
  vtkGetMacro (LastInteractionMode, int );
  vtkSetMacro (LastInteractionMode, int );
  
  //BTX
  // mouse modes
  enum
    {
      PickManipulate = 0,
      SelectRegion,
      LassoRegion,
      Place,
      ViewPan,
      ViewZoom,
      ViewRotate,
      ViewTransform
    };
  // events
  enum
    {
      InteractionModeChangedEvent = 19001
    };
  //ETX 

  // Description:
  // Return a text string describing the mode
  const char *GetInteractionModeAsString(int mode);

protected:
  vtkMRMLInteractionNode();
  ~vtkMRMLInteractionNode();
  vtkMRMLInteractionNode(const vtkMRMLInteractionNode&);
  void operator=(const vtkMRMLInteractionNode&);

  // may be used if temporary overrides are possible.
  int LastInteractionMode;
  // current mouse mode
  int CurrentInteractionMode;
};

#endif
