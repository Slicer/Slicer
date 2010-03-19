#ifndef __vtkMRMLSlicerWelcomeNode_h
#define __vtkMRMLSlicerWelcomeNode_h

#include <list>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkSlicerWelcome.h"

class vtkImageData;

class VTK_SLICERWELCOME_EXPORT vtkMRMLSlicerWelcomeNode : public vtkMRMLNode
{
  public:
 
  static vtkMRMLSlicerWelcomeNode *New();
  vtkTypeMacro(vtkMRMLSlicerWelcomeNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create instance of a SlicerWelcome node.
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
  virtual const char* GetNodeTagName() {return "SlicerWelcomeParameters";};

  vtkGetMacro (GUIWidth, int);
  vtkSetMacro (GUIWidth, int);

  vtkGetMacro (WelcomeGUIWidth, int);
  vtkSetMacro (WelcomeGUIWidth, int);
  
 protected:

   vtkMRMLSlicerWelcomeNode();
  ~vtkMRMLSlicerWelcomeNode();
  vtkMRMLSlicerWelcomeNode(const vtkMRMLSlicerWelcomeNode&);
  void operator=(const vtkMRMLSlicerWelcomeNode&);

  // Description:
  // width of the GUI panel.
  int WelcomeGUIWidth;
  int GUIWidth;
  
};

#endif

