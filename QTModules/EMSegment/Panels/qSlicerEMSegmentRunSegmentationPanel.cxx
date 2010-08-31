// Qt includes
#include <QDebug>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets includes
#include <qMRMLNodeFactory.h>

// EMSegment includes
#include "qSlicerEMSegmentRunSegmentationPanel.h" 
#include "ui_qSlicerEMSegmentRunSegmentationPanel.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSWorkingDataNode.h>
#include <vtkMRMLEMSTargetNode.h>
#include <vtkMRMLROINode.h>

// VTKMRML includes
#include "vtkMRMLSliceLogic.h"

//--------------------------------------------------------------------------
static ctkLogger logger(
    "org.slicer.qtmodules.emsegment.panels.qSlicerEMSegmentRunSegmentationPanel");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qSlicerEMSegmentRunSegmentationPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentRunSegmentationPanel>,
    public Ui_qSlicerEMSegmentRunSegmentationPanel
{
public:
  qSlicerEMSegmentRunSegmentationPanelPrivate()
    {
    }

  void setupUi(qSlicerEMSegmentRunSegmentationPanel *widget);
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentRunSegmentationPanelPrivate methods

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationPanelPrivate::setupUi(
    qSlicerEMSegmentRunSegmentationPanel* widget)
{
  this->Ui_qSlicerEMSegmentRunSegmentationPanel::setupUi(widget);

  // Set attributes on the qMRMLNodeComboBox to select the output label map
  this->OutputLabelMapComboBox->addAttribute("vtkMRMLScalarVolumeNode", "LabelMap", "1");

  // Setup connections
  QObject::connect(this->Display2DVOIButton, SIGNAL(clicked(bool)),
                   widget, SLOT(display2DVOI(bool)));
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentRunSegmentationPanel methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentRunSegmentationPanel::qSlicerEMSegmentRunSegmentationPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentRunSegmentationPanel);
  CTK_D(qSlicerEMSegmentRunSegmentationPanel);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationPanel::setMRMLROINode(vtkMRMLROINode* node)
{
  CTK_D(qSlicerEMSegmentRunSegmentationPanel);

  Q_ASSERT(node);
  Q_ASSERT(this->mrmlManager());
  Q_ASSERT(d->ROIWidget);

  // Finish setting up the ROI widget, now that we have a MRML manager
  vtkMRMLVolumeNode* volumeNode =
      this->mrmlManager()->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0);

  double rasDimensions[3];
  double rasCenter[3];
  vtkMRMLSliceLogic::GetVolumeRASBox(volumeNode, rasDimensions, rasCenter);

  double rasBounds[6];
  rasBounds[0] = qMin(rasCenter[0]-rasDimensions[0]/2.,rasCenter[0]+rasDimensions[0]/2.);
  rasBounds[1] = qMin(rasCenter[1]-rasDimensions[1]/2.,rasCenter[1]+rasDimensions[1]/2.);
  rasBounds[2] = qMin(rasCenter[2]-rasDimensions[2]/2.,rasCenter[2]+rasDimensions[2]/2.);
  rasBounds[3] = qMax(rasCenter[0]-rasDimensions[0]/2.,rasCenter[0]+rasDimensions[0]/2.);
  rasBounds[4] = qMax(rasCenter[1]-rasDimensions[1]/2.,rasCenter[1]+rasDimensions[1]/2.);
  rasBounds[5] = qMax(rasCenter[2]-rasDimensions[2]/2.,rasCenter[2]+rasDimensions[2]/2.);
    
  // TODO
  // this->ROIWidget->SetXRangeExtent(rasBounds[0],rasBounds[3]);
  // this->ROIWidget->SetYRangeExtent(rasBounds[1],rasBounds[4]);
  // this->ROIWidget->SetZRangeExtent(rasBounds[2],rasBounds[5]);

  // this->ROIWidget->SetXResolution(fabs(rasBounds[3]-rasBounds[0])/100.);
  // this->ROIWidget->SetYResolution(fabs(rasBounds[4]-rasBounds[1])/100.);
  // this->ROIWidget->SetZResolution(fabs(rasBounds[5]-rasBounds[2])/100.);

  d->ROIWidget->setMRMLROINode(node);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationPanel::updateWidgetFromMRML()
{
  CTK_D(qSlicerEMSegmentRunSegmentationPanel);

  if (!this->mrmlManager())
    {
    logger.warn("updateWidgetFromMRML - MRMLManager is NULL");
    return;
    }

  // Disable widget if needed
  QList<QWidget*> widgets;
  widgets << d->SaveIntermediateResultsCheckBox
          << d->OutputLabelMapComboBox
          << d->ROIWidget
          << d->Display2DVOIButton
          << d->MultithreadingCheckBox;

  foreach(QWidget* w, widgets)
    {
    w->setEnabled(this->mrmlManager()->HasGlobalParametersNode());
    }

  // Working directory
  d->SaveIntermediateResultsCheckBox->setChecked(
      this->mrmlManager()->GetSaveIntermediateResults());

  if (this->mrmlManager()->GetSaveWorkingDirectory())
    {
    d->DirectoryButton->setDirectory(
        this->mrmlManager()->GetSaveWorkingDirectory());
    }
  else
    {
    d->DirectoryButton->setCaption(tr("Select Working Directory"));
    }

  // Output label map
  if (!this->mrmlManager()->GetOutputVolumeMRMLID())
    {
    qMRMLNodeFactory::AttributeType attributes;
    attributes.insert("LabelMap", "1");
    qMRMLNodeFactory::createNode(
        this->mrmlScene(), "vtkMRMLScalarVolumeNode", vtkMRMLNodeInitializer(), attributes);
    }

  d->OutputLabelMapComboBox->setCurrentNode(this->mrmlManager()->GetOutputVolumeMRMLID());

  // Define VOI
  // TODO: should call SetMRMLROINode above? Incase the working volume has changed?

  // Miscellaneous
  d->MultithreadingCheckBox->setChecked(this->mrmlManager()->GetEnableMultithreading());
  
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationPanel::updateMRMLFromWidget()
{
  CTK_D(qSlicerEMSegmentRunSegmentationPanel);

  Q_ASSERT(this->mrmlManager());

  // Working directory
  this->mrmlManager()->SetSaveIntermediateResults(d->SaveIntermediateResultsCheckBox->isChecked());
  this->mrmlManager()->SetSaveWorkingDirectory(d->DirectoryButton->directory().toLatin1());

  // Output label map
  this->mrmlManager()->SetOutputVolumeMRMLID(d->OutputLabelMapComboBox->currentNode()->GetID());
  
  // Define VOI

  // Miscellaneous
  this->mrmlManager()->SetEnableMultithreading(d->MultithreadingCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationPanel::display2DVOI(bool show)
{

}


//-----------------------------------------------------------------------------
/* Notes:
*** done:
- setCommand
- callback

*** search for "addObserver" / "guiobservers" / processGUIEvents / processMRMLEvents
- ROIMRMLCallBack
- WizardGUICallBack

 */
//-----------------------------------------------------------------------------
