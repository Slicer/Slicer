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

#ifndef __qSlicerAppMainWindow_h
#define __qSlicerAppMainWindow_h

// Slicer includes
#include "qSlicerAppExport.h"
#include "qSlicerMainWindow.h"

class qSlicerAppMainWindowPrivate;

class Q_SLICER_APP_EXPORT qSlicerAppMainWindow : public qSlicerMainWindow
{
  Q_OBJECT
public:
  typedef qSlicerMainWindow Superclass;

  qSlicerAppMainWindow(QWidget *parent=nullptr);
  ~qSlicerAppMainWindow() override;

public slots:
  void on_HelpKeyboardShortcutsAction_triggered();
  void on_HelpBrowseTutorialsAction_triggered();
  void on_HelpInterfaceDocumentationAction_triggered();
  void on_HelpSlicerPublicationsAction_triggered();
  void on_HelpVisualBlogAction_triggered();

  void on_HelpReportBugOrFeatureRequestAction_triggered();
  void on_HelpAboutSlicerAppAction_triggered();

protected:
  qSlicerAppMainWindow(qSlicerAppMainWindowPrivate* pimpl, QWidget* parent);

private:
  Q_DECLARE_PRIVATE(qSlicerAppMainWindow);
  Q_DISABLE_COPY(qSlicerAppMainWindow);
};

#endif
