// MRMLLogic includes
#include "vtkMRMLLayoutLogic.h"

// MRML includes
#include "vtkMRMLLayoutNode.h"
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkXMLDataParser.h>
#include <vtkXMLDataElement.h>

// STD includes
#include <sstream>

const char* conventionalView =
  "<layout type=\"vertical\" split=\"true\" >"
  " <item>"
  "  <view class=\"vtkMRMLViewNode\"/>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\"/>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* fourUpView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLViewNode\"/>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\"/>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* oneUp3DView =
  "<layout type=\"horizontal\">"
  " <item>"
  "  <view class=\"vtkMRMLViewNode\"/>"
  " </item>"
  "</layout>";
const char* oneUpRedView =
  "<layout type=\"horizontal\">"
  " <item>"
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\"/>"
  " </item>"
  "</layout>";
const char* oneUpYellowView =
  "<layout type=\"horizontal\">"
  " <item>"
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\"/>"
  " </item>"
  "</layout>";
const char* oneUpGreenView =
  "<layout type=\"horizontal\">"
  " <item>"
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\"/>"
  " </item>"
  "</layout>";

const char* tabbed3DView =
  "<layout type=\"tab\">"
  " <item multiple=\"true\" name=\"$NodeID\">"
  "  <view class=\"vtkMRMLViewNode\"/>"
  " </item>"
  "</layout>";

const char* tabbedSliceView =
  "<layout type=\"tab\">"
  " <item name=\"Red slice\">"
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\"/>"
  " </item>"
  " <item name=\"Yellow slice\">"
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\"/>"
  " </item>"
  " <item name=\"Green slice\">"
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\"/>"
  " </item>"
  "</layout>";

const char* dual3DView =
  "<layout type=\"vertical\" split=\"true\" >"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLViewNode\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLViewNode\" attributes=\"secondary\"/>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\"/>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* conventionalWidescreenView =
  "<layout type=\"horizontal\" split=\"true\" >"
  " <item>"
  "  <view class=\"vtkMRMLViewNode\"/>"
  " </item>"
  " <item>"
  "  <layout type=\"vertical\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\"/>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* triple3DEndoscopyView =
  "<layout type=\"vertical\" split=\"true\" >"
  " <item>"
  "  <view class=\"vtkMRMLViewNode\"/>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLViewNode\" attributes=\"secondary\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLViewNode\" attributes=\"endoscopy\"/>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

vtkCxxRevisionMacro(vtkMRMLLayoutLogic, "$Revision$");
vtkStandardNewMacro(vtkMRMLLayoutLogic);

//----------------------------------------------------------------------------
vtkMRMLLayoutLogic::vtkMRMLLayoutLogic()
{
  this->LayoutNode = NULL;
  this->LastValidViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
  this->ViewNodes = vtkCollection::New();
}

//----------------------------------------------------------------------------
vtkMRMLLayoutLogic::~vtkMRMLLayoutLogic()
{
  this->SetLayoutNode(NULL);

  this->ViewNodes->Delete();
  this->ViewNodes = NULL;
}

//------------------------------------------------------------------------------
void vtkMRMLLayoutLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkIntArray* sceneEvents = vtkIntArray::New();
  //sceneEvents->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneImportedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents);
  sceneEvents->Delete();

  this->UpdateViewNodes();
  this->UpdateLayoutNode();
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutLogic::ProcessMRMLEvents(vtkObject * caller,
                                            unsigned long event,
                                            void * callData)
{
  vtkDebugMacro("vtkMRMLLayoutLogic::ProcessMRMLEvents: got an event " << event);

  // when there's a new scene, add the default nodes
  //if (event == vtkMRMLScene::NewSceneEvent || event == vtkMRMLScene::SceneClosedEvent)
  if (event == vtkMRMLScene::NewSceneEvent ||
      event == vtkMRMLScene::SceneClosedEvent ||
      event == vtkMRMLScene::SceneImportedEvent)
    {
    vtkDebugMacro("vtkMRMLLayoutLogic::ProcessMRMLEvents: got a NewScene event " << event);
    this->UpdateViewNodes();
    this->UpdateLayoutNode();
    // Restore the layout to its old state after importing a scene
    if (event == vtkMRMLScene::SceneImportedEvent &&
        this->LayoutNode->GetViewArrangement() == vtkMRMLLayoutNode::SlicerLayoutNone)
      {
      this->LayoutNode->SetViewArrangement(this->LastValidViewArrangement);
      }
    }
  else if (event == vtkCommand::ModifiedEvent && caller == this->LayoutNode)
    {
    if (this->LayoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutNone)
      {
      this->LastValidViewArrangement = this->LayoutNode->GetViewArrangement();
      }
    this->UpdateViewCollectionsFromLayout();
    //vtkMRMLAbstractLogic doesn't handle events not coming from the MRML scene.
    return;
    }
  this->Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkMRMLLayoutLogic: " << this->GetClassName() << "\n";
  os << indent << "LayoutNode:         " << (this->LayoutNode ? this->LayoutNode->GetID() : "none") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutLogic::UpdateViewNodes()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  // 3D view nodes
  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(
    this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLViewNode"));
  if (!viewNode)
    {
    viewNode = vtkMRMLViewNode::SafeDownCast(
      this->GetMRMLScene()->CreateNodeByClass("vtkMRMLViewNode"));
    viewNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("View"));
    this->GetMRMLScene()->AddNode(viewNode);
    viewNode->Delete();
    }
  // Slice view nodes
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(
    this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSliceNode"));
  // there must be always at 3 slice nodes (red, green and yellow)
  // here we check there is at least 1 slice node (there is either 0 or 3
  // as they always go together).
  if (!sliceNode)
    {
    std::string name;
    // Red
    sliceNode = vtkMRMLSliceNode::SafeDownCast(
      this->GetMRMLScene()->CreateNodeByClass("vtkMRMLSliceNode"));
    // Note that LayoutName and SingletonTag is the same
    sliceNode->SetLayoutName("Red");
    sliceNode->SetOrientationToAxial();
    name = std::string(sliceNode->GetLayoutName()) + std::string(sliceNode->GetOrientationString());
    sliceNode->SetName(name.c_str());
    this->GetMRMLScene()->AddNode(sliceNode);
    sliceNode->Delete();
    // Yellow
    sliceNode = vtkMRMLSliceNode::SafeDownCast(
      this->GetMRMLScene()->CreateNodeByClass("vtkMRMLSliceNode"));
    // Note that LayoutName and SingletonTag is the same
    sliceNode->SetLayoutName("Yellow");
    sliceNode->SetOrientationToSagittal();
    name = std::string(sliceNode->GetLayoutName()) + std::string(sliceNode->GetOrientationString());
    sliceNode->SetName(name.c_str());
    this->GetMRMLScene()->AddNode(sliceNode);
    sliceNode->Delete();
    // Green
    sliceNode = vtkMRMLSliceNode::SafeDownCast(
      this->GetMRMLScene()->CreateNodeByClass("vtkMRMLSliceNode"));
    // Note that LayoutName and SingletonTag is the same
    sliceNode->SetLayoutName("Green");
    sliceNode->SetOrientationToCoronal();
    name = std::string(sliceNode->GetLayoutName()) + std::string(sliceNode->GetOrientationString());
    sliceNode->SetName(name.c_str());
    this->GetMRMLScene()->AddNode(sliceNode);
    sliceNode->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutLogic::UpdateLayoutNode()
{
  if (!this->GetMRMLScene())
    {
    this->SetLayoutNode(0);
    return;
    }
  if (this->LayoutNode && this->LayoutNode->GetScene() == this->GetMRMLScene())
    {
    return;
    }
  this->GetMRMLScene()->InitTraversal();
  vtkMRMLLayoutNode* sceneLayoutNode = vtkMRMLLayoutNode::SafeDownCast(
    this->GetMRMLScene()->GetNextNodeByClass("vtkMRMLLayoutNode"));
  if (sceneLayoutNode)
    {
    this->SetLayoutNode(sceneLayoutNode);
    }
  else
    {
    sceneLayoutNode = vtkMRMLLayoutNode::New();

    // we want to set the node to the logic before adding it into the scene, in
    // case an object listens to the scene node added event and query the logic
    // about the layout node we need to be ready before it happens.
    this->SetLayoutNode(sceneLayoutNode);
    // we set the view after the layouts have been registered
    sceneLayoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutInitialView);

    // the returned value of vtkMRMLScene::AddNode can be different from its
    // input when the input is a singleton node (vtkMRMLNode::SingletonTag is 1)
    // As we observe the MRML scene, this->MRMLLayoutNode will be set in
    // onNodeAdded
    vtkMRMLNode * nodeCreated = this->GetMRMLScene()->AddNode(sceneLayoutNode);
    // as we checked that there was no vtkMRMLLayoutNode in the scene, the
    // returned node by vtkMRMLScene::AddNode() should be layoutNode
    if (nodeCreated != sceneLayoutNode)
      {
      vtkWarningMacro("Error when adding layout node into the scene");
      }
    sceneLayoutNode->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutLogic::SetLayoutNode(vtkMRMLLayoutNode* layoutNode)
{
  if (this->LayoutNode == layoutNode)
    {
    return;
    }
  this->GetMRMLObserverManager()->SetAndObserveObject(
    vtkObjectPointer(&this->LayoutNode), layoutNode);
  if (this->LayoutNode &&
      this->LayoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutNone)
    {
    this->LastValidViewArrangement = this->LayoutNode->GetViewArrangement();
    }
  this->AddDefaultLayouts();
  this->UpdateViewCollectionsFromLayout();
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutLogic::AddDefaultLayouts()
{
  if (!this->LayoutNode)
    {
    return;
    }
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutInitialView,
                                         conventionalView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutDefaultView,
                                         conventionalView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutConventionalView,
                                         conventionalView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutFourUpView,
                                         fourUpView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutOneUp3DView,
                                         oneUp3DView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView,
                                         oneUpRedView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView,
                                         oneUpYellowView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView,
                                         oneUpGreenView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutTabbed3DView,
                                         tabbed3DView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView,
                                         tabbedSliceView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutDual3DView,
                                         dual3DView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutConventionalWidescreenView,
                                         conventionalWidescreenView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutTriple3DEndoscopyView,
                                         triple3DEndoscopyView);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLLayoutLogic::GetViewFromElement(vtkXMLDataElement* element)
{
  return this->GetViewFromAttributes(this->GetViewElementAttributes(element));
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLLayoutLogic::GetViewFromAttributes(const ViewAttributes& attributes)
{
  if (!this->GetMRMLScene())
    {
    return NULL;
    }
  // filter on the class name, that remove a lot of options
  ViewAttributes::const_iterator it = attributes.find(std::string("class"));
  ViewAttributes::const_iterator end = attributes.end();
  if (it == end)
    {
    return NULL;
    }
  const std::string& className = it->second;
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->GetMRMLScene()->GetNodesByClass(className.c_str()));
  if (nodes.GetPointer() == NULL || nodes->GetNumberOfItems() == 0)
    {
    vtkWarningMacro("Couldn't find nodes matching class: " << className);
    return NULL;
    }
  vtkCollectionSimpleIterator nodesIt;
  vtkMRMLNode* node;
  for (it = attributes.begin(); it != end; ++it)
    {
    nodes->InitTraversal(nodesIt);
    std::string attributeName = it->first;
    std::string attributeValue = it->second;
    if (attributeName == "class")
      {
      continue;
      }
    else if (attributeName == "singletontag")
      {
      for (;(node = vtkMRMLNode::SafeDownCast(nodes->GetNextItemAsObject(nodesIt)));)
        {
        if (attributeValue == node->GetSingletonTag())
          {
          return node;
          }
        }
      vtkWarningMacro("Couln't find node with SingleTag: " << attributeValue);
      }
    // Add here specific codes to retrieve views
    }
  // return the first node that matches;
  return vtkMRMLNode::SafeDownCast(nodes->GetItemAsObject(0));
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutLogic::UpdateViewCollectionsFromLayout()
{
  if (!this->LayoutNode)
    {
    this->ViewNodes->RemoveAllItems();
    return;
    }
  this->ViewNodes->Delete();
  this->ViewNodes = this->GetViewsFromLayout(this->LayoutNode->GetLayoutRootElement());
}

//----------------------------------------------------------------------------
vtkCollection* vtkMRMLLayoutLogic::GetViewsFromLayout(vtkXMLDataElement* root)
{
  vtkCollection* views = vtkCollection::New();
  if (!root)
    {
    // It's normal if the view is SlicerLayoutNone, it's less if it's something
    // else; probably a layout not supported yet
    return views;
    }

  vtkXMLDataElement* viewElement = root;
  while ((viewElement = this->GetNextViewElement(viewElement)))
    {
    vtkMRMLNode* viewNode = this->GetViewFromElement(viewElement);
    if (!viewNode)
      {
      vtkWarningMacro("Can't find node for element: " << viewElement->GetName());
      viewElement->PrintXML(std::cerr, vtkIndent(0));
      }
    else
      {
      views->AddItem(viewNode);
      }
    }
  return views;
}

//----------------------------------------------------------------------------
/*
vtkCollection* vtkMRMLLayoutLogic::GetViewsFromLayoutDescription(const char * layoutDescription)
{
  vtkSmartPointer<vtkXMLDataElement> root;
  root.Take(this->ParseLayout(layoutDescription));
  return this->GetViewsFromLayout(this->LayoutNode->GetLayoutRootElement());
}
*/

//----------------------------------------------------------------------------
vtkMRMLLayoutLogic::ViewAttributes vtkMRMLLayoutLogic
::GetViewElementAttributes(vtkXMLDataElement* viewElement)
{
  ViewAttributes attributes;
  if (std::string(viewElement->GetName()) != "view")
    {
    vtkWarningMacro("Not a valid view element");
    return attributes;
    }
  for (int i = 0; i < viewElement->GetNumberOfAttributes(); ++i)
    {
    attributes[viewElement->GetAttributeName(i)] = viewElement->GetAttributeValue(i);
    }
  return attributes;
}

//----------------------------------------------------------------------------
vtkXMLDataElement* vtkMRMLLayoutLogic::GetNextViewElement(vtkXMLDataElement* viewElement)
{
  if (!viewElement)
    {
    return NULL;
    }
  while (viewElement)
    {
    vtkXMLDataElement* nextViewElement = viewElement->LookupElementWithName("view");
    if (nextViewElement)
      {
      return nextViewElement;
      }
    viewElement = this->GetNextElement(viewElement);
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkXMLDataElement* vtkMRMLLayoutLogic::GetNextElement(vtkXMLDataElement* element)
{
  if (!element)
    {
    return NULL;
    }
  // try it's sibling first
  vtkXMLDataElement* parent = element->GetParent();
  if (!parent)
    {
    return NULL;
    }
  // find the index of the current element
  for (int i = 0; i < parent->GetNumberOfNestedElements() - 1; ++i)
    {
    if (element == parent->GetNestedElement(i))
      {
      // found, return the next element
      return parent->GetNestedElement(i+1);
      }
    }
  // the element didn't have any younger sibling, pick an uncle younger than parent
  return this->GetNextElement(parent);
}
