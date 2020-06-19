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

#ifndef __qSlicerStyle_h
#define __qSlicerStyle_h

// CTK includes
#include <ctkProxyStyle.h>

// Slicer includes
#include "qSlicerBaseQTGUIExport.h"

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerStyle : public ctkProxyStyle
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkProxyStyle Superclass;

  /// Constructors
  qSlicerStyle();
  ~qSlicerStyle() override;

  SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                           const QPoint &pt, const QWidget *widget) const override;

  int pixelMetric(PixelMetric metric, const QStyleOption * option = nullptr,
                          const QWidget * widget = nullptr)const override;

  QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                               SubControl subControl, const QWidget *widget) const override;
  QPalette standardPalette()const override;
  QPalette standardLightPalette()const;
  QPalette standardDarkPalette()const;
  int styleHint(StyleHint hint, const QStyleOption *opt, const QWidget *widget,
                        QStyleHintReturn *returnData) const override;

  /// Behavior of widgets can be tweaked if an event filter is installed on a
  /// widget or application.
  /// If activated, the filter:
  ///  * prevents widgets to receive wheel events when they are in a scroll area
  ///    with a visible scrollbar.
  bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif

