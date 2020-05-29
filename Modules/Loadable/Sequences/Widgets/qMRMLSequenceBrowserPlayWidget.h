/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 5R42CA137886
  It was then updated for the Markups module by Nicole Aucoin, BWH.

==============================================================================*/

#ifndef __qMRMLSequenceBrowserPlayWidget_h
#define __qMRMLSequenceBrowserPlayWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Slicer includes
#include "qMRMLWidget.h"

#include "qSlicerSequencesModuleWidgetsExport.h"

class qMRMLSequenceBrowserPlayWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLSequenceBrowserNode;

/// \ingroup Slicer_QtModules_Markups
class Q_SLICER_MODULE_SEQUENCES_WIDGETS_EXPORT qMRMLSequenceBrowserPlayWidget
: public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  qMRMLSequenceBrowserPlayWidget(QWidget *newParent = 0);
  ~qMRMLSequenceBrowserPlayWidget() override;

  /// Add a keyboard shortcut for play/pause button
  void setPlayPauseShortcut(QString keySequence);

  /// Add a keyboard shortcut for previous frame button
  void setPreviousFrameShortcut(QString keySequence);

  /// Add a keyboard shortcut for next frame button
  void setNextFrameShortcut(QString keySequence);

public slots:
  void setMRMLSequenceBrowserNode(vtkMRMLSequenceBrowserNode* browserNode);
  void setMRMLSequenceBrowserNode(vtkMRMLNode* browserNode);
  void setPlaybackEnabled(bool play);
  void setRecordingEnabled(bool play);
  void setPlaybackRateFps(double playbackRateFps);
  void setPlaybackLoopEnabled(bool loopEnabled);
  void onVcrFirst();
  void onVcrPrevious();
  void onVcrNext();
  void onVcrLast();
  void onVcrPlayPause();
  void onRecordSnapshot();

protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qMRMLSequenceBrowserPlayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSequenceBrowserPlayWidget);
  Q_DISABLE_COPY(qMRMLSequenceBrowserPlayWidget);

};

#endif
