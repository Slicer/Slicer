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

#ifndef __qSlicerExtensionsWizard_h
#define __qSlicerExtensionsWizard_h

// Qt includes
#include <QWizard>
#include <QWizardPage>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerExtensionsWizardPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExtensionsWizard
  : public QWizard
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWizard Superclass;

  /// Constructor
  explicit qSlicerExtensionsWizard(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerExtensionsWizard();

protected:
  QScopedPointer<qSlicerExtensionsWizardPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsWizard);
  Q_DISABLE_COPY(qSlicerExtensionsWizard);
};

#endif
