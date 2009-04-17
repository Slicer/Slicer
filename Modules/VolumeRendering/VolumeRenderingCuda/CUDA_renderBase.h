#ifndef CUDA_RENDERBASE_H
#define CUDA_RENDERBASE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "cudaRendererInformation.h"
#include "cudaVolumeInformation.h"

extern "C"
void CUDArenderBase_calculateGradient(cudaVolumeInformation& volInfo);

extern "C"
void CUDArenderBase_doRender(cudaRendererInformation& renInfo, cudaVolumeInformation& volInfo);

#endif
