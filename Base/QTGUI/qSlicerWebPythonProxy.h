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

#ifndef __qSlicerWebPythonProxy_h
#define __qSlicerWebPythonProxy_h

// Qt includes
#include <QObject>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"


class Q_SLICER_BASE_QTGUI_EXPORT qSlicerWebPythonProxy
  : public QObject
{
  Q_OBJECT

public:

  /// Constructor
  explicit qSlicerWebPythonProxy(QObject *parent = nullptr);

public slots:

  /// Convenient function to execute python code from 
  /// the JavaScript (when an instance of this class
  /// is registered with the WebChannel).
  ///
  /// Internally Slicer's python manager is used to execute
  /// the passed code, and the resulting QVariant is returned
  /// as a string.
  ///
  /// As a precaution, the user is prompted to allow or disallow 
  /// running python code from web pages.
  ///
  /// \sa qSlicerWebWidget::initializeWebEngineProfile
  QString evalPython(const QString &python);

private:
  /// Keep track of user response to avoid going through ctk dialog to check
  bool isPythonEvaluationAllowed();
  bool pythonEvaluationAllowed;

};

#endif
