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

#ifndef __qSlicerCLIProgressBar_h
#define __qSlicerCLIProgressBar_h

// Qt includes
#include <QScopedPointer>
#include <QWidget>
#include <QtGui/QGridLayout>

// CTK includes
#include <ctkVTKObject.h>

#include "qSlicerBaseQTCLIExport.h"
#include "qSlicerWidget.h"

class vtkMRMLCommandLineModuleNode;
class qSlicerCLIProgressBarPrivate;

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIProgressBar : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:

  typedef QWidget Superclass;
  //qSlicerCLIProgressBar(QWidget *parent=0,QVBoxLayout* _parentLayout=0);
  qSlicerCLIProgressBar(QWidget *parent=0);
  virtual ~qSlicerCLIProgressBar();

  /// Get the \a commandLineModuleNode
  Q_INVOKABLE vtkMRMLCommandLineModuleNode * commandLineModuleNode()const;

public slots:

  /// Set the \a commandLineModuleNode
  void setCommandLineModuleNode(vtkMRMLCommandLineModuleNode* commandLineModuleNode);

protected slots:

  /// Update the ui base on the command line module node
  void updateUiFromCommandLineModuleNode(vtkObject* commandLineModuleNode);

protected:

  QScopedPointer<qSlicerCLIProgressBarPrivate> d_ptr;

private:

  Q_DECLARE_PRIVATE(qSlicerCLIProgressBar);
  Q_DISABLE_COPY(qSlicerCLIProgressBar);

};

#endif
