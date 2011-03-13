// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerVolumeRenderingModuleWidget.h"
#include "ui_qSlicerVolumeRenderingModule.h"
#include "vtkMRMLVolumeRenderingParametersNode.h"
#include "vtkMRMLVolumeRenderingScenarioNode.h"

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

  QLabel *ParameterLabel;
  qMRMLNodeComboBox *ParameterNodeSelector;

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
  d->ActiveVolumeLabel->setText(QString::fromUtf8("Volume"));
  d->gridLayout->addWidget(d->ActiveVolumeLabel, 0, 0, 1, 1);

  d->ActiveVolumeNodeSelector = new qMRMLNodeComboBox(this);
  d->ActiveVolumeNodeSelector->setObjectName(QString::fromUtf8("ActiveVolumeNodeSelector"));
  d->ActiveVolumeNodeSelector->setNodeTypes(QStringList() << QString::fromUtf8("vtkMRMLScalarVolumeNode"));
  d->ActiveVolumeNodeSelector->setAddEnabled(false);
  d->ActiveVolumeNodeSelector->setRenameEnabled(true);

  d->gridLayout->addWidget(d->ActiveVolumeNodeSelector, 0, 1, 1, 1);

  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ActiveVolumeNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setMRMLVolumeNode(vtkMRMLNode*)));

  d->ParameterLabel = new QLabel(this);
  d->ParameterLabel->setObjectName(QString::fromUtf8("ParameterLabel"));
  d->ParameterLabel->setText(QString::fromUtf8("Parameter"));
  d->gridLayout->addWidget(d->ParameterLabel, 1, 0, 1, 1);

  d->ParameterNodeSelector = new qMRMLNodeComboBox(this);
  d->ParameterNodeSelector->setObjectName(QString::fromUtf8("ParameterNodeSelector"));
  d->ParameterNodeSelector->setNodeTypes(QStringList() << QString::fromUtf8("vtkMRMLVolumeRenderingParametersNode"));
  d->ParameterNodeSelector->setAddEnabled(true);
  d->ParameterNodeSelector->setRenameEnabled(true);

  d->gridLayout->addWidget(d->ParameterNodeSelector, 1, 1, 1, 1);

  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ParameterNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->ParameterNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setMRMLParameterNode(vtkMRMLNode*)));
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLParameterNode(vtkMRMLNode* paramNode)
{
  if (paramNode == NULL)
    {
    return;
    }
  vtkMRMLScene* scene = paramNode->GetScene();
  if (scene == NULL)
    {
    return;
    }

  // use vtkMRMLVolumeRenderingScenarioNode (assume singletone)
  // to hold current parameter node
  vtkMRMLVolumeRenderingScenarioNode *snode = vtkMRMLVolumeRenderingScenarioNode::SafeDownCast(
    scene->GetNthNodeByClass(0, "vtkMRMLVolumeRenderingScenarioNode"));

  if (snode == NULL)
    {
    snode = vtkMRMLVolumeRenderingScenarioNode::New();
    snode->SetParametersNodeID(paramNode->GetID());
    scene->AddNode(snode);
    snode->Delete();
    }

}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLVolumeNode(vtkMRMLNode* volumeNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

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


    volumeRenderingParametersNode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(
      scene->GetNthNodeByClass(0, "vtkMRMLVolumeRenderingParametersNode"));

    if (volumeRenderingParametersNode == NULL)
    {
      volumeRenderingParametersNode = vtkMRMLVolumeRenderingParametersNode::New();
      volumeRenderingParametersNode->SetCurrentVolumeMapper(0);
      scene->AddNode(volumeRenderingParametersNode);
      volumeRenderingParametersNode->Delete();
      d->ParameterNodeSelector->setCurrentNode(volumeRenderingParametersNode);
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



