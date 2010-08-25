// Qt includes
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>

// SlicerQt includes
#include "qSlicerEMSegmentSpecifyIntensityDistributionPanel.h" 
#include "ui_qSlicerEMSegmentSpecifyIntensityDistributionPanel.h"
#include "qSlicerEMSegmentGraphWidget.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentSpecifyIntensityDistributionPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentSpecifyIntensityDistributionPanel>,
    public Ui_qSlicerEMSegmentSpecifyIntensityDistributionPanel
{
public:
  qSlicerEMSegmentSpecifyIntensityDistributionPanelPrivate()
    {
    }
  void setupUi(qSlicerEMSegmentWidget* widget);
};

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionPanelPrivate::setupUi(qSlicerEMSegmentWidget* widget)
{
  CTK_P(qSlicerEMSegmentSpecifyIntensityDistributionPanel);
  this->Ui_qSlicerEMSegmentSpecifyIntensityDistributionPanel::setupUi(widget);
  QObject::connect(this->PlotDistributionButton, SIGNAL(clicked()),
                   p, SLOT(openGraphDialog()));
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentSpecifyIntensityDistributionPanel::qSlicerEMSegmentSpecifyIntensityDistributionPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentSpecifyIntensityDistributionPanel);
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionPanel);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionPanel::printAdditionalInfo()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionPanel::setup()
{
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionPanel);
  d->EMSegmentAnatomicalTreeWidget->setup();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionPanel::openGraphDialog()
{
  QDialog dialog(this);
  qSlicerEMSegmentGraphWidget* graph = new qSlicerEMSegmentGraphWidget(&dialog);
  QVBoxLayout* boxLayout = new QVBoxLayout;
  boxLayout->addWidget(graph);
  dialog.setLayout(boxLayout);
  graph->setMRMLManager(this->mrmlManager());
  dialog.exec();
}
