#include "qSlicerIOOptions.h"

//------------------------------------------------------------------------------
qSlicerIOOptions::qSlicerIOOptions()
{
}

//------------------------------------------------------------------------------
qSlicerIOOptions::~qSlicerIOOptions()
{
}

//------------------------------------------------------------------------------
const qSlicerIO::IOProperties& qSlicerIOOptions::properties()const
{
  return this->Properties;
}
