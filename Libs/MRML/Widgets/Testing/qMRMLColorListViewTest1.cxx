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
#include <QHBoxLayout>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLColorListView.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLPETProceduralColorNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes

int qMRMLColorListViewTest1(int argc, char * argv [])
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QWidget topLevel;
  qMRMLColorListView ColorListView;
  qMRMLColorListView ColorListView1;
  qMRMLColorListView ColorListView2;

  QHBoxLayout* hboxLayout = new QHBoxLayout;
  hboxLayout->addWidget(&ColorListView);
  hboxLayout->addWidget(&ColorListView1);
  hboxLayout->addWidget(&ColorListView2);
  topLevel.setLayout(hboxLayout);

  vtkNew<vtkMRMLColorTableNode> colorTableNode;
  colorTableNode->SetType(vtkMRMLColorTableNode::Labels);

  ColorListView.setMRMLColorNode(colorTableNode.GetPointer());
  if (ColorListView.mrmlColorNode() != colorTableNode.GetPointer())
    {
    std::cerr << "qMRMLColorListView::setMRMLColorNode() failed" << std::endl;
    return EXIT_FAILURE;
    }
  // for some reasons it generate a warning if the type is changed.
  colorTableNode->NamesInitialisedOff();
  colorTableNode->SetTypeToCool1();

  vtkNew<vtkMRMLPETProceduralColorNode> colorPETNode;
  colorPETNode->SetTypeToRainbow();
  ColorListView2.setMRMLColorNode(colorPETNode.GetPointer());
  if (ColorListView2.mrmlColorNode() != colorPETNode.GetPointer())
    {
    std::cerr << "qMRMLColorListView::setMRMLColorNode() failed" << std::endl;
    return EXIT_FAILURE;
    }
  colorPETNode->SetTypeToMIP();

  topLevel.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

