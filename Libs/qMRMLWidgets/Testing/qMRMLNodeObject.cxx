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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

/// Qt includes
#include <QApplication>
#include <QDebug>

/// qMRML includes
#include "qMRMLNodeObject.h"

/// VTK includes
#include <vtkTimerLog.h>

//-----------------------------------------------------------------------------
qMRMLNodeObject::qMRMLNodeObject(vtkMRMLNode* node, QObject* parent)
  : QObject(parent)
{
  this->Node = node;
  this->ProcessEvents = true;
  this->Message = QString(node ? node->GetName() : "");
}

//-----------------------------------------------------------------------------
void qMRMLNodeObject::modify()
{
  vtkTimerLog* timer = vtkTimerLog::New();
  timer->StartTimer();
  this->Node->Modified();
  if (this->ProcessEvents)
    {
    QApplication::processEvents();
    }
  timer->StopTimer();
  qDebug() << this->Message << " modified: " << timer->GetElapsedTime() << "seconds. FPS:" << 1. / timer->GetElapsedTime();
  timer->Delete();
}

//-----------------------------------------------------------------------------
void qMRMLNodeObject::setProcessEvents(bool process)
{
  this->ProcessEvents = process;
}

//-----------------------------------------------------------------------------
bool qMRMLNodeObject::processEvents()const
{
  return this->ProcessEvents;
}

//-----------------------------------------------------------------------------
void qMRMLNodeObject::setMessage(const QString& message)
{
  this->Message = message;
}

//-----------------------------------------------------------------------------
QString qMRMLNodeObject::message()const
{
  return this->Message;
}
