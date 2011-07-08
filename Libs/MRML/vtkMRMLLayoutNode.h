#ifndef __vtkMRMLLayoutNode_h
#define __vtkMRMLLayoutNode_h

// MRML includes
#include "vtkMRMLNode.h"

class vtkXMLDataElement;

/// Node that describes the view layout of the application
/// When the scene is closing (vtkMRMLScene::Clear), the view arrangement is
/// set to none due to the Copy() call on an empty node.
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
      SlicerLayoutLightboxView, // Remove?
      SlicerLayoutCompareView,
      SlicerLayoutSideBySideLightboxView, // Remove?
      SlicerLayoutNone, // 14
      SlicerLayoutDual3DView,
      SlicerLayoutConventionalWidescreenView,
      SlicerLayoutCompareWidescreenView,
      SlicerLayoutSingleLightboxView, // Remove?
      SlicerLayoutTriple3DEndoscopyView, // Up to here, all layouts are Slicer 3 compatible
      SlicerLayout3DPlusLightboxView, // really used ?, Remove?
      SlicerLayoutThreeOverThreeView, 
      SlicerLayoutFourOverFourView,
      SlicerLayoutCustomView = 99,
      SlicerLayoutUserView = 100
    };

  /// Adds a layout description with integer identifier
  /// "layout". Returns without making any modifications if the
  /// integer identifier "layout" is already in use.
  void AddLayoutDescription(int layout, const char* layoutDescription);

  /// Query whether a layout exists with a specified integer identifier
  bool IsLayoutDescription(int layout);

  /// Get the layout description associated with a specified integer
  /// identifier. The empty string is returned if the layout does not exist.
  std::string GetLayoutDescription(int layout);

  // Get the layout description currently displayed. Used
  // internally. This is XML description corresponding to the ivar
  // ViewArrangement which is the integer identifier for the
  // layout. ViewArrangement and CurrentViewArrangement may not
  // correspond while a view is being switched.
  vtkGetStringMacro(CurrentViewArrangement);

  // Get the XML data model of the CurrentViewArrangement
  vtkGetObjectMacro(LayoutRootElement, vtkXMLDataElement);

  // You are responsible to delete the returned dataElement.
  static vtkXMLDataElement* ParseLayout(const char* description);

protected:
  void UpdateLayoutDescription();
  void SetLayoutDescription(const char* description);
  vtkSetStringMacro(CurrentViewArrangement);

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
