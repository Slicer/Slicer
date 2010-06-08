// Qt includes
#include <QDebug>

// CTK includes
#include <ctkModelTester.h>

#include "qSlicerModuleTemplateModuleWidget.h"
#include "ui_qSlicerModuleTemplateModule.h"

//-----------------------------------------------------------------------------
class qSlicerModuleTemplateModuleWidgetPrivate: public ctkPrivate<qSlicerModuleTemplateModuleWidget>,
                                         public Ui_qSlicerModuleTemplateModule
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerModuleTemplateModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerModuleTemplateModuleWidget::setup()
{
  CTK_D(qSlicerModuleTemplateModuleWidget);
  d->setupUi(this);

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, 
                                                SLOT(setActiveVolumeNode(vtkMRMLNode*)));
  ctkModelTester* tester = new ctkModelTester(this);
  tester->setModel(d->ActiveVolumeNodeSelector->model());
}

//-----------------------------------------------------------------------------
QAction* qSlicerModuleTemplateModuleWidget::showModuleAction()
{
  return new QAction(QIcon(":/Icons/ModuleTemplate.png"), tr("Show ModuleTemplate module"), this);
}

//-----------------------------------------------------------------------------
void qSlicerModuleTemplateModuleWidget::setActiveVolumeNode(vtkMRMLNode* node)
{
  CTK_D(qSlicerModuleTemplateModuleWidget);
}
