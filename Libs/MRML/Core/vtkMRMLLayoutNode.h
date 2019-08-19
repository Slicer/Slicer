#ifndef __vtkMRMLLayoutNode_h
#define __vtkMRMLLayoutNode_h

// MRML includes
#include "vtkMRMLAbstractLayoutNode.h"

class vtkXMLDataElement;

/// \brief Node that describes the view layout of the application.
///
/// When the scene is closing (vtkMRMLScene::Clear), the view arrangement is
/// set to none due to the Copy() call on an empty node.
class VTK_MRML_EXPORT vtkMRMLLayoutNode : public vtkMRMLAbstractLayoutNode
{
public:
  static vtkMRMLLayoutNode *New();
  vtkTypeMacro(vtkMRMLLayoutNode,vtkMRMLAbstractLayoutNode);
  vtkMRMLNode* CreateNodeInstance() override;
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRML methods
  //--------------------------------------------------------------------------

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  /// Get/Set for Current layout
  vtkGetMacro(ViewArrangement, int);
  virtual void SetViewArrangement(int);

  vtkGetMacro(GUIPanelVisibility, int);
  vtkSetMacro(GUIPanelVisibility, int);

  vtkGetMacro(BottomPanelVisibility, int);
  vtkSetMacro(BottomPanelVisibility, int);

  /// 0 is left side, 1 is right side
  vtkGetMacro(GUIPanelLR, int);
  vtkSetMacro(GUIPanelLR, int);

  /// Control the collapse state of the SliceControllers
  vtkGetMacro(CollapseSliceControllers, int);
  vtkSetMacro(CollapseSliceControllers, int);

  /// CompareView configuration Get/Set methods
  vtkGetMacro(NumberOfCompareViewRows, int);
  vtkSetClampMacro(NumberOfCompareViewRows, int, 1, 50);
  vtkGetMacro(NumberOfCompareViewColumns, int);
  vtkSetClampMacro(NumberOfCompareViewColumns, int, 1, 50);

  /// CompareView lightbox configuration Get/Set methods
  vtkGetMacro(NumberOfCompareViewLightboxRows, int);
  vtkSetClampMacro(NumberOfCompareViewLightboxRows, int, 1, 50);
  vtkGetMacro(NumberOfCompareViewLightboxColumns, int);
  vtkSetClampMacro(NumberOfCompareViewLightboxColumns, int, 1, 50);

  /// Set/Get the size of the main and secondary panels (size of Frame1
  /// in each panel)
  vtkGetMacro(MainPanelSize, int);
  vtkSetMacro(MainPanelSize, int);
  vtkGetMacro(SecondaryPanelSize, int);
  vtkSetMacro(SecondaryPanelSize, int);

  /// Set/Get the size of the last selected module
  vtkGetStringMacro(SelectedModule);
  vtkSetStringMacro(SelectedModule);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Layout";}

  enum SlicerLayout
    {
    SlicerLayoutInitialView = 0,
    SlicerLayoutDefaultView = 1,
    SlicerLayoutConventionalView = 2,
    SlicerLayoutFourUpView = 3,
    SlicerLayoutOneUp3DView = 4,
    SlicerLayoutOneUpSliceView = 5, // XXX Slicer 4.5 - Remove this value. Here only for backward compatibility.
    SlicerLayoutOneUpRedSliceView = 6,
    SlicerLayoutOneUpYellowSliceView = 7,
    SlicerLayoutOneUpGreenSliceView = 8,
    SlicerLayoutTabbed3DView = 9,
    SlicerLayoutTabbedSliceView = 10,
    SlicerLayoutLightboxView = 11, // XXX Slicer 4.5 - Remove this value. Here only for backward compatibility.
    SlicerLayoutCompareView = 12,
    SlicerLayoutSideBySideLightboxView = 13, // XXX Slicer 4.5 - Remove this value. Here only for backward compatibility.
    SlicerLayoutNone = 14,
    SlicerLayoutDual3DView = 15,
    SlicerLayoutConventionalWidescreenView = 16,
    SlicerLayoutCompareWidescreenView = 17,
    SlicerLayoutSingleLightboxView = 18, // XXX Slicer 4.5 - Remove this value. Here only for backward compatibility.
    SlicerLayoutTriple3DEndoscopyView = 19, // Up to here, all layouts are Slicer 3 compatible
    SlicerLayout3DPlusLightboxView = 20, // XXX Slicer 4.5 - Remove this value. Here only for backward compatibility.
    SlicerLayoutThreeOverThreeView = 21,
    SlicerLayoutFourOverFourView = 22,
    SlicerLayoutCompareGridView = 23,
    SlicerLayoutConventionalQuantitativeView = 24,
    SlicerLayoutFourUpQuantitativeView = 25,
    SlicerLayoutOneUpQuantitativeView = 26,
    SlicerLayoutTwoOverTwoView = 27,
    SlicerLayoutThreeOverThreeQuantitativeView = 28,
    SlicerLayoutSideBySideView = 29,
    SlicerLayoutFourByThreeSliceView = 30,
    SlicerLayoutFourByTwoSliceView = 31,
    SlicerLayoutFiveByTwoSliceView = 32,
    SlicerLayoutThreeByThreeSliceView = 33,
    SlicerLayoutFourUpTableView = 34,
    SlicerLayout3DTableView = 35,
    SlicerLayoutConventionalPlotView = 36,
    SlicerLayoutFourUpPlotView = 37,
    SlicerLayoutFourUpPlotTableView = 38,
    SlicerLayoutOneUpPlotView = 39,
    SlicerLayoutThreeOverThreePlotView = 40,
    SlicerLayoutDicomBrowserView = 41,
    SlicerLayoutFinalView, // special value, must be placed after the last standard view (used for iterating through all the views)

    SlicerLayoutMaximizedView = 98,
    SlicerLayoutCustomView = 99,
    SlicerLayoutUserView = 100
    };

  /// Adds a layout description with integer identifier
  /// "layout". Returns false without making any modifications if the
  /// integer identifier "layout" has already been added.
  bool AddLayoutDescription(int layout, const char* layoutDescription);

  /// Modifies a layout description for integer identifier
  /// "layout". Returns false without making any modifications if the
  /// integer identifier "layout" has NOT already been added.
  bool SetLayoutDescription(int layout, const char* layoutDescription);

  /// Query whether a layout exists with a specified integer identifier
  bool IsLayoutDescription(int layout);

  /// Get the layout description associated with a specified integer
  /// identifier. The empty string is returned if the layout does not exist.
  std::string GetLayoutDescription(int layout);

  // Get the layout description currently displayed. Used
  // internally. This is XML description corresponding to the ivar
  // ViewArrangement which is the integer identifier for the
  // layout. ViewArrangement and CurrentViewDescription may not
  // correspond while a view is being switched.
  vtkGetStringMacro(CurrentLayoutDescription);

  // Get the XML data model of the CurrentViewDescription
  vtkGetObjectMacro(LayoutRootElement, vtkXMLDataElement);

  // You are responsible to delete the returned dataElement.
  static vtkXMLDataElement* ParseLayout(const char* description);

protected:
  void UpdateCurrentLayoutDescription();
  void SetAndParseCurrentLayoutDescription(const char* description);
  vtkSetStringMacro(CurrentLayoutDescription);

protected:
  vtkMRMLLayoutNode();
  ~vtkMRMLLayoutNode() override;
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
  char*                      CurrentLayoutDescription;
  vtkXMLDataElement*         LayoutRootElement;
};

#endif
