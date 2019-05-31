
// Qt includes
#include <QApplication>
#include <QDebug>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// Annotations includes
#include "qMRMLAnnotationROIWidget.h"
#include "vtkMRMLAnnotationROINode.h"

// MRML includes
#include <vtkMRMLScene.h>

#include "qMRMLWidget.h"

//-----------------------------------------------------------------------------
int qMRMLAnnotationROIWidgetTest1(int argc, char * argv[] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkSmartPointer<vtkMRMLScene> scene =
    vtkSmartPointer<vtkMRMLScene>::New();
  vtkSmartPointer<vtkMRMLAnnotationROINode> roi =
    vtkSmartPointer<vtkMRMLAnnotationROINode>::New();
  scene->AddNode(roi);

  qMRMLAnnotationROIWidget widget;
  widget.setMRMLAnnotationROINode(roi);

  qDebug() << "start edit";

  roi->SetXYZ(1, 1, 1);

  qDebug() << "end edit";

  widget.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(100, &app, SLOT(quit()));
    }

  return app.exec();
}


