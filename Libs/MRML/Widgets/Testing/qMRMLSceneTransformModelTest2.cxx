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

// Qt includes
#include <QApplication>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLSceneTransformModel.h"
#include "qMRMLSortFilterProxyModel.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

int qMRMLSceneTransformModelTest2(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }

  try
    {
    qMRMLSceneTransformModel model;
    qMRMLSortFilterProxyModel sort;
    sort.setSourceModel(&model);
    vtkMRMLScene* scene = vtkMRMLScene::New();
    vtkNew<vtkMRMLApplicationLogic> applicationLogic;
    applicationLogic->SetMRMLScene(scene);
    model.setMRMLScene(scene);
    scene->SetURL(argv[1]);
    scene->Connect();
    std::cout << std::endl << "Loaded" << std::endl;

    scene->SetURL(argv[1]);
    scene->Connect();
    std::cout << std::endl << "Loaded twice" << std::endl;

    scene->Delete();

    }
  catch (const char* error)
    {
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

