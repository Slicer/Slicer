#ifndef CUDA_RENDERSINGLESLICE_H
#define CUDA_RENDERSINGLESLICE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "cudaRendererInformation.h"
#include "cudaVolumeInformation.h"

extern "C"
void prepareShadeFieldSingleSlice(cudaRendererInformation& renInfo, cudaVolumeInformation& volInfo);

extern "C"
void deleteShadeFieldSingleSlice(cudaRendererInformation& renInfo, cudaVolumeInformation& volInfo);

extern "C"
void CUDArenderSingleSlice_doRender(cudaRendererInformation& renInfo, cudaVolumeInformation& volInfo);

#endif
