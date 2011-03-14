#ifndef __vtkMRMLLayoutNode_h
#define __vtkMRMLLayoutNode_h

// MRML includes
#include "vtkMRMLNode.h"

class vtkXMLDataElement;

class VTK_MRML_EXPORT vtkMRMLLayoutNode : public vtkMRMLNode
{
public:
  static vtkMRMLLayoutNode *New();
  vtkTypeMacro(vtkMRMLLayoutNode,vtkMRMLNode);
  virtual vtkMRMLNode* CreateNodeInstance();
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  /// MRML methods
  //--------------------------------------------------------------------------

  /// 
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  /// Get/Set for Current layout
  vtkGetMacro (ViewArrangement, int );
  virtual void SetViewArrangement ( int );

  vtkGetMacro (GUIPanelVisibility, int);
  vtkSetMacro (GUIPanelVisibility, int);
  
  vtkGetMacro ( BottomPanelVisibility, int );
  vtkSetMacro (BottomPanelVisibility, int );

  /// 0 is Leftside, 1 is right side
  vtkGetMacro (GUIPanelLR, int );
  vtkSetMacro (GUIPanelLR, int );

  /// 
  /// Control the collapse state of the SliceControllers
  vtkGetMacro (CollapseSliceControllers, int);
  vtkSetMacro (CollapseSliceControllers, int);

  /// 
  /// CompareView configuration Get/Set methods
  vtkGetMacro ( NumberOfCompareViewRows, int );
  vtkSetClampMacro ( NumberOfCompareViewRows, int, 1, 50);  
  vtkGetMacro ( NumberOfCompareViewColumns, int );
  vtkSetClampMacro ( NumberOfCompareViewColumns, int, 1, 50 );  

  /// 
  /// configure each compare viewer in lightbox mode.
  vtkGetMacro ( NumberOfCompareViewLightboxRows, int );
  vtkSetClampMacro ( NumberOfCompareViewLightboxRows, int, 1, 50 );  
  vtkGetMacro ( NumberOfCompareViewLightboxColumns, int );
  vtkSetClampMacro ( NumberOfCompareViewLightboxColumns, int, 1, 50 );  

  /// 
  /// Set/Get the size of the main and secondary panels (size of Frame1
  /// in each panel)
  vtkGetMacro( MainPanelSize, int);
  vtkSetMacro( MainPanelSize, int);
  vtkGetMacro( SecondaryPanelSize, int);
  vtkSetMacro( SecondaryPanelSize, int);

  /// Set/Get the size of the last selected module
  vtkGetStringMacro( SelectedModule);
  vtkSetStringMacro( SelectedModule);
  
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Layout";};

  enum SlicerLayout
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
      SlicerLayoutTabbedSliceView, // 10
      SlicerLayoutLightboxView,
      SlicerLayoutCompareView,
      SlicerLayoutSideBySideLightboxView,
      SlicerLayoutNone, // 14
      SlicerLayoutDual3DView,
      SlicerLayoutConventionalWidescreenView,
      SlicerLayoutCompareWidescreenView,
      SlicerLayoutSingleLightboxView,
      SlicerLayoutTriple3DEndoscopyView, // Up to here, all layouts are Slicer 3 compatible
      SlicerLayout3DPlusLightboxView, // really used ?
      SlicerLayoutCustomView = 99,
      SlicerLayoutUserView = 100
    };

  void AddLayoutDescription(int layout, const char* layoutDescription);
  bool IsLayoutDescription(int layout);
  std::string GetLayoutDescription(int layout);
  vtkGetStringMacro(CurrentViewArrangement);
  vtkGetObjectMacro(LayoutRootElement, vtkXMLDataElement);

protected:
  void UpdateLayoutDescription();
  void SetLayoutDescription(const char* description);
  vtkSetStringMacro(CurrentViewArrangement);
  // You are responsible to delete the reutrned dataElement.
  vtkXMLDataElement* ParseLayout(const char* description);

protected:
  vtkMRMLLayoutNode();
  ~vtkMRMLLayoutNode();
  vtkMRMLLayoutNode(const vtkMRMLLayoutNode&);
  void operator=(const vtkMRMLLayoutNode&);

  int GUIPanelVisibility;
  int BottomPanelVisibility;
  int GUIPanelLR;
  int CollapseSliceControllers;
  int ViewArrangement;
  int NumberOfCompareViewRows;
  int NumberOfCompareViewColumns;
  int NumberOfCompareViewLightboxRows;
  int NumberOfCompareViewLightboxColumns;

  char *SelectedModule;

  int MainPanelSize;
  int SecondaryPanelSize;

  std::map<int, std::string> Layouts;
  char*                      CurrentViewArrangement;
  vtkXMLDataElement*         LayoutRootElement;
};

#endif
