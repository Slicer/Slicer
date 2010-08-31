/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QMenu>
#include <QFileInfo>

// CTK includes
#include <ctkVTKSliceView.h>
#include <ctkLogger.h>
#include <vtkLightBoxRendererManager.h>

// qMRML includes
#include "qMRMLSliceWidget.h"
#include "qMRMLSliceWidget_p.h"

// MRMLDisplayableManager includes
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLDisplayableManagerGroup.h>
//#include <vtkSliceViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkImageData.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSliceWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceWidgetPrivate::qMRMLSliceWidgetPrivate()
{
  this->DisplayableManagerGroup = 0;
  this->MRMLSliceNode = 0;
}

//---------------------------------------------------------------------------
qMRMLSliceWidgetPrivate::~qMRMLSliceWidgetPrivate()
{
  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::onSceneAboutToBeClosedEvent()
{
  this->VTKSliceView->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::onSceneClosedEvent()
{
  CTK_P(qMRMLSliceWidget);
  if (!p->mrmlScene()->GetIsUpdating())
    {
    this->VTKSliceView->setRenderEnabled(true);
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::onSceneAboutToBeImportedEvent()
{
  this->VTKSliceView->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::onSceneImportedEvent()
{
  this->VTKSliceView->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::onSceneRestoredEvent()
{
  this->VTKSliceView->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::onImageDataModified(vtkImageData * imageData)
{
  logger.trace("onImageDataModifiedEvent");
  this->VTKSliceView->setImageData(imageData);
  this->VTKSliceView->scheduleRender();
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::updateWidgetFromMRMLSliceNode()
{
  Q_ASSERT(this->MRMLSliceNode);
  this->VTKSliceView->lightBoxRendererManager()->SetRenderWindowLayout(
      this->MRMLSliceNode->GetLayoutGridRows(),
      this->MRMLSliceNode->GetLayoutGridColumns());
}

// --------------------------------------------------------------------------
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceWidget::qMRMLSliceWidget(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLSliceWidget);
  CTK_D(qMRMLSliceWidget);
  d->setupUi(this);

  // Highligh first RenderWindowItem
  d->VTKSliceView->lightBoxRendererManager()->SetHighlighted(0, 0, true);

  connect(d->VTKSliceView, SIGNAL(resized(const QSize&,const QSize&)),
          d->SliceController, SLOT(setSliceViewSize(const QSize&)));

  connect(d->SliceController,
          SIGNAL(imageDataModified(vtkImageData*)), d,
          SLOT(onImageDataModified(vtkImageData*)));
}

//------------------------------------------------------------------------------
void qMRMLSliceWidget::registerDisplayableManagers(const QString& scriptedDisplayableManagerDirectory)
{
  CTK_D(qMRMLSliceWidget);

  QStringList displayableManagers;
  //displayableManagers << "vtkSliceDisplayableManager";

#ifdef Slicer3_USE_PYTHONQT
  QFileInfo dirInfo(scriptedDisplayableManagerDirectory);
  if (dirInfo.isDir())
    {
    //displayableManagers<< QString("%1/vtkScriptedExampleDisplayableManager.py").
    //  arg(scriptedDisplayableManagerDirectory);
    }
  else
    {
    logger.error(QString("registerDisplayableManagers - directory %1 doesn't exists !").
                 arg(scriptedDisplayableManagerDirectory));
    }
#endif

  // Register Displayable Managers
  vtkMRMLSliceViewDisplayableManagerFactory* factory =
      vtkMRMLSliceViewDisplayableManagerFactory::GetInstance();

  foreach(const QString displayableManagerName, displayableManagers)
    {
    if (!factory->IsDisplayableManagerRegistered(displayableManagerName.toLatin1()))
      {
      factory->RegisterDisplayableManager(displayableManagerName.toLatin1());
      }
    }

  d->DisplayableManagerGroup = factory->InstantiateDisplayableManagers(
      d->VTKSliceView->lightBoxRendererManager()->GetRenderer(0));
  Q_ASSERT(d->DisplayableManagerGroup);

  // Observe displayable manager group to catch RequestRender events
  d->qvtkConnect(d->DisplayableManagerGroup, vtkCommand::UpdateEvent,
                 d->VTKSliceView, SLOT(scheduleRender()));
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  CTK_D(qMRMLSliceWidget);
  if (newScene == this->mrmlScene())
    {
    return;
    }

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::SceneAboutToBeClosedEvent, d, SLOT(onSceneAboutToBeClosedEvent()));

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::SceneClosedEvent, d, SLOT(onSceneClosedEvent()));

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::SceneAboutToBeImportedEvent, d, SLOT(onSceneAboutToBeImportedEvent()));

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::SceneImportedEvent, d, SLOT(onSceneImportedEvent()));

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::SceneRestoredEvent, d, SLOT(onSceneRestoredEvent()));

  this->Superclass::setMRMLScene(newScene);
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  CTK_D(qMRMLSliceWidget);
  if (newSliceNode == d->MRMLSliceNode)
    {
    return;
    }
  d->DisplayableManagerGroup->SetMRMLDisplayableNode(newSliceNode);
  d->SliceController->setMRMLSliceNode(newSliceNode);

  d->qvtkReconnect(d->MRMLSliceNode, newSliceNode, vtkCommand::ModifiedEvent,
                   d, SLOT(updateWidgetFromMRMLSliceNode()));

  d->MRMLSliceNode = newSliceNode;

  if (newSliceNode)
    {
    d->updateWidgetFromMRMLSliceNode();
    }
}

//---------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* qMRMLSliceWidget::mrmlSliceCompositeNode()const
{
  return ctk_d()->SliceController->mrmlSliceCompositeNode();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceViewName(const QString& newSliceViewName)
{
  CTK_D(qMRMLSliceWidget);
  d->SliceController->setSliceViewName(newSliceViewName);

  // If name matches either 'Red, 'Green' or 'Yellow' set the corresponding color
  // set Orange otherwise
  QColor highlightedBoxColor;
  highlightedBoxColor.setRgbF(0.882352941176, 0.439215686275, 0.0705882352941); // orange
  if (newSliceViewName == "Red")
    {
    highlightedBoxColor.setRgbF(0.952941176471, 0.290196078431, 0.2); // red
    }
  else if (newSliceViewName == "Green")
    {
    highlightedBoxColor.setRgbF(0.43137254902, 0.690196078431, 0.294117647059); // green
    }
  else if (newSliceViewName == "Yellow")
    {
    highlightedBoxColor.setRgbF(0.929411764706, 0.835294117647, 0.298039215686); // yellow
    }

  // Set the color associated with the highlightedBox
  d->VTKSliceView->setHighlightedBoxColor(highlightedBoxColor);
}

//---------------------------------------------------------------------------
QString qMRMLSliceWidget::sliceViewName()const
{
  return ctk_d()->SliceController->sliceViewName();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceOrientation(const QString& orientation)
{
  ctk_d()->SliceController->setSliceOrientation(orientation);
}

//---------------------------------------------------------------------------
QString qMRMLSliceWidget::sliceOrientation()const
{
  return ctk_d()->SliceController->sliceOrientation();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setImageData(vtkImageData* newImageData)
{
  ctk_d()->SliceController->setImageData(newImageData);
}

//---------------------------------------------------------------------------
vtkImageData* qMRMLSliceWidget::imageData() const
{
  return ctk_d()->SliceController->imageData();
}

//---------------------------------------------------------------------------
vtkInteractorObserver* qMRMLSliceWidget::interactorStyle()const
{
  return this->sliceView()->interactorStyle();
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* qMRMLSliceWidget::mrmlSliceNode()const
{
  return ctk_d()->SliceController->mrmlSliceNode();
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* qMRMLSliceWidget::sliceLogic()const
{
  return ctk_d()->SliceController->sliceLogic();
}

// --------------------------------------------------------------------------
void qMRMLSliceWidget::fitSliceToBackground()
{
  ctk_d()->SliceController->fitSliceToBackground();
}

// --------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceWidget, ctkVTKSliceView*, sliceView, VTKSliceView);
CTK_GET_CXX(qMRMLSliceWidget, qMRMLSliceControllerWidget*, sliceController, SliceController);

// --------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceLogics(vtkCollection* logics)
{
  ctk_d()->SliceController->setSliceLogics(logics);
}
