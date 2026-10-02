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

==============================================================================*/

// Qt includes
#include <QStringList>
#include <QTemporaryDir>

// CTK includes
#include <ctkTest.h>

// Slicer includes
#include <qMRMLWidget.h>
#include <qSlicerApplication.h>

// Segmentations includes
#include "qMRMLSegmentsTableView.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLSegmentationStorageNode.h>

// Segmentation core includes
#include <vtkBinaryLabelmapToClosedSurfaceConversionRule.h>
#include <vtkClosedSurfaceToBinaryLabelmapConversionRule.h>
#include <vtkSegmentation.h>
#include <vtkSegmentationConverterFactory.h>

// VTK includes
#include <vtkNew.h>

namespace
{
//-----------------------------------------------------------------------------
QString addSegment(vtkMRMLSegmentationNode* segmentationNode, const char* name, double red, double green, double blue)
{
  double color[3] = { red, green, blue };
  return QString::fromStdString(segmentationNode->GetSegmentation()->AddEmptySegment("", name, color));
}

//-----------------------------------------------------------------------------
QStringList segmentNamesInOrder(vtkMRMLSegmentationNode* segmentationNode)
{
  QStringList names;
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  for (int segmentIndex = 0; segmentIndex < segmentation->GetNumberOfSegments(); ++segmentIndex)
  {
    names << segmentation->GetNthSegment(segmentIndex)->GetName();
  }
  return names;
}
} // namespace

//-----------------------------------------------------------------------------
class qMRMLSegmentsTableViewTester : public QObject
{
  Q_OBJECT

private slots:
  void testMoveSegmentsToRow();
  void testMoveSelectedSegmentsToRowSaveReload();
};

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableViewTester::testMoveSegmentsToRow()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSegmentationNode> segmentationNode;
  scene->AddNode(segmentationNode);
  segmentationNode->CreateDefaultDisplayNodes();

  QString firstSegmentID = addSegment(segmentationNode, "first", 1.0, 0.0, 0.0);
  QString secondSegmentID = addSegment(segmentationNode, "second", 0.0, 1.0, 0.0);
  QString thirdSegmentID = addSegment(segmentationNode, "third", 0.0, 0.0, 1.0);

  qMRMLSegmentsTableView segmentsTableView;
  segmentsTableView.setMRMLScene(scene);
  segmentsTableView.setSegmentationNode(segmentationNode);

  QCOMPARE(segmentNamesInOrder(segmentationNode), QStringList({ "first", "second", "third" }));

  QVERIFY(segmentsTableView.moveSegmentsToRow(QStringList({ thirdSegmentID }), 0));
  QCOMPARE(segmentNamesInOrder(segmentationNode), QStringList({ "third", "first", "second" }));

  QVERIFY(segmentsTableView.moveSegmentsToRow(QStringList({ firstSegmentID, secondSegmentID }), 0));
  QCOMPARE(segmentNamesInOrder(segmentationNode), QStringList({ "first", "second", "third" }));

  QVERIFY(segmentsTableView.moveSegmentsToRow(QStringList({ firstSegmentID, thirdSegmentID }), 2));
  QCOMPARE(segmentNamesInOrder(segmentationNode), QStringList({ "second", "first", "third" }));

  QVERIFY(segmentsTableView.moveSegmentsToRow(QStringList({ secondSegmentID }), 2));
  QCOMPARE(segmentNamesInOrder(segmentationNode), QStringList({ "first", "second", "third" }));

  QVERIFY(!segmentsTableView.moveSegmentsToRow(QStringList({ firstSegmentID, secondSegmentID }), 0));
  QCOMPARE(segmentNamesInOrder(segmentationNode), QStringList({ "first", "second", "third" }));
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableViewTester::testMoveSelectedSegmentsToRowSaveReload()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSegmentationNode> segmentationNode;
  scene->AddNode(segmentationNode);
  segmentationNode->CreateDefaultDisplayNodes();

  QString firstSegmentID = addSegment(segmentationNode, "first", 1.0, 0.0, 0.0);
  QString secondSegmentID = addSegment(segmentationNode, "second", 0.0, 1.0, 0.0);
  addSegment(segmentationNode, "third", 0.0, 0.0, 1.0);

  qMRMLSegmentsTableView segmentsTableView;
  segmentsTableView.setMRMLScene(scene);
  segmentsTableView.setSegmentationNode(segmentationNode);
  segmentsTableView.setSelectedSegmentIDs(QStringList({ firstSegmentID, secondSegmentID }));

  QCOMPARE(segmentNamesInOrder(segmentationNode), QStringList({ "first", "second", "third" }));
  QVERIFY(segmentsTableView.moveSelectedSegmentsToRow(segmentationNode->GetSegmentation()->GetNumberOfSegments()));
  QCOMPARE(segmentNamesInOrder(segmentationNode), QStringList({ "third", "first", "second" }));

  QTemporaryDir tempDir;
  QVERIFY(tempDir.isValid());
  QString segmentationFilePath = tempDir.filePath("reordered.seg.nrrd");

  vtkNew<vtkMRMLSegmentationStorageNode> storageNode;
  scene->AddNode(storageNode);
  segmentationNode->SetAndObserveStorageNodeID(storageNode->GetID());
  storageNode->SetFileName(segmentationFilePath.toUtf8().constData());
  QVERIFY(storageNode->WriteData(segmentationNode));

  vtkNew<vtkMRMLScene> reloadedScene;
  vtkNew<vtkMRMLSegmentationNode> reloadedSegmentationNode;
  reloadedScene->AddNode(reloadedSegmentationNode);
  reloadedSegmentationNode->CreateDefaultDisplayNodes();

  vtkNew<vtkMRMLSegmentationStorageNode> reloadedStorageNode;
  reloadedScene->AddNode(reloadedStorageNode);
  reloadedSegmentationNode->SetAndObserveStorageNodeID(reloadedStorageNode->GetID());
  reloadedStorageNode->SetFileName(segmentationFilePath.toUtf8().constData());
  QVERIFY(reloadedStorageNode->ReadData(reloadedSegmentationNode));

  QCOMPARE(segmentNamesInOrder(reloadedSegmentationNode), QStringList({ "third", "first", "second" }));
}

//-----------------------------------------------------------------------------
int qMRMLSegmentsTableViewTest(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  qSlicerApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkSegmentationConverterFactory* converterFactory = vtkSegmentationConverterFactory::GetInstance();
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkClosedSurfaceToBinaryLabelmapConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New());

  qMRMLSegmentsTableViewTester tc;
  return QTest::qExec(&tc, argc, argv);
}

#include "qMRMLSegmentsTableViewTest.moc"
