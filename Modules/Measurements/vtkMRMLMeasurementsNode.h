#ifndef __vtkMRMLMeasurementsNode_h
#define __vtkMRMLMeasurementsNode_h

#include "vtkStringArray.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkMeasurementsWin32Header.h"
#include <string>
#include <vector>


class VTK_MEASUREMENTS_EXPORT vtkMRMLMeasurementsNode : public vtkMRMLNode
{
  public:
  static vtkMRMLMeasurementsNode *New();
  vtkTypeRevisionMacro(vtkMRMLMeasurementsNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // get/set if the widget is visible
  vtkGetMacro(Visibility, int);
  vtkSetMacro(Visibility, int);
  
  // Description:
  // Create instance of a GAD node.
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
  virtual const char* GetNodeTagName() {return "Measurements"; };

 protected:
  vtkMRMLMeasurementsNode();
  ~vtkMRMLMeasurementsNode();
  vtkMRMLMeasurementsNode(const vtkMRMLMeasurementsNode&);
  void operator=(const vtkMRMLMeasurementsNode&);

  // Description:
  // is the widget visible?
  int Visibility;
};

#endif

