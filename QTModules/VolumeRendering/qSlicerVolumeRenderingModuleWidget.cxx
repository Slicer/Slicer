// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerVolumeRenderingModuleWidget.h"
#include "ui_qSlicerVolumeRenderingModule.h"
#include "vtkMRMLThreeDViewDisplayableManagerFactory.h"
#include "vtkMRMLVolumeRenderingDisplayableManager.h"
#include "vtkMRMLDisplayableManagerGroup.h"

#include <QtGui/QLabel>
#include "qMRMLNodeComboBox.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScalarVolumeNode.h"

//-----------------------------------------------------------------------------
class qSlicerVolumeRenderingModuleWidgetPrivate: public Ui_qSlicerVolumeRenderingModule
{
public:
  qSlicerVolumeRenderingModuleWidgetPrivate();

  QLabel *ActiveVolumeLabel;
  qMRMLNodeComboBox *ActiveVolumeNodeSelector;

};

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidgetPrivate::qSlicerVolumeRenderingModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidget::qSlicerVolumeRenderingModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerVolumeRenderingModuleWidgetPrivate )
{
  this->volumeRenderingDisplayableManager = NULL;
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidget::~qSlicerVolumeRenderingModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setup()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  d->ActiveVolumeLabel = new QLabel(this);
  d->ActiveVolumeLabel->setObjectName(QString::fromUtf8("ActiveVolumeLabel"));

  d->gridLayout->addWidget(d->ActiveVolumeLabel, 0, 0, 1, 1);

  d->ActiveVolumeNodeSelector = new qMRMLNodeComboBox(this);
  d->ActiveVolumeNodeSelector->setObjectName(QString::fromUtf8("ActiveVolumeNodeSelector"));
  d->ActiveVolumeNodeSelector->setNodeTypes(QStringList() << QString::fromUtf8("vtkMRMLScalarVolumeNode"));
  d->ActiveVolumeNodeSelector->setAddEnabled(false);
  d->ActiveVolumeNodeSelector->setRenameEnabled(true);

  d->gridLayout->addWidget(d->ActiveVolumeNodeSelector, 0, 1, 1, 1);

  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ActiveVolumeNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setMRMLVolumeNode(vtkMRMLNode*)));
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLVolumeNode(vtkMRMLNode* volumeNode)
{
  if (volumeNode == NULL)
    {
    return;
    }
  vtkMRMLScene* scene = volumeNode->GetScene();
  if (scene == NULL)
    {
    return;
    }

  vtkMRMLScalarVolumeNode* scalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode);

  if (scalarVolumeNode)
    {
    vtkMRMLVolumeRenderingParametersNode* volumeRenderingParametersNode = NULL;
    // TODO: set parameters node to vtkMRMLVolumeRenderingDisplayableManager

    vtkMRMLVolumeRenderingDisplayableManager* volumeRenderingDisplayableManager = this->getVolumeRenderingDisplayableManager();

    volumeRenderingParametersNode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(
      scene->GetNthNodeByClass(0, "vtkMRMLVolumeRenderingParametersNode"));

    if (volumeRenderingParametersNode == NULL)
    {
      volumeRenderingParametersNode = vtkMRMLVolumeRenderingParametersNode::New();
      volumeRenderingParametersNode->SetCurrentVolumeMapper(0);
      scene->AddNode(volumeRenderingParametersNode);
      volumeRenderingParametersNode->Delete();
    }

    if (volumeRenderingParametersNode)
      {
      volumeRenderingParametersNode->SetAndObserveVolumeNodeID(scalarVolumeNode->GetID());
      if (volumeRenderingParametersNode->GetVolumePropertyNode() == NULL)
        {
        vtkMRMLVolumePropertyNode* propNode = vtkMRMLVolumePropertyNode::New();
        scene->AddNode(propNode);
        volumeRenderingParametersNode->SetAndObserveVolumePropertyNodeID(propNode->GetID());
        }
      }
    }
}

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager* qSlicerVolumeRenderingModuleWidget::getVolumeRenderingDisplayableManager()
{
  if (this->volumeRenderingDisplayableManager == NULL)
  {
    this->volumeRenderingDisplayableManager = vtkMRMLVolumeRenderingDisplayableManager::SafeDownCast(
      vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->GetDisplayableManagerGroup()->
                                   GetDisplayableManagerByClassName("vtkMRMLVolumeRenderingDisplayableManager"));
  }
  return this->volumeRenderingDisplayableManager;

}


