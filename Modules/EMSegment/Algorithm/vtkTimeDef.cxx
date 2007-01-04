#include "vtkTimeDef.h"

#ifndef _WIN32
extern "C" {
  struct timeval preciseTimeEnd;
  struct timeval preciseTimeStart;
}
#endif

