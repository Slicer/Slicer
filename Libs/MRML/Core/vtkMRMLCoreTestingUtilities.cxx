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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 1U24CA194354-01

==============================================================================*/

// MRML includes
#include "vtkMRMLCoreTestingUtilities.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLTransformNode.h"

// vtkAddon includes
#include <vtkAddonTestingUtilities.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkStringArray.h>
#include <vtkTestErrorObserver.h>
#include <vtkURIHandler.h>

namespace vtkMRMLCoreTestingUtilities
{

//----------------------------------------------------------------------------
bool CheckNodeInSceneByID(int line, vtkMRMLScene* scene,
                          const char* nodeID, vtkMRMLNode* expected)
{
  std::string testName = "CheckNodeInSceneByID";

  if (!scene)
    {
    std::cerr << "\nLine " << line << " - scene is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (!nodeID)
    {
    std::cerr << "\nLine " << line << " - nodeID is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (nodeID[0] == '\0')
    {
    std::cerr << "\nLine " << line << " - nodeID is an empty string"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (!expected)
    {
    std::cerr << "\nLine " << line << " - expected node is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  vtkMRMLNode* current = scene->GetNodeByID(nodeID);
  if (current != expected)
    {
    const char* currentID = (current ? current->GetID() : nullptr);
    const char* expectedID = (expected ? expected->GetID() : nullptr);
    std::cerr << "\nLine " << line << " - GetNodeByID(\"" << nodeID << "\")"
              << " : " << testName << " failed"

              << "\n\tcurrent :" << current
              << ", ID: " << (currentID ? currentID : "(null)")

              << "\n\texpected:" << expected
              << ", ID: " << (expectedID ? expectedID : "(null)")
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckNodeIdAndName(int line, vtkMRMLNode* node,
                        const char* expectedID, const char* expectedName)
{
  std::string testName = "CheckNodeIdAndName";
  if (!node)
    {
    std::cerr << "\nLine " << line << " - node is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (!vtkAddonTestingUtilities::CheckString(
        line, std::string(testName) + ": Unexpected nodeID",
        node->GetID(), expectedID)

      ||!vtkAddonTestingUtilities::CheckString(
        line, std::string(testName) + ": Unexpected nodeName",
        node->GetName(), expectedName))
    {
    return false;
    }
  return true;
}

// ----------------------------------------------------------------------------
int ExerciseBasicObjectMethods(vtkObject* object)
{
  CHECK_NOT_NULL(object);
  object->Print( std::cout );
  std::cout << "Name of Class = " << object->GetClassName() << std::endl;
  std::cout << "Name of Superclass = " << object->Superclass::GetClassName() << std::endl;
  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------------
int ExerciseBasicMRMLMethods(vtkMRMLNode* node)
{
  CHECK_EXIT_SUCCESS(ExerciseBasicObjectMethods(node));

  //  Test CreateNodeInstance()
  vtkMRMLNode * newNode = node->CreateNodeInstance();
  CHECK_NOT_NULL(newNode);
  newNode->Delete();

  //  Test UpdateScene()
  node->UpdateScene(nullptr);

  //  Test New()
  vtkSmartPointer < vtkMRMLNode > node1 = vtkSmartPointer < vtkMRMLNode >::Take(node->CreateNodeInstance());

  //  Test GetID()
  CHECK_NULL(node1->GetID());

  //  Test GetNodeTagName()
  CHECK_NOT_NULL(node1->GetNodeTagName());
  CHECK_STRING_DIFFERENT(node1->GetNodeTagName(), "");

  //  Test Copy()
  node1->Copy(node);
  node->Reset(nullptr);

  //  Test SetAttribute() / GetAttribute()
  int mod = node->StartModify();
  std::string attributeName = std::string("attName");
  std::string attributeValue = std::string("attValue");
  node->SetAttribute(attributeName.c_str(), attributeValue.c_str() );
  CHECK_STRING(node->GetAttribute( attributeName.c_str() ), attributeValue.c_str());
  node->EndModify(mod);

  //  Test getters
  TEST_SET_GET_BOOLEAN( node, HideFromEditors );
  TEST_SET_GET_BOOLEAN( node, Selectable );
  TEST_SET_GET_STRING( node, Description );
  TEST_SET_GET_STRING( node, Name );
  TEST_SET_GET_STRING( node, SingletonTag );
  TEST_SET_GET_BOOLEAN( node, SaveWithScene );
  TEST_SET_GET_BOOLEAN( node, AddToScene );
  TEST_SET_GET_BOOLEAN( node, DisableModifiedEvent);
  TEST_SET_GET_BOOLEAN( node, Selected );

  //  Test singleton utility methods
  node->SetSingletonOff();
  CHECK_BOOL(node->IsSingleton(), false);
  node->SetSingletonOn();
  CHECK_BOOL(node->IsSingleton(), true);

  node->Modified();
  node->InvokePendingModifiedEvent();
  node1->SetName("copywithscene");
  node->CopyWithScene(node1);

  //  Test UpdateReferences()
  node->UpdateReferences();
  node->UpdateReferenceID("oldID", "newID");

  //  Test URLEncodeString()
  CHECK_STRING(node1->URLEncodeString("Thou Shall Test !"), "Thou%20Shall%20Test%20!");
  CHECK_STRING(node1->URLDecodeString("Thou%20Shall%20Test%20!"), "Thou Shall Test !");

  //  Test ReadXMLAttributes()
  const char *atts[] = {
            "name", "MyName",
            "description", "Testing a mrml node",
            "hideFromEditors", "false",
            "selectable", "true",
            "selected", "true",
            nullptr};
  node->ReadXMLAttributes(atts);

  //  Test WriteXML
  std::cout << "WriteXML output:" << std::endl << "------------------" << std::endl;
  node->WriteXML(std::cout, 0);
  std::cout << std::endl << "------------------" << std::endl;
  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------------
int ExerciseBasicStorableMRMLMethods(vtkMRMLStorableNode* node)
  {
  CHECK_EXIT_SUCCESS(ExerciseBasicMRMLMethods(node));

  CHECK_INT(node->GetNumberOfStorageNodes(), 0);

  node->SetAndObserveStorageNodeID("noid");
  node->AddAndObserveStorageNodeID("badid");
  node->SetAndObserveNthStorageNodeID(2, "nothing");

  node->SetSlicerDataType("testing");
  CHECK_STRING(node->GetSlicerDataType(), "testing");

  CHECK_STRING(node->GetNthStorageNodeID(0), "noid");
  CHECK_NULL(node->GetNthStorageNode(0));

  vtkMRMLStorageNode* defaultStorageNode = node->CreateDefaultStorageNode();
  if (defaultStorageNode)
    {
    std::cout << "Default storage node created" << std::endl;
    defaultStorageNode->Delete();
    }

  CHECK_NOT_NULL(node->GetUserTagTable());

  return EXIT_SUCCESS;
  }

// ----------------------------------------------------------------------------
int ExerciseBasicTransformableMRMLMethods(vtkMRMLTransformableNode* node)
{
  CHECK_EXIT_SUCCESS(ExerciseBasicStorableMRMLMethods(node));

  CHECK_NULL(node->GetParentTransformNode());

  node->SetAndObserveTransformNodeID(nullptr);
  CHECK_NULL(node->GetTransformNodeID());

  bool canApplyNonLinear = node->CanApplyNonLinearTransforms();
  std::cout << "Node can apply non linear transforms? " << (canApplyNonLinear == true ? "yes" : "no") << std::endl;
  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------------
int ExerciseBasicDisplayableMRMLMethods(vtkMRMLDisplayableNode* node)
  {
  CHECK_EXIT_SUCCESS(ExerciseBasicStorableMRMLMethods(node));

  CHECK_INT(node->GetNumberOfDisplayNodes(), 0);

  node->SetAndObserveDisplayNodeID("noid");
  node->AddAndObserveDisplayNodeID("badid");
  node->SetAndObserveNthDisplayNodeID(2, "nothing");
  CHECK_STRING(node->GetNthDisplayNodeID(0), "noid");
  CHECK_NULL(node->GetNthDisplayNode(0));

  return EXIT_SUCCESS;
  }

// ----------------------------------------------------------------------------
int ExerciseBasicDisplayMRMLMethods(vtkMRMLDisplayNode* node)
  {
  CHECK_EXIT_SUCCESS(ExerciseBasicMRMLMethods(node));

  CHECK_NULL(node->GetDisplayableNode());
  node->SetTextureImageDataConnection(nullptr);
  CHECK_NULL(node->GetTextureImageDataConnection());

  node->SetAndObserveColorNodeID(nullptr);
  CHECK_NULL(node->GetColorNodeID());
  CHECK_NULL(node->GetColorNode());

  node->SetActiveScalarName("testingScalar");
  CHECK_STRING(node->GetActiveScalarName(), "testingScalar");

  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node, Color, 0.0, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node, SelectedColor, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, SelectedAmbient, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, SelectedSpecular, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, Opacity, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, Ambient, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, Diffuse, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, Specular, 0.0, 1.0);
  TEST_SET_GET_DOUBLE_RANGE(node, Power, 0.0, 1.0);
  TEST_SET_GET_BOOLEAN(node, Visibility);
  TEST_SET_GET_BOOLEAN(node, Visibility2D);
  TEST_SET_GET_BOOLEAN(node, Visibility3D);
  TEST_SET_GET_BOOLEAN(node, Clipping);
  TEST_SET_GET_INT_RANGE(node, SliceIntersectionThickness, 0, 10);
  TEST_SET_GET_BOOLEAN(node, BackfaceCulling);
  TEST_SET_GET_BOOLEAN(node, ScalarVisibility);
  TEST_SET_GET_BOOLEAN(node, VectorVisibility);
  TEST_SET_GET_BOOLEAN(node, TensorVisibility);
  TEST_SET_GET_BOOLEAN(node, AutoScalarRange);
  double expectedRange[2] = {-10, 10};
  node->SetScalarRange(expectedRange);
  double *scalarRange = node->GetScalarRange();
  CHECK_NOT_NULL(scalarRange);
  if (scalarRange[0] != expectedRange[0] || scalarRange[1] != expectedRange[1])
    {
    std::cerr << __LINE__ << " ERROR getting scalar range" << std::endl;
    return EXIT_FAILURE;
    }

  const char *red = "vtkMRMLSliceNodeRed";
  const char *green = "vtkMRMLSliceNodeGreen";
  const char *yellow = "vtkMRMLSliceNodeYellow";
  const char *threeD = "vtkMRMLViewNode1";
  CHECK_INT(node->GetNumberOfViewNodeIDs(), 0);
  CHECK_BOOL(node->IsDisplayableInView(red), true);
  CHECK_BOOL(node->IsDisplayableInView(green), true);
  CHECK_BOOL(node->IsDisplayableInView(yellow), true);
  CHECK_BOOL(node->IsDisplayableInView(threeD), true);

  node->AddViewNodeID(green);
  CHECK_BOOL(node->IsDisplayableInView(red), false);
  CHECK_BOOL(node->IsDisplayableInView(green), true);
  CHECK_BOOL(node->IsDisplayableInView(yellow), false);
  CHECK_BOOL(node->IsDisplayableInView(threeD), false);

  node->SetDisplayableOnlyInView(red);
  CHECK_BOOL(node->IsDisplayableInView(red), true);
  CHECK_BOOL(node->IsDisplayableInView(green), false);
  CHECK_BOOL(node->IsDisplayableInView(yellow), false);
  CHECK_BOOL(node->IsDisplayableInView(threeD), false);

  return EXIT_SUCCESS;
  }

// ----------------------------------------------------------------------------
int ExerciseBasicStorageMRMLMethods(vtkMRMLStorageNode* node)
  {
  CHECK_EXIT_SUCCESS(ExerciseBasicMRMLMethods(node));

  vtkNew<vtkTest::ErrorObserver> errorWarningObserver;
  int errorObserverTag = node->AddObserver(vtkCommand::WarningEvent, errorWarningObserver.GetPointer());
  int warningObserverTag = node->AddObserver(vtkCommand::ErrorEvent, errorWarningObserver.GetPointer());

  node->ReadData(nullptr);
  CHECK_BOOL(errorWarningObserver->GetError(), true);
  errorWarningObserver->Clear();

  node->WriteData(nullptr);
  CHECK_BOOL(errorWarningObserver->GetError(), true);
  errorWarningObserver->Clear();

  TEST_SET_GET_STRING(node, FileName);
  const char *f0 = node->GetNthFileName(0);
  std::cout << "Filename 0 = " << (f0 == nullptr ? "NULL" : f0) << std::endl;
  TEST_SET_GET_BOOLEAN(node, UseCompression);
  TEST_SET_GET_STRING(node, URI);

  vtkURIHandler *handler = vtkURIHandler::New();
  node->SetURIHandler(nullptr);
  CHECK_NULL(node->GetURIHandler());
  node->SetURIHandler(handler);
  CHECK_NOT_NULL(node->GetURIHandler());
  node->SetURIHandler(nullptr);
  handler->Delete();

  TEST_SET_GET_INT_RANGE(node, ReadState, 0, 5);
  const char *rstate = node->GetReadStateAsString();
  std::cout << "Read state, after int test = " << rstate << std::endl;
  node->SetReadStatePending();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, Pending = " << rstate << std::endl;
  node->SetReadStateIdle();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, Idle = " << rstate << std::endl;
  node->SetReadStateScheduled();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, Scheduled = " << rstate << std::endl;
  node->SetReadStateTransferring();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, Transferring = " << rstate << std::endl;
  node->SetReadStateTransferDone();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, TransfrerDone = " << rstate << std::endl;
  node->SetReadStateCancelled();
  rstate = node->GetReadStateAsString();
  std::cout << "Read state, Cancelled = " << rstate << std::endl;

  TEST_SET_GET_INT_RANGE(node, WriteState, 0, 5);
  const char *wstate = node->GetWriteStateAsString();
  std::cout << "Write state, after int test = " << wstate << std::endl;
  node->SetWriteStatePending();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, Pending = " << wstate << std::endl;
  node->SetWriteStateIdle();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, Idle = " << wstate << std::endl;
  node->SetWriteStateScheduled();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, Scheduled = " << wstate << std::endl;
  node->SetWriteStateTransferring();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, Transferring = " << wstate << std::endl;
  node->SetWriteStateTransferDone();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, TransfrerDone = " << wstate << std::endl;
  node->SetWriteStateCancelled();
  wstate = node->GetWriteStateAsString();
  std::cout << "Write state, Cancelled = " << wstate << std::endl;

  std::string fullName = node->GetFullNameFromFileName();
  std::cout << "fullName = " << fullName.c_str() << std::endl;
  std::string fullName0 = node->GetFullNameFromNthFileName(0);
  std::cout << "fullName0 = " << fullName0.c_str() << std::endl;

  vtkStringArray *types = node->GetSupportedWriteFileTypes();
  std::cout << "Supported write types:" << std::endl;
  for (vtkIdType i = 0; i < types->GetNumberOfValues(); i++)
    {
    std::cout << "\t" << types->GetValue(i).c_str() << std::endl;
    }
  int sup = node->SupportedFileType(nullptr);
  CHECK_BOOL(errorWarningObserver->GetWarning(), true);
  errorWarningObserver->Clear();

  std::cout << "Filename or uri supported? " << sup << std::endl;
  sup = node->SupportedFileType("testing.vtk");
  std::cout << ".vtk supported?  " << sup << std::endl;
  sup = node->SupportedFileType("testing.nrrd");
  std::cout << ".nrrd supported?  " << sup << std::endl;
  sup = node->SupportedFileType("testing.mgz");
  std::cout << ".mgz supported?  " << sup << std::endl;

  TEST_SET_GET_STRING(node, WriteFileFormat);
  node->AddFileName("testing.txt");
  std::cout << "Number of file names = " << node->GetNumberOfFileNames() << std::endl;
  CHECK_BOOL(node->FileNameIsInList("testing.txt"), true);

  node->ResetNthFileName(0, "moretesting.txt");
  node->ResetNthFileName(100, "notinlist.txt");
  node->ResetNthFileName(0, nullptr);
  CHECK_BOOL(node->FileNameIsInList("notinlist"), false);

  node->ResetFileNameList();
  CHECK_INT(node->GetNumberOfFileNames(), 0);

  node->ResetURIList();
  std::cout << "Number of uri's after resetting list = " << node->GetNumberOfURIs() << std::endl;
  node->AddURI("http://www.nowhere.com/filename.txt");
  CHECK_INT(node->GetNumberOfURIs(),1);

  CHECK_STRING(node->GetNthURI(0), "http://www.nowhere.com/filename.txt");

  node->ResetNthURI(0, "http://www.nowhere.com/newfilename.txt");
  node->ResetNthURI(100, "ftp://not.in.list");
  node->ResetNthURI(100, nullptr);
  const char *dataDirName = "/test-ing/a/dir ect.ory";
  node->SetDataDirectory(dataDirName);
  node->SetFileName("/tmp/file.txt");
  node->SetDataDirectory(dataDirName);
  CHECK_STRING(node->GetFileName(), "/test-ing/a/dir ect.ory/file.txt");

  std::cout << "Resetting Data Directory to " << dataDirName << " succeeded, got new file name of " << node->GetFileName() << std::endl;
  const char *uriPrefix = "http://www.somewhere.com/";
  node->SetURIPrefix(uriPrefix);

  const char *defaultExt = node->GetDefaultWriteFileExtension();
  std::cout << "Default write extension = " << (defaultExt == nullptr ? "null" : defaultExt) << std::endl;

  std::cout << "Is null file path relative? " << node->IsFilePathRelative(nullptr) << std::endl;
  std::cout << "Is absolute file path relative? " << node->IsFilePathRelative("/spl/tmp/file.txt") << std::endl;
  std::cout << "Is relative file path relative? " << node->IsFilePathRelative("tmp/file.txt") << std::endl;
  node->RemoveObserver(errorObserverTag);
  node->RemoveObserver(warningObserverTag);
  return EXIT_SUCCESS;
  }

// ----------------------------------------------------------------------------
int ExerciseBasicTransformMRMLMethods(vtkMRMLTransformNode* node)
  {
  CHECK_EXIT_SUCCESS(ExerciseBasicStorableMRMLMethods(node));

  std::cout << "IsLinear = " << node->IsLinear()<< std:: endl;
  CHECK_NOT_NULL(node->GetTransformToParent());
  bool isTransformToWorldLinear = node->IsTransformToWorldLinear();
  std::cout << "IsTransformToWorldLinear = " << isTransformToWorldLinear << std::endl;
  vtkSmartPointer < vtkMRMLTransformNode > t = vtkSmartPointer < vtkMRMLTransformNode >::Take(vtkMRMLTransformNode::SafeDownCast(node->CreateNodeInstance()));
  std::cout << "IsTransformToNodeLinear = " << node->IsTransformToNodeLinear(t) << std::endl;
  vtkSmartPointer<vtkGeneralTransform> g =  vtkSmartPointer<vtkGeneralTransform>::New();
  node->GetTransformToWorld(g);
  node->GetTransformToNode(t, g);
  vtkSmartPointer<vtkMatrix4x4> m =  vtkSmartPointer<vtkMatrix4x4>::New();
  if (!isTransformToWorldLinear)
    {
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    }
  int getMatrixTransformToWorldResult = node->GetMatrixTransformToWorld(m);
  if (!isTransformToWorldLinear)
    {
    TESTING_OUTPUT_ASSERT_ERRORS_END();
    }
  else
    {
    CHECK_BOOL(getMatrixTransformToWorldResult!=0, true);
    }
  if (!isTransformToWorldLinear)
    {
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    }
  int getMatrixTransformToNodeResult = node->GetMatrixTransformToNode(t, m);
  if (!isTransformToWorldLinear)
    {
    TESTING_OUTPUT_ASSERT_ERRORS_END();
    }
  else
    {
    CHECK_BOOL(getMatrixTransformToNodeResult!=0, true);
    }
  std::cout << "IsTransformNodeMyParent = " << node->IsTransformNodeMyParent(t) << std::endl;
  std::cout << "IsTransformNodeMyChild = " << node->IsTransformNodeMyChild(t) << std::endl;
  return EXIT_SUCCESS;
  }

// ----------------------------------------------------------------------------
int ExerciseAllBasicMRMLMethods(vtkMRMLNode* node)
{
  if (vtkMRMLDisplayNode::SafeDownCast(node))
    {
    return ExerciseBasicDisplayMRMLMethods(vtkMRMLDisplayNode::SafeDownCast(node));
    }
  if (vtkMRMLStorageNode::SafeDownCast(node))
    {
    return ExerciseBasicStorageMRMLMethods(vtkMRMLStorageNode::SafeDownCast(node));
    }

  // Go from specific to general types, test the most specific interface
  if (vtkMRMLTransformNode::SafeDownCast(node))
    {
    return ExerciseBasicTransformMRMLMethods(vtkMRMLTransformNode::SafeDownCast(node));
    }
  if (vtkMRMLDisplayableNode::SafeDownCast(node))
    {
    return ExerciseBasicDisplayableMRMLMethods(vtkMRMLDisplayableNode::SafeDownCast(node));
    }
  if (vtkMRMLTransformableNode::SafeDownCast(node))
    {
    return ExerciseBasicTransformableMRMLMethods(vtkMRMLTransformableNode::SafeDownCast(node));
    }
  if (vtkMRMLStorableNode::SafeDownCast(node))
    {
    return ExerciseBasicStorableMRMLMethods(vtkMRMLStorableNode::SafeDownCast(node));
    }
  return ExerciseBasicMRMLMethods(node);
}

// ----------------------------------------------------------------------------
int ExerciseSceneLoadingMethods(const char * sceneFilePath, vtkMRMLScene* inputScene /* = nullptr */)
{
  vtkSmartPointer<vtkMRMLScene> scene;
  if (inputScene)
    {
    scene = inputScene;
    }
  else
    {
      scene = vtkSmartPointer<vtkMRMLScene>::New();
    }

  // Add default slice orientation presets
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(scene);

  scene->SetURL(sceneFilePath);
  scene->Connect();
  int numberOfNodes = scene->GetNumberOfNodes();
  scene->Connect();
  int numberOfNodesAfterReconnect = scene->GetNumberOfNodes();
  CHECK_INT(numberOfNodes, numberOfNodesAfterReconnect);

  scene->Import();
  int numberOfNodesAfterImport = scene->GetNumberOfNodes();
  CHECK_BOOL(numberOfNodesAfterImport>numberOfNodes, true);

  scene->Import();

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
vtkMRMLNodeCallback::vtkMRMLNodeCallback()
{
  this->ResetNumberOfEvents();
}

//---------------------------------------------------------------------------
vtkMRMLNodeCallback::~vtkMRMLNodeCallback() = default;

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::ResetNumberOfEvents()
{
  this->ReceivedEvents.clear();
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::SetMRMLNode(vtkMRMLNode* node)
{
  this->Node = node;
}

//---------------------------------------------------------------------------
std::string vtkMRMLNodeCallback::GetErrorString()
{
  return this->ErrorString;
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::SetErrorString(const char* error)
{
  this->ErrorString = std::string(error?error:"");
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::SetErrorString(int line, const char* error)
{
  std::stringstream ss;
  ss << "line " << line << " - " << (error?error:"(undefined)");
  this->ErrorString = ss.str();
}

//---------------------------------------------------------------------------
int vtkMRMLNodeCallback::GetNumberOfModified()
{
  return this->GetNumberOfEvents(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
int vtkMRMLNodeCallback::GetNumberOfEvents(unsigned long event)
{
  return this->ReceivedEvents[event];
}

//---------------------------------------------------------------------------
int vtkMRMLNodeCallback::GetTotalNumberOfEvents()
{
  int eventsCount = 0;
  for (std::map<unsigned long, unsigned int>::const_iterator it = this->ReceivedEvents.begin(),
       end = this->ReceivedEvents.end(); it != end; ++it)
    {
    eventsCount += it->second;
    }
  return eventsCount;
}

//---------------------------------------------------------------------------
std::vector<unsigned long> vtkMRMLNodeCallback::GetReceivedEvents()
{
  std::vector<unsigned long> receivedEvents;
  for(std::map<unsigned long,unsigned int>::iterator it = this->ReceivedEvents.begin();
      it != this->ReceivedEvents.end();
      ++it)
    {
    unsigned long event = it->first;
    if (this->GetNumberOfEvents(event) > 0)
      {
      receivedEvents.push_back(event);
      }
    }
  return receivedEvents;
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkCallbackCommand::PrintSelf(os,indent);

  os << indent << "ErrorString: " << this->GetErrorString() << "\n";
  os << indent << "TotalNumberOfEvents: " << this->GetTotalNumberOfEvents() << "\n";
  os << indent << "NumberOfModified: " << this->GetNumberOfModified() << "\n";
  std::vector<unsigned long> receivedEvent = this->GetReceivedEvents();
  os << indent << "ReceivedEvents: \n";
  for(std::vector<unsigned long>::iterator it = receivedEvent.begin();
      it != receivedEvent.end();
      ++it)
    {
    os << indent.GetNextIndent() << *it << " \n";
    }
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::Execute(vtkObject *vtkcaller,
  unsigned long eid, void *vtkNotUsed(calldata))
{
  // Let's return if an error already occurred
  if (this->ErrorString.size() > 0)
    {
    return;
    }
  if (this->Node &&
      this->Node != vtkMRMLNode::SafeDownCast(vtkcaller))
    {
    this->SetErrorString("vtkMRMLNodeCallback::Execute - node != vtkcaller");
    return;
    }

  ++this->ReceivedEvents[eid];
}

//---------------------------------------------------------------------------
int vtkMRMLNodeCallback::CheckStatus()
{
  if (!this->ErrorString.empty())
    {
    std::cerr << this->ErrorString << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

} // namespace vtkMRMLCoreTestingUtilities
