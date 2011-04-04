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
#include <cassert>

const char* conventionalView =
  "<layout type=\"vertical\" split=\"true\" >"
  " <item>"
  "  <view class=\"vtkMRMLViewNode\"/>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* fourUpView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLViewNode\"/>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "    </view>"
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
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
  "   <property name=\"orientation\" action=\"default\">Axial</property>"
  "  </view>"
  " </item>"
  "</layout>";
const char* oneUpYellowView =
  "<layout type=\"horizontal\">"
  " <item>"
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">"
  "   <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "  </view>"
  " </item>"
  "</layout>";
const char* oneUpGreenView =
  "<layout type=\"horizontal\">"
  " <item>"
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">"
  "   <property name=\"orientation\" action=\"default\">Coronal</property>"
  "  </view>"
  " </item>"
  "</layout>";

const char* tabbed3DView =
  "<layout type=\"tab\">"
  " <item multiple=\"true\">"
  "  <view class=\"vtkMRMLViewNode\"/>"
  " </item>"
  "</layout>";

const char* tabbedSliceView =
  "<layout type=\"tab\">"
  " <item name=\"Red slice\">"
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
  "   <property name=\"orientation\" action=\"default\">Axial</property>"
  "  </view>"
  " </item>"
  " <item name=\"Yellow slice\">"
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">"
  "   <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "  </view>"
  " </item>"
  " <item name=\"Green slice\">"
  "  <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">"
  "   <property name=\"orientation\" action=\"default\">Coronal</property>"
  "  </view>"
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
  "    <view class=\"vtkMRMLViewNode\" type=\"secondary\"/>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "    </view>"
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
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "    </view>"
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
  "    <view class=\"vtkMRMLViewNode\" type=\"secondary\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLViewNode\" type=\"endoscopy\"/>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* compareView =
  "<layout type=\"vertical\" split=\"true\" >"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLViewNode\"/>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"vertical\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Compare1\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Compare2\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* compareWidescreenView =
  "<layout type=\"horizontal\" split=\"true\" >"
  " <item>"
  "  <layout type=\"vertical\">"
  "   <item>"
  "    <view class=\"vtkMRMLViewNode\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Compare1\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Compare2\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* threeOverThreeView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"relayout\">Axial</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"relayout\">Sagittal</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"relayout\">Coronal</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"MediumVioletRed\">"
  "     <property name=\"orientation\" action=\"relayout\">Axial</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"RoyalBlue\">"
  "     <property name=\"orientation\" action=\"relayout\">Sagittal</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"YellowGreen\">"
  "     <property name=\"orientation\" action=\"relayout\">Coronal</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  "</layout>";

const char* fourOverFourView =
  "<layout type=\"vertical\">"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLViewNode\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "    </view>"
  "   </item>"
  "  </layout>"
  " </item>"
  " <item>"
  "  <layout type=\"horizontal\">"
  "   <item>"
  "    <view class=\"vtkMRMLViewNode\" type=\"secondary\"/>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"MediumVioletRed\">"
  "     <property name=\"orientation\" action=\"default\">Axial</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"RoyalBlue\">"
  "     <property name=\"orientation\" action=\"default\">Sagittal</property>"
  "    </view>"
  "   </item>"
  "   <item>"
  "    <view class=\"vtkMRMLSliceNode\" singletontag=\"YellowGreen\">"
  "     <property name=\"orientation\" action=\"default\">Coronal</property>"
  "    </view>"
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
  this->ConventionalLayoutRootElement = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLLayoutLogic::~vtkMRMLLayoutLogic()
{
  this->SetLayoutNode(NULL);

  this->ViewNodes->Delete();
  this->ViewNodes = NULL;
  if (this->ConventionalLayoutRootElement)
    {
    this->ConventionalLayoutRootElement->Delete();
    }
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
    // Create default 3D view + slice views
    this->UpdateViewNodes();
    // Create/Retrieve Layout node
    this->UpdateLayoutNode();
    // Restore the layout to its old state after importing a scene
    // TBD: check on GetIsUpdating() should be enough
    if (((event == vtkMRMLScene::SceneClosedEvent &&
          !this->GetMRMLScene()->GetIsUpdating()) ||
         event == vtkMRMLScene::SceneImportedEvent) &&
        this->LayoutNode->GetViewArrangement() == vtkMRMLLayoutNode::SlicerLayoutNone)
      {
      this->LayoutNode->SetViewArrangement(this->LastValidViewArrangement);
      }
    }
  else if (event == vtkCommand::ModifiedEvent && caller == this->LayoutNode)
    {
    this->UpdateFromLayoutNode();
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
  if (this->ConventionalLayoutRootElement == NULL)
    {
    // vtkMRMLLayoutLogic is responsible for the returned vtkXMLDataElement
    // pointer, this is why we delete it in the ~vtkMRMLLayoutLogic()
    this->ConventionalLayoutRootElement = vtkMRMLLayoutNode::ParseLayout(conventionalView);
    }
  this->CreateMissingViews(this->ConventionalLayoutRootElement);
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
    this->UpdateFromLayoutNode();
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
void vtkMRMLLayoutLogic::UpdateFromLayoutNode()
{
  if (this->LayoutNode &&
      this->LayoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutNone)
    {
    this->LastValidViewArrangement = this->LayoutNode->GetViewArrangement();
    }
  this->CreateMissingViews();
  this->UpdateViewCollectionsFromLayout();
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutLogic::SetLayoutNode(vtkMRMLLayoutNode* layoutNode)
{
  if (this->LayoutNode == layoutNode)
    {
    return;
    }
  // vtkMRMLLayoutLogic needs to receive the ModifiedEvent before anyone (in
  // particular qMRMLLayoutManager). It should be the case as vtkMRMLLayoutLogic
  // is the first to add an observer to it. However VTK wrongly calls the
  // firstly added observer after all the other observer are called. So we need
  // to enforce, using a priority, that it is first. The observer manager
  // can't give control over the priority so we need to do the observation
  // manually.
  //this->GetMRMLObserverManager()->SetAndObserveObject(
  //  vtkObjectPointer(&this->LayoutNode), layoutNode);

  if (this->LayoutNode)
    {
    this->LayoutNode->RemoveObservers(vtkCommand::ModifiedEvent, this->GetMRMLCallbackCommand());
    }
  this->LayoutNode = layoutNode;
  if (this->LayoutNode)
    {
    this->LayoutNode->AddObserver(vtkCommand::ModifiedEvent, this->GetMRMLCallbackCommand(), 10.);
    }

  // To do only once (when the layout node is set)
  this->AddDefaultLayouts();
  this->UpdateFromLayoutNode();
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
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutCompareView,
                                         compareView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutCompareWidescreenView,
                                         compareWidescreenView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutThreeOverThreeView,
                                         threeOverThreeView);
  this->LayoutNode->AddLayoutDescription(vtkMRMLLayoutNode::SlicerLayoutFourOverFourView,
                                         fourOverFourView);
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
vtkMRMLNode* vtkMRMLLayoutLogic::CreateViewFromAttributes(const ViewAttributes& attributes)
{
  // filter on the class name, that remove a lot of options
  ViewAttributes::const_iterator it = attributes.find(std::string("class"));
  ViewAttributes::const_iterator end = attributes.end();
  if (it == end)
    {
    return NULL;
    }
  const std::string& className = it->second;
  vtkMRMLNode* node = this->GetMRMLScene()->CreateNodeByClass(className.c_str());
  it = attributes.find(std::string("type"));
  if (it != end)
    {
    const std::string& type = it->second;
    node->SetAttribute("ViewType", type.c_str());
    }
  if (className == "vtkMRMLViewNode")
    {
    node->SetName(this->GetMRMLScene()->GetUniqueNameByString("View"));
    }
  else if (className == "vtkMRMLSliceNode")
    {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
    it = attributes.find(std::string("singletontag"));
    if (it != end)
      {
      const std::string& singletonTag = it->second;
      sliceNode->SetLayoutName(singletonTag.c_str());
      }
/*
    it = attributes.find(std::string("type"));
    if (it != end)
      {
      const std::string& orientation = it->second;
      sliceNode->SetOrientation(orientation.c_str());
      }
*/
    std::string name = std::string(sliceNode->GetLayoutName());// + std::string(sliceNode->GetOrientationString());
    node->SetName(name.c_str());
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutLogic::ApplyProperties(const ViewProperties& properties, vtkMRMLNode* view, const std::string& action)
{
  for (unsigned int i = 0; i < properties.size(); ++i)
    {
    ViewProperty property = properties[i];
    ViewProperty::const_iterator it = property.find("action");
    if (it != property.end() &&
        it->second != action)
      {
      continue;
      }
    this->ApplyProperty(property, view);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutLogic::ApplyProperty(const ViewProperty& property, vtkMRMLNode* view)
{
  std::string value;
  ViewProperty::const_iterator it = property.find("value");
  if (it != property.end())
    {
    value = it->second;
    }
  it = property.find("name");
  if (it == property.end())
    {
    vtkWarningMacro("Invalid property, no name given.");
    return;
    }

  const std::string name = it->second;
  // Orientation
  if (name == std::string("orientation"))
    {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(view);
    if (!sliceNode)
      {
      vtkWarningMacro("Invalid orientation property.");
      return;
      }
    sliceNode->SetOrientation(value.c_str());
    }
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLLayoutLogic::GetViewFromAttributes(const ViewAttributes& attributes)
{
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->GetViewsFromAttributes(attributes));
  if (nodes.GetPointer() == NULL || nodes->GetNumberOfItems() == 0)
    {
    return NULL;
    }
  return vtkMRMLNode::SafeDownCast(nodes->GetItemAsObject(0));
}

//----------------------------------------------------------------------------
vtkCollection* vtkMRMLLayoutLogic::GetViewsFromAttributes(const ViewAttributes& attributes)
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
  vtkCollection* nodes = this->GetMRMLScene()->GetNodesByClass(className.c_str());
  if (nodes == NULL || nodes->GetNumberOfItems() == 0)
    {
    vtkWarningMacro("Couldn't find nodes matching class: " << className);
    if (nodes)
      {
      nodes->Delete();
      nodes = 0;
      }
    return nodes;
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
        if (attributeValue != node->GetSingletonTag())
          {
          nodes->RemoveItem(node);
          }
        }
      if (nodes->GetNumberOfItems() > 1)
        {
        vtkWarningMacro("Couldn't find node with SingletonTag: " << attributeValue );
        // Did not find the node, return an empty list to trigger the
        // calling method, CreateMissingViews(), to create the appropriate node.
        nodes->RemoveAllItems();
        break;
        }
      assert(nodes->GetNumberOfItems() == 0 ||
             vtkMRMLNode::SafeDownCast(nodes->GetItemAsObject(0))->GetSingletonTag() == attributeValue);
      }
    else if (attributeName == "type")
      {
      if (className == "vtkMRMLSliceNode")
        {
        continue;
        }
      for (;(node = vtkMRMLNode::SafeDownCast(nodes->GetNextItemAsObject(nodesIt)));)
        {
        std::string viewType =
          node->GetAttribute("ViewType") ? node->GetAttribute("ViewType") : "";
        if (attributeValue != viewType &&
            // if there is no viewType, it's a main view.
            !(attributeValue == "main" && viewType != std::string()))
          {
          nodes->RemoveItem(node);
          }
        }
      }
    // Add here specific codes to retrieve views
    }
  return nodes;
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
void vtkMRMLLayoutLogic::CreateMissingViews()
{
  this->CreateMissingViews(
    this->LayoutNode ? this->LayoutNode->GetLayoutRootElement() : 0);
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutLogic::CreateMissingViews(vtkXMLDataElement* layoutRootElement)
{
  vtkXMLDataElement* viewElement = layoutRootElement;
  while ((viewElement = this->GetNextViewElement(viewElement)))
    {
    vtkMRMLNode* viewNode = this->GetViewFromElement(viewElement);
    if (viewNode)
      {
      continue;
      }
    ViewAttributes attributes = this->GetViewElementAttributes(viewElement);
    viewNode = this->CreateViewFromAttributes(attributes);
    if (!viewNode)
      {
      vtkWarningMacro("Can't find node for element: " << viewElement->GetName());
      viewElement->PrintXML(std::cerr, vtkIndent(0));
      }
    ViewProperties properties = this->GetViewElementProperties(viewElement);
    this->ApplyProperties(properties, viewNode, "default");
    this->GetMRMLScene()->AddNode(viewNode);
    viewNode->Delete();
    }
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
::GetViewElementAttributes(vtkXMLDataElement* viewElement)const
{
  ViewAttributes attributes;
  assert(viewElement->GetName() == std::string("view"));
  for (int i = 0; i < viewElement->GetNumberOfAttributes(); ++i)
    {
    attributes[viewElement->GetAttributeName(i)] = viewElement->GetAttributeValue(i);
    }
  return attributes;
}

//----------------------------------------------------------------------------
vtkMRMLLayoutLogic::ViewProperties vtkMRMLLayoutLogic
::GetViewElementProperties(vtkXMLDataElement* viewElement)const
{
  ViewProperties properties;
  assert(viewElement->GetName() == std::string("view"));

  for (int i = 0; i < viewElement->GetNumberOfNestedElements(); ++i)
    {
    properties.push_back(this->GetViewElementProperty(viewElement->GetNestedElement(i)));
    }
  return properties;
}

//----------------------------------------------------------------------------
vtkMRMLLayoutLogic::ViewProperty vtkMRMLLayoutLogic
::GetViewElementProperty(vtkXMLDataElement* viewProperty)const
{
  ViewProperty property;
  assert(viewProperty->GetName() == std::string("property"));

  for (int i = 0; i < viewProperty->GetNumberOfAttributes(); ++i)
    {
    property[viewProperty->GetAttributeName(i)] = viewProperty->GetAttributeValue(i);
    }
  property["value"]  = viewProperty->GetCharacterData();
  return property;
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
