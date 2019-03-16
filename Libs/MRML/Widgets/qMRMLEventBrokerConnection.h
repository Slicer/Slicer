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

#ifndef __qMRMLEventBrokerConnection_h
#define __qMRMLEventBrokerConnection_h

// CTK includes
#include <ctkVTKConnection.h>
#include <ctkVTKObjectEventsObserver.h>

// MRMLWidgets includes
#include "qMRMLWidgetsExport.h"

class QMRML_WIDGETS_EXPORT qMRMLEventBrokerConnection: public ctkVTKConnection
{
Q_OBJECT

public:
  typedef ctkVTKConnection Superclass;
  explicit qMRMLEventBrokerConnection(QObject* parent);
  ~qMRMLEventBrokerConnection() override;

protected:
  void addObserver(vtkObject* caller, unsigned long vtk_event, vtkCallbackCommand* callback, float priority=0.0f) override;
  void removeObserver(vtkObject* caller, unsigned long vtk_event, vtkCallbackCommand* callback) override;

private:
  Q_DISABLE_COPY(qMRMLEventBrokerConnection);
};

//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLConnectionFactory: public ctkVTKConnectionFactory
{
public:
  ctkVTKConnection* createConnection(ctkVTKObjectEventsObserver*)const override;
};

#endif
