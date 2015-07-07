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


#include "vtkMRMLCoreTestingMacros.h"

int qSlicerCoreIOManagerTest1(int argc, char * argv [])
{
  if(argc < 2)
    {
    std::cerr << "Missing arguments" << std::endl;
    return EXIT_FAILURE;
    }

  // make the core application so that the manager can be instantiated
  qSlicerCoreApplication app(argc, argv);

  qSlicerCoreIOManager manager;

  vtkNew<vtkMRMLScene> scene1;

  QString filename = argv[1];

  //manager.loadScene( scene1.GetPointer(), filename );

  vtkNew<vtkMRMLScene> scene2;

  //manager.loadScene( scene2.GetPointer(), filename );

  //manager.closeScene( scene1.GetPointer() );

  QString extension = "mrml";

  //QString fileType = manager.fileTypeFromExtension( extension );

  std::cout << "File Type from extension " << qPrintable(extension);
  //std::cout << " is " << qPrintable(fileType) << std::endl;

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
                << "model.1.vtk" << "color.table.txt.ctbl";
  QStringList expectedExtensions;
  // thisisafailurecase is the default Qt completeSuffix since it doesn't match any
  // known Slicer ext, same with no suffix, and the vtp.gz one
  expectedExtensions << ".nrrd" << ".nii.gz" << ".nii.gz"
                     << ".thisisafailurecase" << "." << ".vtp.gz"
                     << ".vtk" << ".ctbl";

  for (int i = 0; i < testFileNames.size(); ++i)
    {
    QString ext = manager.completeSlicerWritableFileNameSuffix(testFileNames[i]);
    if (expectedExtensions[i] != ext)
      {
      qWarning() << "Failed on file " << testFileNames[i]
                 << ", expected extension " << expectedExtensions[i]
                 << ", but got " << ext;
      return EXIT_FAILURE;
      }
    qDebug() << "Found extension " << ext << " from file " << testFileNames[i];
    }
  return EXIT_SUCCESS;
}
