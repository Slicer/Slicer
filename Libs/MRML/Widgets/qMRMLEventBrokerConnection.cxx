/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// qMRML includes
#include "qMRMLEventBrokerConnection.h"

// MRML includes
#include <vtkEventBroker.h>

//-----------------------------------------------------------------------------
ctkVTKConnection* qMRMLConnectionFactory::createConnection(
  ctkVTKObjectEventsObserver* parent)const
{
  return new qMRMLEventBrokerConnection(parent);
}

//------------------------------------------------------------------------------
qMRMLEventBrokerConnection::qMRMLEventBrokerConnection(QObject* parent)
  : Superclass(parent)
{
}


//------------------------------------------------------------------------------
qMRMLEventBrokerConnection::~qMRMLEventBrokerConnection()
{
  this->disconnect();
}

//-----------------------------------------------------------------------------
void qMRMLEventBrokerConnection::addObserver(vtkObject* caller, unsigned long vtk_event,
  vtkCallbackCommand* callback, float priority)
{
  vtkEventBroker::GetInstance()->AddObservation(caller, vtk_event, nullptr, callback, priority);
}

//-----------------------------------------------------------------------------
void qMRMLEventBrokerConnection::removeObserver(vtkObject* caller, unsigned long vtk_event, vtkCallbackCommand* callback)
{
  vtkEventBroker::GetInstance()->RemoveObservations(caller, vtk_event, nullptr, callback);
}
