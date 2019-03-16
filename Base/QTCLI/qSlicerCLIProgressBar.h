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
#include <QMetaType>
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// Slicer includes
#include "qSlicerBaseQTCLIExport.h"

class vtkMRMLCommandLineModuleNode;
class qSlicerCLIProgressBarPrivate;

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIProgressBar : public QWidget
{
  Q_OBJECT
  Q_ENUMS(Visibility)
  QVTK_OBJECT

  /// This property controls how the module name is visible.
  /// AlwaysHidden by default.
  /// \sa nameVisibility(), setNameVisibility(),
  /// nameVisibility
  Q_PROPERTY(Visibility nameVisibility READ nameVisibility WRITE setNameVisibility)
  /// This property controls how the status label is visible.
  /// AlwaysVisible by default.
  /// \sa statusVisibility(), setStatusVisibility(),
  /// statusVisibility
  Q_PROPERTY(Visibility statusVisibility READ statusVisibility WRITE setStatusVisibility)
  /// This property controls how the progress bar is visible.
  /// VisibleAfterCompletion by default.
  /// \sa progressVisibility(), setProgressVisibility(),
  /// progressVisibility
  Q_PROPERTY(Visibility progressVisibility READ progressVisibility WRITE setProgressVisibility)
public:

  typedef QWidget Superclass;
  qSlicerCLIProgressBar(QWidget *parent=nullptr);
  ~qSlicerCLIProgressBar() override;

  /// Get the \a commandLineModuleNode
  Q_INVOKABLE vtkMRMLCommandLineModuleNode * commandLineModuleNode()const;

  /// Visibility behavior of the GUI elements of the CLI progress bar.
  enum Visibility
  {
    AlwaysHidden = 0,
    AlwaysVisible,
    HiddenWhenIdle,
    VisibleAfterCompletion
  };

  /// Visibility of the module name.
  /// \sa nameVisibility
  Visibility nameVisibility()const;
  /// Visibility of the status label.
  /// \sa statusVisibility
  Visibility statusVisibility()const;
  /// Visibility of the progress bar.
  /// \sa progressVisiblity
  Visibility progressVisibility()const;

public slots:

  /// Set the \a commandLineModuleNode
  void setCommandLineModuleNode(vtkMRMLCommandLineModuleNode* commandLineModuleNode);

  /// Set the module name visibility
  /// \sa nameVisibility
  void setNameVisibility(qSlicerCLIProgressBar::Visibility visibility);

  /// Set the status label visibility
  /// \sa statusVisibility
  void setStatusVisibility(qSlicerCLIProgressBar::Visibility visibility);

  /// Set the progress bar visibility
  /// \sa progressVisibility
  void setProgressVisibility(qSlicerCLIProgressBar::Visibility visibility);

protected slots:

  /// Update the ui base on the command line module node
  void updateUiFromCommandLineModuleNode(vtkObject* commandLineModuleNode);

  /// Update the ui base on the command line module node
  void showDetails(bool show);

protected:

  QScopedPointer<qSlicerCLIProgressBarPrivate> d_ptr;

private:

  Q_DECLARE_PRIVATE(qSlicerCLIProgressBar);
  Q_DISABLE_COPY(qSlicerCLIProgressBar);

};


#endif
