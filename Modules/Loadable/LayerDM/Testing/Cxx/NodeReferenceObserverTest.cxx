// LayerDM includes
#include "vtkMRMLLayerDMObjectEventObserver.h"
#include "vtkMRMLLayerDMNodeReferenceObserver.h"

// Slicer includes
#include <vtkMRMLMarkupsFiducialDisplayNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

// CTK includes
#include <ctkTest.h>

namespace
{
struct Spy
{

  void OnRefModifiedCall(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role, int eventType)
  {
    calls.emplace_back(std::make_tuple(fromNode, toNode, role, eventType));
    callCount++;
  }

  bool WasCalled() const { return callCount > 0; }

  void Clear()
  {
    calls.clear();
    callCount = 0;
  }

  using CallT = std::tuple<vtkMRMLNode*, vtkMRMLNode*, std::string, int>;
  CallT GetCall(int iCall) { return calls[iCall]; }

  std::vector<CallT> calls;
  std::vector<CallT> removedCalls;
  int callCount{};
};

struct Test
{
  Test(const vtkSmartPointer<vtkMRMLScene>& inScene = nullptr)
    : scene{ inScene ? inScene : vtkSmartPointer<vtkMRMLScene>::New() }
  {
    obs->SetReferenceModifiedCallBack([this](vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role, int eventType)
                                      { spy.OnRefModifiedCall(fromNode, toNode, role, eventType); });
    obs->SetScene(scene);

    scene->AddNode(markups);
    scene->AddNode(d1);
    scene->AddNode(d2);
    scene->AddNode(d3);
  }

  vtkSmartPointer<vtkMRMLScene> scene;
  vtkNew<vtkMRMLLayerDMNodeReferenceObserver> obs;
  Spy spy;
  vtkNew<vtkMRMLMarkupsFiducialNode> markups;
  vtkNew<vtkMRMLMarkupsFiducialDisplayNode> d1;
  vtkNew<vtkMRMLMarkupsFiducialDisplayNode> d2;
  vtkNew<vtkMRMLMarkupsFiducialDisplayNode> d3;
};
} // namespace

class NodeReferenceObserverTester : public QObject
{
  Q_OBJECT

private slots:
  void testOnNodesAddedToSceneDoesntTriggerRefAddedRemoved() const
  {
    Test test;
    QCOMPARE(test.markups->GetScene(), test.scene);
    QCOMPARE(test.d1->GetScene(), test.scene);
    QCOMPARE(test.d2->GetScene(), test.scene);
    QCOMPARE(test.spy.callCount, 0);
  }

  void testTriggersRefAddedOnRefAdded() const
  {
    Test test;
    test.markups->SetAndObserveDisplayNodeID(test.d1->GetID());
    QCOMPARE(test.spy.callCount, 1);

    auto [fromNode, toNode, role, eventType] = test.spy.GetCall(0);
    QCOMPARE(fromNode, test.markups);
    QCOMPARE(toNode, test.d1);
    QCOMPARE(role, "display");
    QCOMPARE(eventType, vtkMRMLLayerDMNodeReferenceObserver::ReferenceAddedEvent);
  }

  void testTriggersRefRemovedOnRefRemoved() const
  {
    Test test;
    test.markups->SetAndObserveDisplayNodeID(test.d1->GetID());
    test.spy.Clear();

    test.markups->SetAndObserveDisplayNodeID("");
    QCOMPARE(test.spy.callCount, 1);

    auto [fromNode, toNode, role, eventType] = test.spy.GetCall(0);
    QCOMPARE(fromNode, test.markups);
    QCOMPARE(toNode, test.d1);
    QCOMPARE(role, "display");
    QCOMPARE(eventType, vtkMRMLLayerDMNodeReferenceObserver::ReferenceRemovedEvent);
  }

  void testTriggersAddedWhenAddingAndNotReplacingExistingRef() const
  {
    Test test;
    test.markups->SetAndObserveDisplayNodeID(test.d1->GetID());
    test.markups->AddAndObserveDisplayNodeID(test.d2->GetID());
    QCOMPARE(test.spy.callCount, 2);

    auto [fromNode, toNode, role, eventType] = test.spy.GetCall(0);
    QCOMPARE(fromNode, test.markups);
    QCOMPARE(toNode, test.d1);
    QCOMPARE(role, "display");
    QCOMPARE(eventType, vtkMRMLLayerDMNodeReferenceObserver::ReferenceAddedEvent);

    auto [fromNode2, toNode2, role2, eventType2] = test.spy.GetCall(1);
    QCOMPARE(fromNode2, test.markups);
    QCOMPARE(toNode2, test.d2);
    QCOMPARE(role2, "display");
    QCOMPARE(eventType2, vtkMRMLLayerDMNodeReferenceObserver::ReferenceAddedEvent);
  }

  void testTriggersRemovedAndAddedOnReferenceModified() const
  {
    Test test;
    test.markups->SetAndObserveDisplayNodeID(test.d1->GetID());
    test.markups->AddAndObserveDisplayNodeID(test.d2->GetID());
    test.spy.Clear();

    test.markups->SetAndObserveNthDisplayNodeID(0, test.d3->GetID());
    QCOMPARE(test.spy.callCount, 2);

    auto [fromNode, toNode, role, eventType] = test.spy.GetCall(0);
    QCOMPARE(fromNode, test.markups);
    QCOMPARE(toNode, test.d1);
    QCOMPARE(role, "display");
    QCOMPARE(eventType, vtkMRMLLayerDMNodeReferenceObserver::ReferenceRemovedEvent);

    auto [fromNode3, toNode3, role3, eventType3] = test.spy.GetCall(1);
    QCOMPARE(fromNode3, test.markups);
    QCOMPARE(toNode3, test.d3);
    QCOMPARE(role3, "display");
    QCOMPARE(eventType3, vtkMRMLLayerDMNodeReferenceObserver::ReferenceAddedEvent);
  }

  void testTriggersRemovedOnToNodeRemovedFromScene() const
  {
    Test test;
    test.markups->SetAndObserveDisplayNodeID(test.d1->GetID());
    test.spy.Clear();

    test.scene->RemoveNode(test.d1);
    QCOMPARE(test.spy.callCount, 1);
    auto [fromNode, toNode, role, eventType] = test.spy.GetCall(0);
    QCOMPARE(fromNode, test.markups);
    QCOMPARE(toNode, test.d1);
    QCOMPARE(role, "display");
    QCOMPARE(eventType, vtkMRMLLayerDMNodeReferenceObserver::ReferenceRemovedEvent);
  }

  void testTriggersRemovedOnFromNodeRemovedFromScene() const
  {
    Test test;
    test.markups->SetAndObserveDisplayNodeID(test.d1->GetID());
    test.spy.Clear();

    test.scene->RemoveNode(test.markups);
    QCOMPARE(test.spy.callCount, 1);
    auto [fromNode, toNode, role, eventType] = test.spy.GetCall(0);
    QCOMPARE(fromNode, test.markups);
    QCOMPARE(toNode, test.d1);
    QCOMPARE(role, "display");
    QCOMPARE(eventType, vtkMRMLLayerDMNodeReferenceObserver::ReferenceRemovedEvent);
  }

  void testTriggersRemovedOnSceneClear() const
  {
    Test test;
    test.markups->SetAndObserveDisplayNodeID(test.d1->GetID());
    test.spy.Clear();

    test.scene->Clear();
    QCOMPARE(test.spy.callCount, 1);
    auto [fromNode, toNode, role, eventType] = test.spy.GetCall(0);
    QCOMPARE(fromNode, test.markups);
    QCOMPARE(toNode, test.d1);
    QCOMPARE(role, "display");
    QCOMPARE(eventType, vtkMRMLLayerDMNodeReferenceObserver::ReferenceRemovedEvent);
  }

  void testTriggersAddedWhenSettingAlreadyExistingScene() const
  {
    auto scene = vtkSmartPointer<vtkMRMLScene>::New();
    auto markups = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->AddNode(vtkNew<vtkMRMLMarkupsFiducialNode>{}));
    markups->CreateDefaultDisplayNodes();

    auto d1 = vtkMRMLMarkupsFiducialDisplayNode::SafeDownCast(markups->GetDisplayNode());
    QVERIFY(d1);

    Test test(scene);
    QCOMPARE(test.spy.callCount, 1);
    auto [fromNode, toNode, role, eventType] = test.spy.GetCall(0);
    QCOMPARE(fromNode, markups);
    QCOMPARE(toNode, d1);
    QCOMPARE(role, "display");
    QCOMPARE(eventType, vtkMRMLLayerDMNodeReferenceObserver::ReferenceAddedEvent);
  }

  void testNodesRemovedFromSceneAreCorrectlyRemovedFromObserver()
  {
    auto scene = vtkSmartPointer<vtkMRMLScene>::New();
    auto markups = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->AddNode(vtkNew<vtkMRMLMarkupsFiducialNode>{}));
    markups->CreateDefaultDisplayNodes();

    // Verify that the number of nodes in the observer are consistent with the number of nodes present in the scene
    Test test(scene);
    int nNodes = test.obs->GetNumberOfNodes();
    QCOMPARE(scene->GetNumberOfNodes(), nNodes);

    int nRefTo = test.obs->GetReferenceToSize();
    QVERIFY(nRefTo <= nNodes);

    int nRefFrom = test.obs->GetReferenceFromSize();
    QVERIFY(test.obs->GetReferenceFromSize() <= nNodes);

    // Verify that the references from / to the created markups is consistent with the current display node
    const auto refTo = test.obs->GetNodeToReferences(markups);
    QVERIFY(refTo.find({ markups->GetDisplayNode(), "display" }) != refTo.end());

    const auto refFrom = test.obs->GetNodeFromReferences(markups->GetDisplayNode());
    QVERIFY(refFrom.find({ markups, "display" }) != refFrom.end());

    // Remove the markups and its display node from the scene
    scene->RemoveNode(markups->GetDisplayNode());
    scene->RemoveNode(markups);

    // Expect the number of nodes tracked to have been reduced by 2
    QCOMPARE(scene->GetNumberOfNodes(), nNodes - 2);
    QCOMPARE(test.obs->GetNumberOfNodes(), nNodes - 2);
  }
};

CTK_TEST_MAIN(NodeReferenceObserverTest)

#include "NodeReferenceObserverTest.moc"
