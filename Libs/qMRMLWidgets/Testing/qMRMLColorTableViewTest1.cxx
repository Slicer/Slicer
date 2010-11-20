/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QT includes
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QTimer>

// qMRML includes
#include "qMRMLColorTableView.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLFreeSurferProceduralColorNode.h>
#include <vtkMRMLPETProceduralColorNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// STD includes
#include <cstdlib>
#include <iostream>

int qMRMLColorTableViewTest1(int argc, char * argv [])
{
  QApplication app(argc, argv);

  QWidget topLevel;
  qMRMLColorTableView ColorTableView;
  qMRMLColorTableView ColorTableView1;
  qMRMLColorTableView ColorTableView2;

  QHBoxLayout* hboxLayout = new QHBoxLayout;
  hboxLayout->addWidget(&ColorTableView);
  hboxLayout->addWidget(&ColorTableView1);
  hboxLayout->addWidget(&ColorTableView2);
  topLevel.setLayout(hboxLayout);

  vtkSmartPointer<vtkMRMLColorTableNode> colorTableNode =
    vtkSmartPointer<vtkMRMLColorTableNode>::New();
  colorTableNode->SetType(vtkMRMLColorTableNode::Labels);
  
  ColorTableView.setMRMLColorNode(colorTableNode);
  if (ColorTableView.mrmlColorNode() != colorTableNode.GetPointer())
    {
    std::cerr << "qMRMLColorTableView::setMRMLColorNode() failed" << std::endl;
    return EXIT_FAILURE;
    }
  // for some reasons it generate a warning if the type is changed.
  colorTableNode->NamesInitialisedOff();
  colorTableNode->SetTypeToCool1();
  
  vtkSmartPointer<vtkMRMLFreeSurferProceduralColorNode> colorFreeSurferNode =
    vtkSmartPointer<vtkMRMLFreeSurferProceduralColorNode>::New();
  colorFreeSurferNode->SetTypeToRedBlue();

  ColorTableView1.setMRMLColorNode(colorFreeSurferNode);
  if (ColorTableView1.mrmlColorNode() != colorFreeSurferNode.GetPointer())
    {
    std::cerr << "qMRMLColorTableView::setMRMLColorNode() failed" << std::endl;
    return EXIT_FAILURE;
    }
  colorFreeSurferNode->SetTypeToLabels();
  
  vtkSmartPointer<vtkMRMLPETProceduralColorNode> colorPETNode =
    vtkSmartPointer<vtkMRMLPETProceduralColorNode>::New();
  colorPETNode->SetTypeToRainbow();
  ColorTableView2.setMRMLColorNode(colorPETNode);
  if (ColorTableView2.mrmlColorNode() != colorPETNode.GetPointer())
    {
    std::cerr << "qMRMLColorTableView::setMRMLColorNode() failed" << std::endl;
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

