// Qt includes
#include <QCheckBox>
#include <QDebug>

// SlicerQt includes
#include "qCTKFlowLayout.h"
#include "qSlicerEMSegmentGraphWidget.h"
#include "ui_qSlicerEMSegmentGraphWidget.h"

// EMSegment includes
#include "vtkEMSegmentMRMLManager.h"
#include "vtkMRMLEMSTreeNode.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkMRMLEMSTargetNode.h"
#include "vtkPlotGaussian.h"

// MRML includes
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkChartXY.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerEMSegmentGraphWidgetPrivate : public ctkPrivate<qSlicerEMSegmentGraphWidget>,
                                          public Ui_qSlicerEMSegmentGraphWidget
{
public:
  qSlicerEMSegmentGraphWidgetPrivate();
  virtual void setupUi(qSlicerEMSegmentWidget* widget);
  QList<vtkIdType> classNodeIDs(vtkIdType nodeID)const;
  void addClass(vtkIdType nodeID);
  void updateClass(vtkIdType nodeID, int input);

  // input is 0 or 1
  QString volumeID(int input)const;
  int volumeIndex(const QString&)const;

  vtkSmartPointer<vtkContextView> Chart0View;
  vtkSmartPointer<vtkContextView> Chart1View;
  vtkSmartPointer<vtkChartXY>     Chart0;
  vtkSmartPointer<vtkChartXY>     Chart1;
  QMap<QString, QString>          VolumeList;
  qCTKFlowLayout*                 ClassListLayout;
  QMap<vtkIdType, vtkPlotGaussian*> ClassPlotList[2];
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentGraphWidgetPrivate::qSlicerEMSegmentGraphWidgetPrivate()
{
  this->Chart0View = vtkSmartPointer<vtkContextView>::New();
  this->Chart0 = vtkSmartPointer<vtkChartXY>::New();
  this->Chart0View->GetScene()->AddItem(this->Chart0);

  this->Chart1View = vtkSmartPointer<vtkContextView>::New();
  this->Chart1= vtkSmartPointer<vtkChartXY>::New();
  this->Chart1View->GetScene()->AddItem(this->Chart1);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentGraphWidgetPrivate::setupUi(qSlicerEMSegmentWidget* widget)
{
  CTK_P(qSlicerEMSegmentGraphWidget);
  this->Ui_qSlicerEMSegmentGraphWidget::setupUi(widget);
  this->Chart0View->SetInteractor(this->Graph0Widget->GetInteractor());
  this->Graph0Widget->SetRenderWindow(this->Chart0View->GetRenderWindow());
  this->Chart1View->SetInteractor(this->Graph1Widget->GetInteractor());
  this->Graph1Widget->SetRenderWindow(this->Chart1View->GetRenderWindow());

  this->ClassListLayout = new qCTKFlowLayout;
  this->Input0Layout->addLayout(this->ClassListLayout);

  QObject::connect(this->Input0ComboBox, SIGNAL(currentIndexChanged(const QString&)),
                   p, SLOT(onCurrentInput0VolumeChanged(const QString&)));
  QObject::connect(this->Input1ComboBox, SIGNAL(currentIndexChanged(const QString&)),
                   p, SLOT(onCurrentInput1VolumeChanged(const QString&)));
}

//-----------------------------------------------------------------------------
QList<vtkIdType> qSlicerEMSegmentGraphWidgetPrivate::classNodeIDs(vtkIdType nodeID)const
{
  CTK_P(const qSlicerEMSegmentGraphWidget);
  Q_ASSERT(p->mrmlManager());
  if (p->mrmlManager()->GetTreeNodeIsLeaf(nodeID))
    {
    return QList<vtkIdType>() << nodeID;
    }
  QList<vtkIdType> list;
  const int childCount = p->mrmlManager()->GetTreeNodeNumberOfChildren(nodeID);
  for(int i = 0; i < childCount; ++i)
    {
    list += this->classNodeIDs(p->mrmlManager()->GetTreeNodeChildNodeID(nodeID, i));
    }
  return list;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentGraphWidgetPrivate::addClass(vtkIdType classID)
{
  CTK_P(qSlicerEMSegmentGraphWidget);
  vtkMRMLEMSTreeNode* classNode = p->mrmlManager()->GetTreeNode(classID);
  QCheckBox* checkBox = new QCheckBox(p);
  // ID
  checkBox->setAccessibleName(QString::number(classID));
  // name
  checkBox->setText(classNode->GetName());
  // color
  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(
    p->mrmlScene()->GetNodeByID( p->mrmlManager()->GetColormap()?
                                 p->mrmlManager()->GetColormap():
                                 "vtkMRMLColorTableNodeLabels"));
  vtkLookupTable* lut = colorTableNode ? colorTableNode->GetLookupTable() : 0;
  double rgba[4] = {0., 0., 0., 1.};
  if (lut)
    {
    lut->GetTableValue(p->mrmlManager()->GetTreeNodeIntensityLabel(classID),rgba);
    }
  QColor classColor = QColor::fromRgbF(rgba[0], rgba[1], rgba[2], 1.);
  if (classColor.lightnessF() < 0.4)
    {
    classColor.setHsvF(classColor.hueF(), classColor.saturationF(), 0.4);
    }
  QPalette checkBoxPalette = checkBox->palette();
  checkBoxPalette.setColor(QPalette::Base, classColor);
  checkBox->setPalette(checkBoxPalette);
  
  // tooltip
  checkBox->setToolTip(QString("Check to display Gaussian of %1 class with label: ").arg(classID) + classNode->GetName() );
  this->ClassListLayout->addWidget(checkBox);
  // Plot gaussian, chart0
  vtkSmartPointer<vtkPlotGaussian> plot =
    vtkSmartPointer<vtkPlotGaussian>::New();
  plot->GetInput()->GetColumn(1)->SetName(classNode->GetName());
  plot->SetInputArray(1,classNode->GetName());
  plot->SetColor(rgba[0], rgba[1], rgba[2]);
  this->Chart0->AddPlot(plot);
  this->ClassPlotList[0][classID] = plot;
  checkBox->setChecked(plot->GetVisible());
  QObject::connect(checkBox, SIGNAL(toggled(bool)),
                   p, SLOT(onClassVisibilityToggled(bool)));
  this->updateClass(classID, 0);
  // Plot gaussian, chart1
  plot = vtkSmartPointer<vtkPlotGaussian>::New();
  plot->GetInput()->GetColumn(1)->SetName(classNode->GetName());
  plot->SetInputArray(1,classNode->GetName());
  plot->SetColor(rgba[0], rgba[1], rgba[2]);
  this->Chart1->AddPlot(plot);
  this->ClassPlotList[1][classID] = plot;
  checkBox->setChecked(plot->GetVisible());
  QObject::connect(checkBox, SIGNAL(toggled(bool)),
                   p, SLOT(onClassVisibilityToggled(bool)));
  this->updateClass(classID, 1);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentGraphWidgetPrivate::updateClass(vtkIdType classID, int input)
{
  CTK_P(qSlicerEMSegmentGraphWidget);
  vtkPlotGaussian* plot = this->ClassPlotList[input][classID];
  int index = this->volumeIndex(this->volumeID(input));
  plot->SetMean(p->mrmlManager()->GetTreeNodeDistributionLogMean(classID, index));
  plot->SetCovariance(p->mrmlManager()->GetTreeNodeDistributionLogCovariance(classID, index, index));
  plot->SetProbability(p->mrmlManager()->GetTreeNodeClassProbability(classID));
  plot->SetLog(true);
  plot->GetScene()->SetDirty(true);
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentGraphWidgetPrivate::volumeID(int input)const
{
  QString volumeName;
  switch(input)
    {
    case 0:
      volumeName = this->Input0ComboBox->currentText();
      break;
    case 1:
      volumeName = this->Input1ComboBox->currentText();
      break;
    }
  return this->VolumeList.key(volumeName);
}

//-----------------------------------------------------------------------------
int qSlicerEMSegmentGraphWidgetPrivate::volumeIndex(const QString& volumeID)const
{
  return this->VolumeList.keys().indexOf(volumeID);
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentGraphWidget::qSlicerEMSegmentGraphWidget(QWidget *parentWidget)
  :Superclass(parentWidget)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentGraphWidget);
  CTK_D(qSlicerEMSegmentGraphWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentGraphWidget::setMRMLManager(vtkEMSegmentMRMLManager* newMRMLManager)
{
  if (this->mrmlManager() == newMRMLManager)
    {
    return;
    }
  this->Superclass::setMRMLManager(newMRMLManager);
  this->updateFromMRMLManager();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentGraphWidget::updateFromMRMLManager()
{
  CTK_D(qSlicerEMSegmentGraphWidget);
  d->VolumeList.clear();
  d->Input0ComboBox->clear();
  d->Input1ComboBox->clear();

  vtkEMSegmentMRMLManager* manager = this->mrmlManager();
  vtkMRMLEMSWorkingDataNode* workingDataNode = manager->GetWorkingDataNode();
  Q_ASSERT_X(workingDataNode, __FUNCTION__, "Can't find a valid vtkMRMLEMSWorkingDataNode");
  vtkMRMLEMSTargetNode* input = workingDataNode->GetInputTargetNode();
  vtkMRMLEMSTargetNode* aligned = workingDataNode->GetAlignedTargetNode();
  if (!aligned)
    {// TBD: currently not set up correctly so we simply use input
    aligned = input;
    }

  // Retrieve all the volumes from the manager
  const int volumeCount = aligned->GetNumberOfVolumes();
  for (int i = 0; i < volumeCount; ++i)
    {
    QString volumeID = aligned->GetNthVolumeNodeID(i);
    d->VolumeList[volumeID] = input->GetNthInputChannelName(i);
    if (d->VolumeList[volumeID].isEmpty())
      {
      vtkMRMLVolumeNode* volume = vtkMRMLVolumeNode::SafeDownCast(
        this->mrmlScene()->GetNodeByID(volumeID.toLatin1()));
      d->VolumeList[volumeID] = volume->GetName();
      }
    }
  // populate the combo boxes with the volumes names
  d->Input0ComboBox->addItems(d->VolumeList.values());
  d->Input0ComboBox->setCurrentIndex(0);
  if (volumeCount > 1)
    {
    d->Input1ComboBox->addItems(d->VolumeList.values());
    d->Input1ComboBox->setCurrentIndex(1);
    }

  // populate the class list
  foreach(vtkIdType classID, d->classNodeIDs(manager->GetTreeRootNodeID()))
    {
    vtkMRMLEMSTreeNode* classNode = manager->GetTreeNode(classID);
    if (!classNode)
      {
      qWarning() << "Node" << classID << "does not exist";
      continue;
      }
    d->addClass(classID);
    }
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentGraphWidget::onCurrentInput0VolumeChanged(const QString& volumeName)
{
  CTK_D(qSlicerEMSegmentGraphWidget);
  QString input1 = d->Input1ComboBox->currentText();
  if (input1 == volumeName)
    {// the volume can't be selectable as Input1, select a different one
    QStringList volumes = d->VolumeList.values();
    volumes.removeAll(volumeName);
    d->Input1ComboBox->setCurrentIndex(d->Input1ComboBox->findText(volumes.first()));
    }
  foreach(vtkIdType classID, d->ClassPlotList[0].keys())
    {
    d->updateClass(classID, 0);
    }

  d->Chart0->SetTitle(volumeName.toLatin1());
  d->Chart0->RecalculateBounds();
  d->Chart0View->Render();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentGraphWidget::onCurrentInput1VolumeChanged(const QString& volumeName)
{
  CTK_D(qSlicerEMSegmentGraphWidget);
  QString input0 = d->Input0ComboBox->currentText();
  if (input0 == volumeName)
    {// the volume can't be selectable as Input1, select a different one
    QStringList volumes = d->VolumeList.values();
    volumes.removeAll(volumeName);
    d->Input0ComboBox->setCurrentIndex(d->Input0ComboBox->findText(volumes.first()));
    }
  foreach(vtkIdType classID, d->ClassPlotList[1].keys())
    {
    d->updateClass(classID, 1);
    }
  d->Chart1->SetTitle(volumeName.toLatin1());
  d->Chart1->RecalculateBounds();
  d->Chart1View->Render();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentGraphWidget::onClassVisibilityToggled(bool visible)
{
  CTK_D(qSlicerEMSegmentGraphWidget);
  QCheckBox* classCheckBox = qobject_cast<QCheckBox*>(this->sender());
  Q_ASSERT(classCheckBox);
  vtkIdType classID = classCheckBox->accessibleName().toInt();
  vtkPlotGaussian* plot = d->ClassPlotList[0][classID];
  plot->SetVisible(visible);
  plot->GetScene()->SetDirty(true);
  d->Chart0View->Render();
  plot = d->ClassPlotList[1][classID];
  plot->SetVisible(visible);
  plot->GetScene()->SetDirty(true);
  d->Chart1View->Render();
}
