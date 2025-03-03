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

#ifndef __qMRMLColorModel_p_h
#define __qMRMLColorModel_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
class QStandardItemModel;

// qMRML includes
#include "qMRMLColorModel.h"

// MRML includes
class vtkMRMLColorNode;

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
// qMRMLColorModelPrivate
//------------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLColorModelPrivate
{
  Q_DECLARE_PUBLIC(qMRMLColorModel);
protected:
  qMRMLColorModel* const q_ptr;
public:
  qMRMLColorModelPrivate(qMRMLColorModel& object);
  virtual ~qMRMLColorModelPrivate();
  void init();

  void updateColumnCount();
  virtual int maxColumnId()const;

  vtkSmartPointer<vtkMRMLColorLogic>  ColorLogic;
  vtkSmartPointer<vtkCallbackCommand> CallBack;
  vtkSmartPointer<vtkMRMLColorNode>   MRMLColorNode;

  bool NoneEnabled{ false };

  int ColorColumn{ 0 };
  int OpacityColumn{ 1 };
  int LabelColumn{ 2 };
  int TerminologyColumn{ 3 };
  int CheckableColumn{ -1 };

  /// This flag allows to make sure that during updating widget from MRML,
  /// GUI updates will not trigger MRML node updates.
  bool IsUpdatingWidgetFromMRML{ false };
};

#endif
