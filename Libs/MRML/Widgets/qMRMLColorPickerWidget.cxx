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
#include <QDialog>
#include <QKeyEvent>
#include <QStringListModel>

// qMRML includes
#include "qMRMLColorPickerWidget.h"
#include "ui_qMRMLColorPickerWidget.h"

// MRMLLogic includes
#include <vtkMRMLColorLogic.h>

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qMRMLColorPickerWidgetPrivate: public Ui_qMRMLColorPickerWidget
{
  Q_DECLARE_PUBLIC(qMRMLColorPickerWidget);

protected:
  qMRMLColorPickerWidget* const q_ptr;

public:
  qMRMLColorPickerWidgetPrivate(qMRMLColorPickerWidget& object);
  void init();

  vtkSmartPointer<vtkMRMLColorLogic> ColorLogic;
};

//------------------------------------------------------------------------------
qMRMLColorPickerWidgetPrivate::qMRMLColorPickerWidgetPrivate(qMRMLColorPickerWidget& object)
  : q_ptr(&object)
{
  // Create a default color logic
  this->ColorLogic = vtkSmartPointer<vtkMRMLColorLogic>::New();
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidgetPrivate::init()
{
  Q_Q(qMRMLColorPickerWidget);
  this->setupUi(q);
  QObject::connect(this->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentColorNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this->SearchBox, SLOT(clear()));
  QObject::connect(this->MRMLColorListView, SIGNAL(colorSelected(int)),
                   q, SIGNAL(colorEntrySelected(int)));
  QObject::connect(this->MRMLColorListView, SIGNAL(colorSelected(QColor)),
                   q, SIGNAL(colorSelected(QColor)));
  QObject::connect(this->MRMLColorListView, SIGNAL(colorSelected(QString)),
                   q, SIGNAL(colorNameSelected(QString)));

  // SearchBox
  this->SearchBox->setPlaceholderText("Search color...");
  this->SearchBox->setShowSearchIcon(true);
  this->SearchBox->installEventFilter(q);
  QObject::connect(this->SearchBox, SIGNAL(textChanged(QString)),
                   q, SLOT(onTextChanged(QString)));
}

//------------------------------------------------------------------------------
qMRMLColorPickerWidget::qMRMLColorPickerWidget(QWidget *_parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLColorPickerWidgetPrivate(*this))
{
  Q_D(qMRMLColorPickerWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLColorPickerWidget::~qMRMLColorPickerWidget() = default;

//------------------------------------------------------------------------------
void qMRMLColorPickerWidget::setMRMLColorLogic(vtkMRMLColorLogic* colorLogic)
{
  Q_D(qMRMLColorPickerWidget);
  d->ColorLogic = colorLogic;
}

//------------------------------------------------------------------------------
vtkMRMLColorLogic* qMRMLColorPickerWidget::mrmlColorLogic()const
{
  Q_D(const qMRMLColorPickerWidget);
  return d->ColorLogic.GetPointer();
}

//------------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLColorPickerWidget::currentColorNode()const
{
  Q_D(const qMRMLColorPickerWidget);
  return vtkMRMLColorNode::SafeDownCast(d->ColorTableComboBox->currentNode());
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidget::setCurrentColorNode(vtkMRMLNode* node)
{
  Q_D(qMRMLColorPickerWidget);
  d->ColorTableComboBox->setCurrentNode(node);
  this->qvtkDisconnect(this->mrmlScene(), vtkMRMLScene::NodeAddedEvent,
                       this, SLOT(onNodeAdded(vtkObject*,vtkObject*)));
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidget::setCurrentColorNodeToDefault()
{
  Q_D(qMRMLColorPickerWidget);
  if (!this->mrmlScene())
    {
    return;
    }
  vtkMRMLNode* defaultColorNode =
    this->mrmlScene()->GetNodeByID( d->ColorLogic.GetPointer() != nullptr ?
                                    d->ColorLogic->GetDefaultEditorColorNodeID() :
                                    nullptr);
  if (defaultColorNode)
    {
    this->setCurrentColorNode(defaultColorNode);
    }
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidget::onNodeAdded(vtkObject* scene, vtkObject* nodeObject)
{
  Q_D(qMRMLColorPickerWidget);
  Q_UNUSED(scene);
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(nodeObject);
  if (node != nullptr && d->ColorLogic.GetPointer() != nullptr &&
      QString(node->GetID()) == d->ColorLogic->GetDefaultEditorColorNodeID())
    {
    this->setCurrentColorNode(node);
    }
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLColorPickerWidget);
  this->setCurrentColorNode(nullptr); // eventually disconnect NodeAddedEvent
  this->qMRMLWidget::setMRMLScene(scene);
  if (scene && !d->ColorTableComboBox->currentNode())
    {
    this->qvtkConnect(scene, vtkMRMLScene::NodeAddedEvent,
                      this, SLOT(onNodeAdded(vtkObject*,vtkObject*)));
    this->setCurrentColorNodeToDefault();
   }
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidget::onCurrentColorNodeChanged(vtkMRMLNode* colorNode)
{
  Q_D(qMRMLColorPickerWidget);
  // Search for the largest item
  QSize maxSizeHint;
  QModelIndex rootIndex = d->MRMLColorListView->rootIndex();
  const int count = d->MRMLColorListView->model()->rowCount(rootIndex);
  for (int i = 0; i < count; ++i)
    {
    QSize sizeHint = d->MRMLColorListView->sizeHintForIndex(
      d->MRMLColorListView->model()->index(i, 0, rootIndex));
    maxSizeHint.setWidth(qMax(maxSizeHint.width(), sizeHint.width()));
    maxSizeHint.setHeight(qMax(maxSizeHint.height(), sizeHint.height()));
    }
  // Set the largest the default size for all the items, that way they will
  // be aligned horizontally and vertically.
  d->MRMLColorListView->setGridSize(maxSizeHint);
  // Inform that the color node has changed.
  emit currentColorNodeChanged(colorNode);
}

//------------------------------------------------------------------------------
void qMRMLColorPickerWidget::onTextChanged(const QString& colorText)
{
  Q_D(qMRMLColorPickerWidget);
  QRegExp regExp(colorText,Qt::CaseInsensitive, QRegExp::RegExp);
  d->MRMLColorListView->sortFilterProxyModel()->setFilterRegExp(regExp);

  QModelIndex newCurrentIndex;

  if (!d->SearchBox->text().isEmpty())
    {
    QModelIndex start = d->MRMLColorListView->sortFilterProxyModel()
                        ->index(0,0);
    QModelIndexList indexList = d->MRMLColorListView->sortFilterProxyModel()
                              ->match(start, 0,
                                      d->SearchBox->text(), 1,
                                      Qt::MatchStartsWith);

    if (indexList.isEmpty())
      {
      indexList = d->MRMLColorListView->sortFilterProxyModel()
                                ->match(start, 0,
                                        d->SearchBox->text(), 1,
                                        Qt::MatchContains);
      }
    if(indexList.count() > 0 )
      {
      newCurrentIndex = indexList[0];
      }
    }
  // Show to the user and set the current index
  d->MRMLColorListView->setCurrentIndex(newCurrentIndex);
  d->SearchBox->setFocus();
}

//------------------------------------------------------------------------------
bool qMRMLColorPickerWidget::eventFilter(QObject* target, QEvent* event)
{
  Q_D(qMRMLColorPickerWidget);
  if (target == d->SearchBox)
    {
    if (event->type() == QEvent::Show)
      {
      d->SearchBox->clear();
      d->MRMLColorListView->setFocus();
      }
    if (event->type() == QEvent::KeyPress)
      {
      QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);
      if (keyEvent->key() == Qt::Key_Up ||
          keyEvent->key() == Qt::Key_Down)
        {
        // give the Focus to MRMLColorListView
        d->MRMLColorListView->setFocus();
        }
      }
    }
  return this->qMRMLWidget::eventFilter(target, event);
}
