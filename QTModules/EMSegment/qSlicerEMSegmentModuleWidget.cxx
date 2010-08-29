// Qt includes
#include <QDebug>
#include <QList>

// EMSegment QTModule includes
#include "qSlicerEMSegmentModuleWidget.h"
#include "ui_qSlicerEMSegmentModule.h"
#include "qSlicerEMSegmentDefineAnatomicalTreePanel.h"
#include "qSlicerEMSegmentDefineAtlasPanel.h"
#include "qSlicerEMSegmentDefineInputChannelsPanel.h"
#include "qSlicerEMSegmentDefinePreprocessingPanel.h"
#include "qSlicerEMSegmentDefineTaskPanel.h"
#include "qSlicerEMSegmentEditNodeBasedParametersPanel.h"
#include "qSlicerEMSegmentEditRegistrationParametersPanel.h"
#include "qSlicerEMSegmentSpecifyIntensityDistributionPanel.h"
#include "qSlicerEMSegmentRunSegmentationPanel.h"

// EMSegment/Logic includes
#include <vtkSlicerEMSegmentLogic.h>

//-----------------------------------------------------------------------------
class qSlicerEMSegmentModuleWidgetPrivate: public ctkPrivate<qSlicerEMSegmentModuleWidget>,
                                           public Ui_qSlicerEMSegmentModule
{
public:

  vtkSlicerEMSegmentLogic* logic() const;

  QList<qSlicerEMSegmentWidget*> Panels;

};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
vtkSlicerEMSegmentLogic* qSlicerEMSegmentModuleWidgetPrivate::logic()const
{
  CTK_P(const qSlicerEMSegmentModuleWidget);
  return vtkSlicerEMSegmentLogic::SafeDownCast(p->logic());
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentModuleWidget methods

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerEMSegmentModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerEMSegmentModuleWidget::setup()
{
  CTK_D(qSlicerEMSegmentModuleWidget);
  d->setupUi(this);

  // Since Workflow manager is work-in-progress, let's disable control button
  QList<QWidget*> controlWidgets;
  controlWidgets << d->NextButton << d->BackButton << d->SegmentButton << d->CancelButton;
  foreach(QWidget* w, controlWidgets)
    {
    w->setDisabled(true);
    }

  // Instantiate Panels
  d->Panels << new qSlicerEMSegmentDefineTaskPanel
      << new qSlicerEMSegmentDefineInputChannelsPanel
      << new qSlicerEMSegmentDefineAnatomicalTreePanel
      << new qSlicerEMSegmentDefineAtlasPanel
      << new qSlicerEMSegmentEditRegistrationParametersPanel
      << new qSlicerEMSegmentDefinePreprocessingPanel
      << new qSlicerEMSegmentSpecifyIntensityDistributionPanel
      << new qSlicerEMSegmentEditNodeBasedParametersPanel
      << new qSlicerEMSegmentRunSegmentationPanel;

  foreach(qSlicerEMSegmentWidget* w, d->Panels)
    {
    // Add Panels to Stack
    d->PanelStack->addWidget(w);

    // Set MRML manager instance
    w->setMRMLManager(d->logic()->GetMRMLManager());

    // Connect mrmlSceneChanged signal
    this->connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                  w, SLOT(setMRMLScene(vtkMRMLScene*)));

    // Set scene
    w->setMRMLScene(this->mrmlScene());
    }

  this->connect(d->TestPanelsComboBox, SIGNAL(currentIndexChanged(int)),
                SLOT(showPanel(int)));

  d->PanelStack->setCurrentIndex(0);
}


//-----------------------------------------------------------------------------
void qSlicerEMSegmentModuleWidget::showPanel(int panelId)
{
  CTK_D(qSlicerEMSegmentModuleWidget);
  d->PanelStack->setCurrentIndex(panelId);
}
