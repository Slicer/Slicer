// Qt includes
#include <QDebug>

// CTK includes
#include <ctkModelTester.h>

#include "qSlicerVolumesModuleWidget.h"
#include "ui_qSlicerVolumesModule.h"

//-----------------------------------------------------------------------------
class qSlicerVolumesModuleWidgetPrivate: public ctkPrivate<qSlicerVolumesModuleWidget>,
                                         public Ui_qSlicerVolumesModule
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerVolumesModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::setup()
{
  CTK_D(qSlicerVolumesModuleWidget);
  d->setupUi(this);

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, 
                                                SLOT(setActiveVolumeNode(vtkMRMLNode*)));
  //ctkModelTester* tester = new ctkModelTester(this);
  //tester->setModel(d->ActiveVolumeNodeSelector->model());
}

//-----------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::setActiveVolumeNode(vtkMRMLNode* node)
{
  CTK_D(qSlicerVolumesModuleWidget);
  d->VolumeDisplayWidget->setMRMLVolumeNode(node);
}
