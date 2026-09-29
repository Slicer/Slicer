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

  This file was originally developed by Luis Ibanez, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/
// Qt includes
#include <QDebug>
#include <QImage>
#include <QStringList>
#include <QUrl>

// Qt Core includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerFileReader.h"

// MRML includes
#include <vtkMRMLIOProperties.h>
#include <vtkMRMLMessageCollection.h>
#include "vtkMRMLTextNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"

#include "vtkMRMLCoreTestingMacros.h"

// STD includes
#include <iostream>

namespace
{
/// Legacy Qt-based reader (without Q_OBJECT macro)
class qSlicerSharedTestFileReader : public qSlicerFileReader
{
public:
  qSlicerSharedTestFileReader(QObject* parent = nullptr)
    : qSlicerFileReader(parent)
  {
  }
  QString description() const override { return "Shared test reader"; }
  IOFileType fileType() const override { return "SharedTestFile"; }
  QStringList extensions() const override { return QStringList() << "Shared test (*.sharedtest)"; }
  bool load(const IOProperties& vtkNotUsed(properties)) override { return true; }
};
} // namespace

//-----------------------------------------------------------------------------
int TestSharedQtReaders()
{
  // All IO manager instances share the application logic's file IO manager.
  // The original Qt-based reader must be returned by all instances (not a generic wrapper).
  qSlicerCoreIOManager otherManager;
  {
    qSlicerCoreIOManager registeringManager;
    qSlicerSharedTestFileReader* reader = new qSlicerSharedTestFileReader;
    registeringManager.registerIO(reader);
    CHECK_POINTER(registeringManager.reader("Shared test reader"), reader);
    CHECK_POINTER(otherManager.reader("Shared test reader"), reader);
  }
  // The reader is deleted with the manager that registered it
  CHECK_NULL(otherManager.reader("Shared test reader"));
  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int TestPropertiesConversion()
{
  // Conversion of IO properties between Qt and VTK must preserve values and types
  QImage image(4, 3, QImage::Format_RGBA8888);
  image.fill(Qt::red);
  QVariantMap nestedMap;
  nestedMap["path"] = QStringList() << "a" << "b";
  nestedMap["count"] = 3;
  QVariantHash nestedHash;
  nestedHash["value"] = 2.5;
  qSlicerIO::IOProperties properties;
  properties["fileName"] = QString("/data/Head.nrrd");
  properties["labelmap"] = true;
  properties["count"] = 7;
  properties["large"] = qlonglong(5000000000LL);
  properties["spacing"] = 0.5;
  properties["fileNames"] = QStringList() << "a.png" << "b.png";
  properties["list"] = QVariantList() << QString("text") << 12 << 2.5 << true;
  properties["map"] = nestedMap;
  properties["hash"] = nestedHash;
  properties["screenShot"] = image;
  properties["unsupported"] = QUrl("http://slicer.org"); // unsupported type, an error is logged

  vtkNew<vtkMRMLIOProperties> vtkProperties;
  qSlicerIO::toVTKProperties(properties, vtkProperties);
  CHECK_BOOL(vtkProperties->HasProperty("unsupported"), false);
  CHECK_BOOL(vtkProperties->IsListProperty("list"), true);
  CHECK_BOOL(vtkProperties->IsMapProperty("map"), true);
  CHECK_BOOL(vtkProperties->IsMapProperty("hash"), true);

  qSlicerIO::IOProperties result = qSlicerIO::fromVTKProperties(vtkProperties);
  CHECK_BOOL(result["fileName"].toString() == "/data/Head.nrrd", true);
  CHECK_INT(result["labelmap"].userType(), QMetaType::Bool);
  CHECK_BOOL(result["labelmap"].toBool(), true);
  CHECK_INT(result["count"].toInt(), 7);
  CHECK_BOOL(result["large"].toLongLong() == 5000000000LL, true);
  CHECK_DOUBLE(result["spacing"].toDouble(), 0.5);
  CHECK_INT(result["fileNames"].userType(), QMetaType::QStringList);
  CHECK_INT(result["fileNames"].toStringList().size(), 2);
  CHECK_INT(result["list"].userType(), QMetaType::QVariantList);
  QVariantList list = result["list"].toList();
  CHECK_INT(list.size(), 4);
  CHECK_INT(list[0].userType(), QMetaType::QString);
  CHECK_INT(list[1].toInt(), 12);
  CHECK_DOUBLE(list[2].toDouble(), 2.5);
  CHECK_INT(list[3].userType(), QMetaType::Bool);
  CHECK_INT(result["map"].userType(), QMetaType::QVariantMap);
  QVariantMap resultMap = result["map"].toMap();
  CHECK_INT(resultMap["path"].toStringList().size(), 2);
  CHECK_INT(resultMap["count"].toInt(), 3);
  CHECK_DOUBLE(result["hash"].toMap()["value"].toDouble(), 2.5);
  CHECK_INT(result["screenShot"].userType(), QMetaType::QImage);
  CHECK_INT(result["screenShot"].value<QImage>().width(), 4);
  CHECK_BOOL(result.contains("unsupported"), false);
  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int TestLongNodeNameSaving(const char* temporaryDirectory)
{
  vtkNew<vtkMRMLScene> scene;
  if (temporaryDirectory)
  {
    scene->SetRootDirectory(temporaryDirectory);
  }

  std::string longNodeName = "Loremipsumdolorsitametconsecteturadipiscingelitseddoeiusmodtemporin"
                             "cididuntutlaboreetdoloremagnaaliquaUtenimadminimveniamquisnostrudex"
                             "ercitationullamcolaborisnisiutaliquipexeacommodoconsequatDuisauteir"
                             "uredolorinreprehenderitinvoluptatevelitessecillumdoloreeufugiatnull"
                             "apariaturExcepteursintoccaecatcupidatatnonproidentsuntinculpaquioff"
                             "iciadeseruntmollitanimidestlaborum";
  std::string extension = ".txt";
  std::string longFileName = longNodeName + extension;
  std::string safeFileName = qSlicerCoreIOManager::forceFileNameValidCharacters(QString::fromStdString(longFileName)).toStdString();

  qSlicerCoreIOManager ioManager;
  ioManager.setDefaultMaximumFileNameLength(25);
  safeFileName = ioManager.forceFileNameMaxLength(QString::fromStdString(longFileName), extension.length()).toStdString();

  vtkSmartPointer<vtkMRMLTextNode> textNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode", longNodeName));
  textNode->SetText(longNodeName);
  textNode->SetForceCreateStorageNode(true);
  textNode->AddDefaultStorageNode();
  vtkMRMLStorageNode* storageNode = textNode->GetStorageNode();

#ifdef Q_OS_WIN
  std::cout << std::endl << "||||||||||||||||||||" << std::endl;
  std::cout << std::endl << "Testing long file name: " << longFileName << std::endl;
  storageNode->SetFileName(longFileName.c_str());
  CHECK_INT(storageNode->WriteData(textNode), 0); // Writing should fail. File name too long.
#endif

  std::cout << std::endl << "||||||||||||||||||||" << std::endl;
  std::cout << std::endl << "Testing safe file name: " << safeFileName << std::endl;
  storageNode->SetFileName(safeFileName.c_str());
  storageNode->GetUserMessages()->ClearMessages();
  CHECK_INT(storageNode->WriteData(textNode), 1); // Writing should succeed.

  std::cout << std::endl << "||||||||||||||||||||" << std::endl;
  std::cout << std::endl << "Testing scene save with long node name: " << longNodeName << std::endl;
  storageNode->SetFileName("");
  std::stringstream scenePathSS;
  scenePathSS << scene->GetRootDirectory() << "/" << "Loremipsum.mrb";
  std::string scenePath = scenePathSS.str();
  CHECK_BOOL(scene->WriteToMRB(scenePath.c_str()), true); // Scene should automatically shorten filename.

  return EXIT_SUCCESS;
}

int qSlicerCoreIOManagerTest1(int argc, char* argv[])
{
  // make the core application so that the manager can be instantiated
  qSlicerCoreApplication app(argc, argv);

  qSlicerCoreIOManager manager;

  CHECK_EXIT_SUCCESS(TestSharedQtReaders());
  CHECK_EXIT_SUCCESS(TestPropertiesConversion());

  // get all the writable file extensions
  QStringList allWritableExtensions = manager.allWritableFileExtensions();
  if (allWritableExtensions.isEmpty())
  {
    std::cerr << "Failed to get the list of all writable file extensions." << std::endl;
    return EXIT_FAILURE;
  }
  qDebug() << "All writable extensions = ";
  for (const QString& ext : allWritableExtensions)
  {
    qDebug() << ext;
  }

  // get all the readable file extensions
  QStringList allReadableExtensions = manager.allReadableFileExtensions();
  if (allReadableExtensions.isEmpty())
  {
    std::cerr << "Failed to get the list of all readable file extensions." << std::endl;
    return EXIT_FAILURE;
  }
  qDebug() << "All readable extensions = ";
  for (const QString& ext : allReadableExtensions)
  {
    qDebug() << ext;
  }

  // test getting specific writable file extensions
  QStringList testFileNames;
  testFileNames << "MRHead.nrrd" << "brain.nii.gz" << "brain.1.nii.gz"
                << "brain.thisisafailurecase" << "brain" << "model.vtp.gz"
                << "sometransform_version2.0_some.h5"
                << "model.1.vtk" << "color.table.txt.ctbl"
                << "something.seg.nrrd" << "some.more.seg.seg.nrrd" << "some.less.nrrd";
  QStringList storageNodeClassNames;
  storageNodeClassNames << "vtkMRMLScalarVolumeNode" << "vtkMRMLScalarVolumeNode" << "vtkMRMLScalarVolumeNode"
                        << "vtkMRMLScalarVolumeNode" << "vtkMRMLScalarVolumeNode" << "vtkMRMLModelNode"
                        << "vtkMRMLTransformNode"
                        << "vtkMRMLModelNode" << "vtkMRMLColorTableNode"
                        << "vtkMRMLSegmentationNode" << "vtkMRMLSegmentationNode" << "vtkMRMLSegmentationNode";
  QStringList expectedExtensions;
  // thisisafailurecase is the default Qt completeSuffix since it doesn't match any
  // known Slicer ext, same with no suffix, and the vtp.gz one
  expectedExtensions << ".nrrd" << ".nii.gz" << ".nii.gz"
                     << "." << "." << "."
                     << ".h5"
                     << ".vtk" << ".ctbl"
                     << ".seg.nrrd" << ".seg.nrrd" << ".nrrd";

  for (int i = 0; i < testFileNames.size(); ++i)
  {
    vtkSmartPointer<vtkMRMLNode> node = vtkSmartPointer<vtkMRMLNode>::Take(app.mrmlScene()->CreateNodeByClass(storageNodeClassNames[i].toUtf8().constData()));
    app.mrmlScene()->AddNode(node);
    vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(node);
    storableNode->AddDefaultStorageNode(testFileNames[i].toUtf8().constData());
    QString ext = manager.completeSlicerWritableFileNameSuffix(storableNode);
    if (expectedExtensions[i] != ext)
    {
      qWarning() << "Failed on file " << testFileNames[i] << ", expected extension " << expectedExtensions[i] << ", but got " << ext;
      return EXIT_FAILURE;
    }
    qDebug() << "Found extension " << ext << " from file " << testFileNames[i] << " using " << storageNodeClassNames[i];
  }

  const char* temporaryDirectory = nullptr;
  if (argc > 1)
  {
    temporaryDirectory = argv[1];
  }
  else
  {
    temporaryDirectory = app.mrmlScene()->GetRootDirectory();
  }
  CHECK_EXIT_SUCCESS(TestLongNodeNameSaving(temporaryDirectory));

  return EXIT_SUCCESS;
}
