///  vtkMRMLLayoutLogic - MRML logic class for layout manipulation
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the colors

#ifndef __vtkMRMLLayoutLogic_h
#define __vtkMRMLLayoutLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"
#include "vtkMRMLLogicWin32Header.h"

// MRML includes
class vtkMRMLLayoutNode;

// VTK includes
class vtkXMLDataElement;

// STD includes
#include <stdlib.h>

/// vtkMRMLLayoutLogic is a logic that controls the layout node and the view
/// nodes in a MRML scene.
/// It ensures that at least one 3D view and three slice views are always in
/// the MRML scene (after a scene is closed or imported).
/// The logic keeps an up-to-date list of the different MRML view nodes
/// (3D, slice ...) that are mapped into the layout.
/// A typical use case would be:
/// vtkMRMLScene* scene = vtkMRMLScene::New();
/// vtkMRMLLayoutLogic* layoutLogic = vtkMRMLLayoutLogic::New();
/// layoutLogic->SetMRMLScene(scene);
/// layoutLogic->GetLayoutNode()->SetViewArrangement(
///   vtkMRMLLayoutNode::SlicerLayoutConventionalView);
/// vtkCollection* views = layoutLogic->GetViewNodes();
/// vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(
///   views->GetItemAsObject(0));
/// vtkMRMLSliceNode* redNode = vtkMRMLSliceNode::SafeDownCast(
///   views->GetItemAsObject(1));
/// vtkMRMLSliceNode* yellowNode = vtkMRMLSliceNode::SafeDownCast(
///   views->GetItemAsObject(2));
/// vtkMRMLSliceNode* greenNode = vtkMRMLSliceNode::SafeDownCast(
///   views->GetItemAsObject(3));
class VTK_MRML_LOGIC_EXPORT vtkMRMLLayoutLogic : public vtkMRMLAbstractLogic
{
public:
  /// The Usual vtk class functions
  static vtkMRMLLayoutLogic *New();
  vtkTypeRevisionMacro(vtkMRMLLayoutLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  typedef std::map<std::string, std::string> ViewAttributes;

  ///
  /// Update logic state when MRML scene chenges
  void ProcessMRMLEvents(vtkObject * caller,
                         unsigned long event,
                         void * callData);

  /// Add all the layout descriptions of the known layouts
  /// TBD: could be done in vtkMRMLLayoutNode directly... not sure what's best.
  virtual void   AddDefaultLayouts();

  /// Returns the best node that fits the description given in the XML
  /// attributes in the element. The attributes should be precise enough to
  /// match a unique node in the scene. Returns 0 if no node can be found.
  vtkMRMLNode*   GetViewFromElement(vtkXMLDataElement* element);
  /// This returns the best view node that matches the attributes
  vtkMRMLNode*   GetViewFromAttributes(const ViewAttributes& attributes);

  /// Returns the up-to-date list of all the nodes that are mapped in the current
  /// layout.
  vtkGetObjectMacro(ViewNodes, vtkCollection);

  /// Returns the unique layout node of the scene. The logic scan the scene at
  /// first and if it can't find a layout node, it creates one.
  vtkGetObjectMacro(LayoutNode, vtkMRMLLayoutNode);

protected:
  /// Logic constructor
  vtkMRMLLayoutLogic();
  /// Logic destructor
  virtual ~vtkMRMLLayoutLogic();
  // disable copy constructor and operator
  vtkMRMLLayoutLogic(const vtkMRMLLayoutLogic&);
  void operator=(const vtkMRMLLayoutLogic&);

  /// Reimplemented to listen to specific scene events
  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  /// Makes sure there is at least one 3D view node and three slice nodes (red,
  /// yellow and green)
  void UpdateViewNodes();

  /// Call SetLayoutNode with the unique layout node in the mrml scene
  /// Create a vtkMRMLLayoutNode if there is no layout node in the scene
  void UpdateLayoutNode();

  /// Not public as we internally take care of chosing/updating the layout node
  void SetLayoutNode(vtkMRMLLayoutNode* layoutNode);

  /// Make sure the view node list mapped in the current layout is up-to-date.
  void UpdateViewCollectionsFromLayout();

  /// As we pass the root element of the entire layout, it returns a list of
  /// all the nodes that are found in the layout.
  vtkCollection*     GetViewsFromLayout(vtkXMLDataElement* root);

  /// Utility functions to browse XML data elements
  vtkXMLDataElement* GetNextViewElement(vtkXMLDataElement* viewElement);
  vtkXMLDataElement* GetNextElement(vtkXMLDataElement* element);
  ViewAttributes     GetViewElementAttributes(vtkXMLDataElement* viewElement);

  /// Pointer on the unique Layout node of the mrml node.
  vtkMRMLLayoutNode* LayoutNode;
  int                LastValidViewArrangement;
  /// Up-to-date list of the nodes that are mapped into the scene
  vtkCollection*     ViewNodes;
};

#endif

