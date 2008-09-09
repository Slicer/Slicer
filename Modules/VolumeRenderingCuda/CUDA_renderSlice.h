#ifndef CUDA_RENDERSLICE_H
#define CUDA_RENDERSLICE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "cudaRendererInformation.h"
#include "cudaVolumeInformation.h"

extern "C"
void prepareShadeFieldSlice(cudaRendererInformation& renInfo, cudaVolumeInformation& volInfo);

extern "C"
void deleteShadeFieldSlice(cudaRendererInformation& renInfo, cudaVolumeInformation& volInfo);

extern "C"
void CUDArenderSlice_doRender(cudaRendererInformation& renInfo, cudaVolumeInformation& volInfo);

#endif
