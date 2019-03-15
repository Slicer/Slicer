/*=========================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Benjamin LONG, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

=========================================================================*/

// Qt includes
#include <QAbstractItemModel>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

// CTK includes
#include "ctkCheckableModelHelper.h"
#include "ctkComboBox.h"

// qMRML includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLNodeComboBoxEventTranslator.h"

// MRML includes
#include "vtkMRMLNode.h"

// ----------------------------------------------------------------------------
qMRMLNodeComboBoxEventTranslator::qMRMLNodeComboBoxEventTranslator(QObject *parent)
  : pqWidgetEventTranslator(parent)
{
  this->CurrentObject = nullptr;
}

// ----------------------------------------------------------------------------
bool qMRMLNodeComboBoxEventTranslator::translateEvent(QObject *Object,
                                                      QEvent *Event,
                                                      bool &Error)
{
  Q_UNUSED(Error);

  qMRMLNodeComboBox* widget = nullptr;
  for(QObject* test = Object; widget == nullptr && test != nullptr; test = test->parent())
    {
    widget = qobject_cast<qMRMLNodeComboBox*>(test);
    }
  if(!widget)
    {
    return false;
    }

  if(Event->type() == QEvent::Enter && Object == widget)
    {
    if(this->CurrentObject != Object)
      {
      if(this->CurrentObject)
        {
        disconnect(this->CurrentObject, nullptr, this, nullptr);
        }
      this->CurrentObject = Object;
      connect(widget, SIGNAL(destroyed(QObject*)),
              this, SLOT(onDestroyed(QObject*)));
      connect(widget, SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
              this, SLOT(onRowsInserted()));
      connect(widget, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode*)),
              this, SLOT(onNodeAboutToBeRemoved(vtkMRMLNode*)));
      connect(widget, SIGNAL(currentNodeRenamed(QString)),
              this, SLOT(onCurrentNodeRenamed(QString)));
      }
    if(this->CurrentObject)
      {
      connect(this->CurrentObject, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
              this, SLOT(onCurrentNodeChanged(vtkMRMLNode*)), Qt::UniqueConnection);
      }
    }

  return true;
}

// ----------------------------------------------------------------------------
void qMRMLNodeComboBoxEventTranslator::onDestroyed(QObject* /*Object*/)
{
  this->CurrentObject = nullptr;
}

// ----------------------------------------------------------------------------
void qMRMLNodeComboBoxEventTranslator::onRowsInserted()
{
  disconnect(this->CurrentObject, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
             this, SLOT(onCurrentNodeChanged(vtkMRMLNode*)));
  emit recordEvent(this->CurrentObject, "nodeAddedByUser", "");
}

// ----------------------------------------------------------------------------
void qMRMLNodeComboBoxEventTranslator::onCurrentNodeChanged(vtkMRMLNode* node)
{
  if(node)
    {
    emit recordEvent(this->CurrentObject, "currentNodeChanged", QString(node->GetID()));
    }
  else
    {
    emit recordEvent(this->CurrentObject, "currentNodeChanged", "None");
    }
}

// ----------------------------------------------------------------------------
void qMRMLNodeComboBoxEventTranslator::onNodeAboutToBeRemoved(vtkMRMLNode* node)
{
  if(node)
    {
    disconnect(this->CurrentObject, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
               this, SLOT(onCurrentNodeChanged(vtkMRMLNode*)));
    emit recordEvent(this->CurrentObject, "nodeAboutToBeRemoved", QString(node->GetName()));
    }
  else
    {
    emit recordEvent(this->CurrentObject, "nodeAboutToBeRemoved", "None");
    }
}

// ----------------------------------------------------------------------------
void qMRMLNodeComboBoxEventTranslator::onCurrentNodeRenamed(const QString& newName)
{
  emit recordEvent(this->CurrentObject, "nodeRenamed", newName);
}
