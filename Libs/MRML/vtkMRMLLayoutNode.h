#ifndef __vtkMRMLLayoutNode_h
#define __vtkMRMLLayoutNode_h


#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkObject.h"

class VTK_MRML_EXPORT vtkMRMLLayoutNode : public vtkMRMLNode
{
public:
  static vtkMRMLLayoutNode *New();
  vtkTypeMacro(vtkMRMLLayoutNode,vtkMRMLNode);
  virtual vtkMRMLNode* CreateNodeInstance();
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRML methods
  //--------------------------------------------------------------------------

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Get/Set for Current layout
  vtkGetMacro (ViewArrangement, int );
  virtual void SetViewArrangement ( int );

  // Get/Set for Previous layout
  vtkGetMacro (OldViewArrangement, int );
  vtkSetMacro (OldViewArrangement, int );
  
  vtkGetMacro (GUIPanelVisibility, int);
  vtkSetMacro (GUIPanelVisibility, int);
  
  vtkGetMacro ( BottomPanelVisibility, int );
  vtkSetMacro (BottomPanelVisibility, int );

  // 0 is Leftside, 1 is right side
  vtkGetMacro (GUIPanelLR, int );
  vtkSetMacro (GUIPanelLR, int );

  // Description:
  // CompareView configuration Get/Set methods
  vtkGetMacro ( NumberOfCompareViewRows, int );
  vtkSetMacro ( NumberOfCompareViewRows, int );  
  vtkGetMacro ( NumberOfCompareViewColumns, int );
  vtkSetMacro ( NumberOfCompareViewColumns, int );  

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Layout";};


  //BTX
  enum
    {
      SlicerLayoutInitialView = 0,
      SlicerLayoutDefaultView,
      SlicerLayoutConventionalView,
      SlicerLayoutFourUpView,
      SlicerLayoutOneUp3DView,
      SlicerLayoutOneUpSliceView,
      SlicerLayoutOneUpRedSliceView,
      SlicerLayoutOneUpYellowSliceView,
      SlicerLayoutOneUpGreenSliceView,
      SlicerLayoutTabbed3DView,
      SlicerLayoutTabbedSliceView,
      SlicerLayoutLightboxView,
      SlicerLayoutCompareView
    };
  //ETX


protected:
  vtkMRMLLayoutNode();
  ~vtkMRMLLayoutNode();
  vtkMRMLLayoutNode(const vtkMRMLLayoutNode&);
  void operator=(const vtkMRMLLayoutNode&);

  int GUIPanelVisibility;
  int BottomPanelVisibility;
  int GUIPanelLR;
  int ViewArrangement;
  int OldViewArrangement;
  int NumberOfCompareViewRows;
  int NumberOfCompareViewColumns;
};

#endif
