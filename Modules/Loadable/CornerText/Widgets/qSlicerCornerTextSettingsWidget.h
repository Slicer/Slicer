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

==============================================================================*/

#ifndef __qSlicerCornerTextSettingsWidget_h
#define __qSlicerCornerTextSettingsWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include "ctkPimpl.h"
#include "ctkVTKObject.h"

// MRML includes
#include "qMRMLWidget.h"
#include "vtkMRMLCornerTextLogic.h"
#include "vtkMRMLLayoutLogic.h"
#include "vtkMRMLScene.h"

// Settings Widgets includes
#include "qSlicerCornerTextModuleWidgetsExport.h"

class qSlicerCornerTextSettingsWidgetPrivate;

class Q_SLICER_MODULE_CORNERTEXT_WIDGETS_EXPORT qSlicerCornerTextSettingsWidget
  : public qMRMLWidget // so we can implement setMRMLScene
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  qSlicerCornerTextSettingsWidget(QWidget *parent=0);
  ~qSlicerCornerTextSettingsWidget() override;

  vtkMRMLLayoutLogic* layoutLogic() const;
  void setLayoutLogic(vtkMRMLLayoutLogic*);

  vtkMRMLCornerTextLogic* cornerTextLogic() const;
  void setAndObserveCornerTextLogic(vtkMRMLCornerTextLogic*);

protected slots:

  void enableSliceViewAnnotations(bool enable);
  void setTopLeftCornerActive(bool enable);
  void setTopRightCornerActive(bool enable);
  void setBottomLeftCornerActive(bool enable);
  void setIncludeDisplayLevelsLte(int level);  // 1, 2, or 3
  void setFontFamily(const QString& fontFamily);
  void setFontSize(int fontSize);
  void setDICOMAnnotationsPersistence(bool enable);
  void restoreDefaults();

  void onLayoutLogicModifiedEvent();

  // To sync with logic when modified & when application starts up
  void updateWidgetFromCornerTextLogic();

protected:
  QScopedPointer<qSlicerCornerTextSettingsWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCornerTextSettingsWidget);
  Q_DISABLE_COPY(qSlicerCornerTextSettingsWidget);
};

#endif
