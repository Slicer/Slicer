/**
 * @file  ProgressUpdateGUI.h
 * @brief Abstract class used to post progress messages and status bar updates
 *
 */
/*
 * Original Author: Kevin Teich
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
  virtual void UpdateProgressPercent ( float iPercent ) = 0; /// 0 - 100
  virtual void EndActionWithProgress () = 0;
};

#endif /// ProgressUpdateGUI_H
