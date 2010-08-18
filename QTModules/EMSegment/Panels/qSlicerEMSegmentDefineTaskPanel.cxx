// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerEMSegmentDefineTaskPanel.h" 
#include "ui_qSlicerEMSegmentDefineTaskPanel.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSNode.h>

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefineTaskPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentDefineTaskPanel>,
    public Ui_qSlicerEMSegmentDefineTaskPanel
{
public:
  qSlicerEMSegmentDefineTaskPanelPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineTaskPanel::qSlicerEMSegmentDefineTaskPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefineTaskPanel);
  CTK_D(qSlicerEMSegmentDefineTaskPanel);
  d->setupUi(this);

  // Connect TaskSelector
  connect(d->TaskSelectorComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          SLOT(selectTask(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineTaskPanel::printAdditionalInfo()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineTaskPanel::selectTask(vtkMRMLNode* mrmlNode)
{
  Q_ASSERT(vtkMRMLEMSNode::SafeDownCast(mrmlNode));
  this->mrmlManager()->SetNode(vtkMRMLEMSNode::SafeDownCast(mrmlNode));
}
