/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QToolButton>

// CTK includes
#include <ctkLogger.h>
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLTableViewControllerWidget.h"
#include "qMRMLTableView.h"
#include "qMRMLTableWidget.h"

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLTableWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLTableWidgetPrivate
class qMRMLTableWidgetPrivate
  : public QObject
{
  Q_DECLARE_PUBLIC(qMRMLTableWidget);
protected:
  qMRMLTableWidget* const q_ptr;
public:
  qMRMLTableWidgetPrivate(qMRMLTableWidget& object);
  ~qMRMLTableWidgetPrivate() override;

  void init();

  qMRMLTableView*       TableView;
  qMRMLTableViewControllerWidget* TableController;
};


//---------------------------------------------------------------------------
qMRMLTableWidgetPrivate::qMRMLTableWidgetPrivate(qMRMLTableWidget& object)
  : q_ptr(&object)
{
  this->TableView = nullptr;
  this->TableController = nullptr;
}

//---------------------------------------------------------------------------
qMRMLTableWidgetPrivate::~qMRMLTableWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLTableWidgetPrivate::init()
{
  Q_Q(qMRMLTableWidget);

  QVBoxLayout* layout = new QVBoxLayout(q);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  this->TableController = new qMRMLTableViewControllerWidget;
  layout->addWidget(this->TableController);

  this->TableView = new qMRMLTableView;
  layout->addWidget(this->TableView);

  this->TableController->setTableView(this->TableView);

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->TableView, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->TableController, SLOT(setMRMLScene(vtkMRMLScene*)));
}

// --------------------------------------------------------------------------
// qMRMLTableWidget methods

// --------------------------------------------------------------------------
qMRMLTableWidget::qMRMLTableWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLTableWidgetPrivate(*this))
{
  Q_D(qMRMLTableWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLTableWidget::~qMRMLTableWidget()
{
  Q_D(qMRMLTableWidget);
  d->TableView->setMRMLScene(nullptr);
  d->TableController->setMRMLScene(nullptr);
}


// --------------------------------------------------------------------------
void qMRMLTableWidget::setMRMLTableViewNode(vtkMRMLTableViewNode* newTableViewNode)
{
  Q_D(qMRMLTableWidget);
  d->TableView->setMRMLTableViewNode(newTableViewNode);
  d->TableController->setMRMLTableViewNode(newTableViewNode);
}

// --------------------------------------------------------------------------
vtkMRMLTableViewNode* qMRMLTableWidget::mrmlTableViewNode()const
{
  Q_D(const qMRMLTableWidget);
  return d->TableView->mrmlTableViewNode();
}

// --------------------------------------------------------------------------
qMRMLTableView* qMRMLTableWidget::tableView()const
{
  Q_D(const qMRMLTableWidget);
  return d->TableView;
}

// --------------------------------------------------------------------------
qMRMLTableViewControllerWidget* qMRMLTableWidget::tableController()const
{
  Q_D(const qMRMLTableWidget);
  return d->TableController;
}

//---------------------------------------------------------------------------
void qMRMLTableWidget::setViewLabel(const QString& newTableViewLabel)
{
  Q_D(qMRMLTableWidget);
  d->TableController->setViewLabel(newTableViewLabel);
}

//---------------------------------------------------------------------------
QString qMRMLTableWidget::viewLabel()const
{
  Q_D(const qMRMLTableWidget);
  return d->TableController->viewLabel();
}
