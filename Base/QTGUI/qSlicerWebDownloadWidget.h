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

  This file was originally developed by Steve Pieper, Isomics, Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerWebDownloadWidget_h
#define __qSlicerWebDownloadWidget_h

// Qt includes
#include <QDialog>
class QWebEngineDownloadItem;

// CTK
class ctkPathLineEdit;

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerWebDownloadWidget
  : public QDialog
{
  Q_OBJECT

public:

  /// Constructor
  explicit qSlicerWebDownloadWidget(QWidget *parent = nullptr);

public slots:

  void handleDownload(QWebEngineDownloadItem* download);

};

#endif
