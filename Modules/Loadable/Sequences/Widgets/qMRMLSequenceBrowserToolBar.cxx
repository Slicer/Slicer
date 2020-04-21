/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// qMRML includes
#include "qMRMLSequenceBrowserToolBar.h"
#include "qMRMLNodeComboBox.h"
#include "qMRMLSequenceBrowserPlayWidget.h"
#include "qMRMLSequenceBrowserSeekWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSequenceBrowserNode.h>

// VTK includes
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qMRMLSequenceBrowserToolBarPrivate
{
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLSequenceBrowserToolBar);
protected:
  qMRMLSequenceBrowserToolBar* const q_ptr;
public:
  qMRMLSequenceBrowserToolBarPrivate(qMRMLSequenceBrowserToolBar& object);
  void init();
  void setMRMLScene(vtkMRMLScene* newScene);
  qMRMLNodeComboBox* SequenceBrowserNodeSelector;
  qMRMLSequenceBrowserPlayWidget* SequenceBrowserPlayWidget;
  qMRMLSequenceBrowserSeekWidget* SequenceBrowserSeekWidget;
};

//--------------------------------------------------------------------------
// qMRMLSequenceBrowserToolBarPrivate methods

//---------------------------------------------------------------------------
qMRMLSequenceBrowserToolBarPrivate::qMRMLSequenceBrowserToolBarPrivate(qMRMLSequenceBrowserToolBar& object)
  : q_ptr(&object)
{
  this->SequenceBrowserNodeSelector = 0;
  this->SequenceBrowserPlayWidget = 0;
  this->SequenceBrowserSeekWidget = 0;
}

//---------------------------------------------------------------------------
void qMRMLSequenceBrowserToolBarPrivate::init()
{
  Q_Q(qMRMLSequenceBrowserToolBar);

  this->SequenceBrowserNodeSelector = new qMRMLNodeComboBox();
  this->SequenceBrowserNodeSelector->setNodeTypes(QStringList(QString("vtkMRMLSequenceBrowserNode")));
  this->SequenceBrowserNodeSelector->setNoneEnabled(false);
  this->SequenceBrowserNodeSelector->setAddEnabled(false);
  this->SequenceBrowserNodeSelector->setRenameEnabled(true);
  this->SequenceBrowserNodeSelector->setMaximumWidth(350);

  this->SequenceBrowserPlayWidget = new qMRMLSequenceBrowserPlayWidget();
  this->SequenceBrowserPlayWidget->setMaximumWidth(450);

  this->SequenceBrowserSeekWidget = new qMRMLSequenceBrowserSeekWidget();
  this->SequenceBrowserSeekWidget->setMinimumWidth(150);
  this->SequenceBrowserSeekWidget->setMaximumWidth(350);

  q->addWidget(this->SequenceBrowserPlayWidget);
  q->addWidget(this->SequenceBrowserSeekWidget);
  q->addWidget(this->SequenceBrowserNodeSelector);

  this->SequenceBrowserPlayWidget->setPlayPauseShortcut("Ctrl+Shift+Down");
  this->SequenceBrowserPlayWidget->setPreviousFrameShortcut("Ctrl+Shift+Left");
  this->SequenceBrowserPlayWidget->setNextFrameShortcut("Ctrl+Shift+Right");

  QObject::connect(this->SequenceBrowserNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this->SequenceBrowserPlayWidget, SLOT(setMRMLSequenceBrowserNode(vtkMRMLNode*)) );
  QObject::connect(this->SequenceBrowserNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this->SequenceBrowserSeekWidget, SLOT(setMRMLSequenceBrowserNode(vtkMRMLNode*)) );
  QObject::connect(this->SequenceBrowserNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SIGNAL(activeBrowserNodeChanged(vtkMRMLNode*)) );
}
// --------------------------------------------------------------------------
void qMRMLSequenceBrowserToolBarPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qMRMLSequenceBrowserToolBar);

  this->SequenceBrowserNodeSelector->setMRMLScene(newScene);
}

// --------------------------------------------------------------------------
// qMRMLSequenceBrowserToolBar methods

// --------------------------------------------------------------------------
qMRMLSequenceBrowserToolBar::qMRMLSequenceBrowserToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
   , d_ptr(new qMRMLSequenceBrowserToolBarPrivate(*this))
{
  Q_D(qMRMLSequenceBrowserToolBar);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLSequenceBrowserToolBar::qMRMLSequenceBrowserToolBar(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLSequenceBrowserToolBarPrivate(*this))
{
  Q_D(qMRMLSequenceBrowserToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qMRMLSequenceBrowserToolBar::~qMRMLSequenceBrowserToolBar() = default;

// --------------------------------------------------------------------------
void qMRMLSequenceBrowserToolBar::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSequenceBrowserToolBar);
  d->setMRMLScene(scene);
}

// --------------------------------------------------------------------------
void qMRMLSequenceBrowserToolBar::setActiveBrowserNode(vtkMRMLSequenceBrowserNode * newActiveBrowserNode)
{
  Q_D(qMRMLSequenceBrowserToolBar);
  d->SequenceBrowserNodeSelector->setCurrentNode(newActiveBrowserNode);
}
