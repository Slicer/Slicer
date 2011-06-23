/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

#ifndef __qSlicerExtensionsWizardOverviewPage_h
#define __qSlicerExtensionsWizardOverviewPage_h

// Qt includes
#include <QWizardPage>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"
#include "qSlicerQListQVariantMapMetaType.h"

class QTreeWidgetItem;
class QNetworkReply;
class qSlicerExtensionsWizardOverviewPagePrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExtensionsWizardOverviewPage
  : public QWizardPage
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWizardPage Superclass;

  /// Constructor
  explicit qSlicerExtensionsWizardOverviewPage(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerExtensionsWizardOverviewPage();

  virtual void initializePage();
  virtual bool validatePage();

public slots:
  void installSelectedItems();
  void uninstallSelectedItems();

protected slots:
  void onItemClicked(QTreeWidgetItem* item, int column);
  void downloadFinished(QNetworkReply*);

protected:
  QScopedPointer<qSlicerExtensionsWizardOverviewPagePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsWizardOverviewPage);
  Q_DISABLE_COPY(qSlicerExtensionsWizardOverviewPage);
};

#endif
