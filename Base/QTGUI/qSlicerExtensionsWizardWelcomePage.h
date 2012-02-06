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

#ifndef __qSlicerExtensionsWizardWelcomePage_h
#define __qSlicerExtensionsWizardWelcomePage_h

// Qt includes
#include <QUuid>
#include <QWizardPage>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"
#include "qSlicerQListQVariantMapMetaType.h"

class qSlicerExtensionsWizardWelcomePagePrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExtensionsWizardWelcomePage
  : public QWizardPage
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWizardPage Superclass;

  /// Constructor
  explicit qSlicerExtensionsWizardWelcomePage(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerExtensionsWizardWelcomePage();

  virtual void initializePage();
  virtual bool isComplete()const;
  virtual bool validatePage();

public slots:
  void deleteTemporaryArchiveFiles();

protected slots:
  void onExtensionInfosReceived(const QUuid& queryUuid, const QList<QVariantMap>& extensionInfos);

protected:
  QScopedPointer<qSlicerExtensionsWizardWelcomePagePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsWizardWelcomePage);
  Q_DISABLE_COPY(qSlicerExtensionsWizardWelcomePage);
};

#endif
