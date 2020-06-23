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
#include "qMRMLColorTableView.h"

// MRMLLogic includes
#include <vtkMRMLColorLogic.h>

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLPETProceduralColorNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include "qMRMLWidget.h"

int qMRMLColorTableViewTest1(int argc, char * argv [])
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QWidget topLevel;
  qMRMLColorTableView ColorTableView;
  qMRMLColorTableView ColorTableView1;
  qMRMLColorTableView ColorTableView2;

  QHBoxLayout* hboxLayout = new QHBoxLayout;
  hboxLayout->addWidget(&ColorTableView);
  hboxLayout->addWidget(&ColorTableView1);
  hboxLayout->addWidget(&ColorTableView2);
  topLevel.setLayout(hboxLayout);

  vtkNew<vtkMRMLColorTableNode> colorTableNode;
  colorTableNode->SetType(vtkMRMLColorTableNode::Labels);

  ColorTableView.setMRMLColorNode(colorTableNode.GetPointer());
  if (ColorTableView.mrmlColorNode() != colorTableNode.GetPointer())
    {
    std::cerr << "qMRMLColorTableView::setMRMLColorNode() failed" << std::endl;
    return EXIT_FAILURE;
    }
  // for some reasons it generate a warning if the type is changed.
  colorTableNode->NamesInitialisedOff();
  colorTableNode->SetTypeToCool1();

  vtkNew<vtkMRMLPETProceduralColorNode> colorPETNode;
  colorPETNode->SetTypeToRainbow();
  ColorTableView2.setMRMLColorNode(colorPETNode.GetPointer());
  if (ColorTableView2.mrmlColorNode() != colorPETNode.GetPointer())
    {
    std::cerr << "qMRMLColorTableView::setMRMLColorNode() failed" << std::endl;
    return EXIT_FAILURE;
    }
  colorPETNode->SetTypeToMIP();

  topLevel.show();

  vtkSmartPointer<vtkMRMLColorTableNode> userNode
    = vtkSmartPointer<vtkMRMLColorTableNode>::Take(
      vtkMRMLColorLogic::CopyNode(colorTableNode.GetPointer(), "User"));


  qMRMLColorTableView colorTableView;
  colorTableView.setWindowTitle("Editable");
  colorTableView.setMRMLColorNode(userNode);
  colorTableView.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

