/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLLayoutLogic_h
#define __vtkMRMLLayoutLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"
#include "vtkMRMLLogicExport.h"

// MRML includes
class vtkMRMLAbstractViewNode;
class vtkMRMLLayoutNode;

// VTK includes
class vtkCollection;
class vtkXMLDataElement;

// STD includes
#include <cstdlib>
#include <vector>

/// \brief MRML logic class for layout manipulation
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the colors
///
/// vtkMRMLLayoutLogic is a logic that controls the layout node and the view
/// nodes in a MRML scene.
/// It ensures that at least one 3D view and three slice views are always in
/// the MRML scene (after a scene is closed or imported).
/// The logic keeps an up-to-date list of the different MRML view nodes
/// (3D, slice ...) that are mapped into the layout.
/// A typical use case would be:
/// <code>
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
/// </code>
/// When vtkMRMLScene::Clear() is called, vtkMRMLLayoutNode::Copy() is called
/// with an empty layout node, it sets the view arrangement to None.
/// So when the scene is created/closed/imported, the view arrangement is
/// restored to its previous valid layout.
class VTK_MRML_LOGIC_EXPORT vtkMRMLLayoutLogic : public vtkMRMLAbstractLogic
{
public:
  /// The Usual vtk class functions
  static vtkMRMLLayoutLogic *New();
  vtkTypeMacro(vtkMRMLLayoutLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  typedef std::map<std::string, std::string> ViewAttributes;
  typedef ViewAttributes ViewProperty;
  typedef std::vector<ViewProperty> ViewProperties;

  /// Add all the layout descriptions of the known layouts
  /// TBD: could be done in vtkMRMLLayoutNode directly... not sure what's best.
  virtual void   AddDefaultLayouts();

  /// Returns the best node that fits the description given in the XML
  /// attributes in the element. The attributes should be precise enough to
  /// match a unique node in the scene. Returns 0 if no node can be found.
  vtkMRMLNode*   GetViewFromElement(vtkXMLDataElement* element);
  /// This returns the best view node that matches the attributes
  vtkMRMLNode*   GetViewFromAttributes(const ViewAttributes& attributes);

  /// This returns the best view node that matches the attributes
  vtkCollection* GetViewsFromAttributes(const ViewAttributes& attributes);

  vtkMRMLNode*   CreateViewFromAttributes(const ViewAttributes& attributes);

  void ApplyProperties(const ViewProperties& properties, vtkMRMLNode* view, const std::string& action);
  void ApplyProperty(const ViewProperty& property, vtkMRMLNode* view);

  /// Returns the up-to-date list of all the nodes that are mapped in the current
  /// layout.
  vtkGetObjectMacro(ViewNodes, vtkCollection);

  /// Returns the unique layout node of the scene. The logic scan the scene at
  /// first and if it can't find a layout node, it creates one.
  vtkGetObjectMacro(LayoutNode, vtkMRMLLayoutNode);

  /// Convenient function that creates and set a layout made of only 1 view.
  /// \sa CreateMaximizedViewLayoutDescription(),
  /// vtkMRMLLayoutNode::SetLayoutDescription(),
  /// vtkMRMLLayoutNode::SlicerLayoutCustomView, vtkMRMLLayoutNode::SetViewArrangement
  void MaximizeView(vtkMRMLAbstractViewNode* viewToMaximize);

  /// Create a layout description that maximizes a view.
  /// Note that the view node must be a singleton.
  /// \sa MaximizeView()
  void CreateMaximizedViewLayoutDescription(int layout,
                                            vtkMRMLAbstractViewNode* viewToMaximize);

protected:
  /// Logic constructor
  vtkMRMLLayoutLogic();
  /// Logic destructor
  ~vtkMRMLLayoutLogic() override;
  // disable copy constructor and operator
  vtkMRMLLayoutLogic(const vtkMRMLLayoutLogic&);
  void operator=(const vtkMRMLLayoutLogic&);

  /// Reimplemented to listen to specific scene events
  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  void OnMRMLNodeModified(vtkMRMLNode* node) override;
  void OnMRMLSceneStartRestore() override;
  void OnMRMLSceneEndRestore() override;

  void UnobserveMRMLScene() override;
  void UpdateFromMRMLScene() override;

  /// Makes sure there is at least one 3D view node and three slice nodes (red,
  /// yellow and green)
  void UpdateViewNodes();

  /// Call SetLayoutNode with the unique layout node in the mrml scene
  /// Create a vtkMRMLLayoutNode if there is no layout node in the scene
  void UpdateLayoutNode();

  /// Not public as we internally take care of choosing/updating the layout node
  void SetLayoutNode(vtkMRMLLayoutNode* layoutNode);

  /// Update the logic when the layout node is set or modified
  void UpdateFromLayoutNode();
  /// Make sure the view node list mapped in the current layout is up-to-date.
  void UpdateViewCollectionsFromLayout();
  void CreateMissingViews();
  void CreateMissingViews(vtkXMLDataElement* layoutRootElement);

  /// As we pass the root element of the entire layout, it returns a list of
  /// all the nodes that are found in the layout.
  vtkCollection*     GetViewsFromLayout(vtkXMLDataElement* root);

  /// Define the compare view layouts available based on settings
  /// in the layout node
  void UpdateCompareViewLayoutDefinitions();

  /// Utility functions to browse XML data elements
  vtkXMLDataElement* GetNextViewElement(vtkXMLDataElement* viewElement);
  vtkXMLDataElement* GetNextElement(vtkXMLDataElement* element);
  ViewAttributes     GetViewElementAttributes(vtkXMLDataElement* viewElement)const;
  ViewProperties     GetViewElementProperties(vtkXMLDataElement* viewElement)const;
  ViewProperty       GetViewElementProperty(vtkXMLDataElement* viewProperty)const;

  /// Pointer on the unique Layout node of the mrml node.
  vtkMRMLLayoutNode* LayoutNode;
  int                LastValidViewArrangement;
  /// Up-to-date list of the nodes that are mapped into the scene
  vtkCollection*     ViewNodes;
  vtkXMLDataElement* ConventionalLayoutRootElement;
};

#endif

