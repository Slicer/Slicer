/**
 * @file  ProgressUpdateGUI.h
 * @brief Abstract class used to post progress messages and status bar updates
 *
 */
/*
 * Original Author: Kevin Teich
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2007/05/24 11:09:07 $
 *    $Revision: 1.2 $
 *
 * Copyright (C) 2007,
 * The General Hospital Corporation (Boston, MA).
 * All rights reserved.
 *
 * Distribution, usage and copying of this software is covered under the
 * terms found in the License Agreement file named 'COPYING' found in the
 * FreeSurfer source code root directory, and duplicated here:
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferOpenSourceLicense
 *
 * General inquiries: freesurfer@nmr.mgh.harvard.edu
 * Bug reports: analysis-bugs@nmr.mgh.harvard.edu
 *
 */

#ifndef ProgressUpdateGUI_H
#define ProgressUpdateGUI_H

#include <QdecConfigure.h>
#include "vtkQdecWin32Header.h"

//#include "vtkQdec.h"

class VTK_Qdec_EXPORT ProgressUpdateGUI {
public:
  virtual ~ProgressUpdateGUI () {};
  virtual void BeginActionWithProgress ( const char * isTitle ) = 0;
  virtual void UpdateProgressMessage ( const char* isMessage ) = 0;
  virtual void UpdateProgressPercent ( float iPercent ) = 0; // 0 - 100
  virtual void EndActionWithProgress () = 0;
};

#endif // ProgressUpdateGUI_H
