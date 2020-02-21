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
#include <QStringList>

// Qt Core includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLStorableNode.h"

#include "vtkMRMLCoreTestingMacros.h"

int qSlicerCoreIOManagerTest1(int argc, char * argv [])
{
  // make the core application so that the manager can be instantiated
  qSlicerCoreApplication app(argc, argv);

  qSlicerCoreIOManager manager;

  // get all the writable file extensions
  QStringList allWritableExtensions = manager.allWritableFileExtensions();
  if (allWritableExtensions.isEmpty())
    {
    std::cerr << "Failed to get the list of all writable file extensions."
              << std::endl;
    return EXIT_FAILURE;
    }
  qDebug() << "All writable extensions = ";
  foreach (QString ext, allWritableExtensions)
    {
    qDebug() << ext;
    }

  // get all the readable file extensions
  QStringList allReadableExtensions = manager.allReadableFileExtensions();
  if (allReadableExtensions.isEmpty())
    {
    std::cerr << "Failed to get the list of all readable file extensions."
              << std::endl;
    return EXIT_FAILURE;
    }
  qDebug() << "All readable extensions = ";
  foreach (QString ext, allReadableExtensions)
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
      qWarning() << "Failed on file " << testFileNames[i]
                 << ", expected extension " << expectedExtensions[i]
                 << ", but got " << ext;
      return EXIT_FAILURE;
      }
    qDebug() << "Found extension " << ext << " from file " << testFileNames[i] << " using " << storageNodeClassNames[i];
    }

  return EXIT_SUCCESS;
}
