/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  The signed distance field computation (jump flooding) and the ray casting
  are based on SlicerLive by Steve Pieper (https://github.com/pieper/SlicerLive,
  Apache License 2.0).

==============================================================================*/

#include "vtkSegmentationLabelmapSurfaceMapper.h"

// SegmentationCore includes
#include <vtkOrientedImageData.h>
#include <vtkOrientedImageDataResample.h>

// VTK includes
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLQuadHelper.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkOpenGLVertexArrayObject.h>
#include <vtkInformation.h>
#include <vtkIntArray.h>
#include <vtkOpenGLRenderPass.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkWeakPointer.h>
#include <vtkShaderProgram.h>
#include <vtkSmartPointer.h>
#include <vtkTextureObject.h>
#include <vtk_glad.h>

// STD includes
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace
{

// Distances are computed exactly up to this distance (in voxels) from the surface. Farther voxels are only known to be
// farther than this, which is enough for ray casting.
const int BandVoxels = 32;

// Standard deviation of the Gaussian smoothing for SmoothingFactor = 1, relative to the largest voxel size.
// The default SmoothingFactor (0.5) keeps the surface smooth without rounding off small details.
const double SmoothingSigmaPerFactor = 1.7;

//----------------------------------------------------------------------------
// Shader sources start with //VTK::System::Dec, which is replaced by #version: in OpenGL ES it must be on the first line.
// Shaders of the signed distance field computation. Each pass draws one slice (z = sliceZ) of a
// 3D texture of the padded grid (a quad covering the slice); a fragment computes one voxel of the slice.

const char* SliceCommonFS = R"(//VTK::System::Dec
//VTK::Output::Dec
in vec2 texCoord;
uniform vec3 gridDimensions;
uniform int sliceZ;
ivec3 gridSize() { return ivec3(gridDimensions + vec3(0.5)); }
ivec3 currentVoxel() { return ivec3(ivec2(texCoord * gridDimensions.xy), sliceZ); }
)";

// Whether a grid voxel is in a shown segment, from the labelmap. Used to compute the mask.
const char* LabelFS = R"(
uniform sampler3D labelTexture;
uniform sampler2D paletteTexture;
uniform vec3 labelDimensions;
uniform vec3 downsampling;
uniform int padding;
uniform int paletteSize;
uniform float labelScale;
bool isShownLabel(int label)
{
  if (label <= 0 || label >= paletteSize)
  {
    return false;
  }
  return texelFetch(paletteTexture, ivec2(label % 256, label / 256), 0).a > 0.0;
}
// A downsampled voxel is shown if any of the labelmap voxels in it is shown (so that small or thin painted regions,
// for example in a single slice, do not disappear).
// At most 4 labelmap voxels are checked along each axis: all of them if the downsampling factor is at most 4,
// evenly spaced samples (so some voxels may be missed) if it is larger.
bool isShownVoxel(ivec3 voxel)
{
  ivec3 labelSize = ivec3(labelDimensions + vec3(0.5));
  ivec3 factor = ivec3(downsampling + vec3(0.5));
  ivec3 stride = (factor + ivec3(3)) / 4;
  ivec3 labelVoxel = (voxel - ivec3(padding)) * factor;
  if (any(lessThan(voxel, ivec3(padding))) || any(greaterThanEqual(labelVoxel, labelSize)))
  {
    return false;
  }
  for (int k = 0; k < 4; k++)
  {
    for (int j = 0; j < 4; j++)
    {
      for (int i = 0; i < 4; i++)
      {
        ivec3 offset = ivec3(i, j, k) * stride;
        if (any(greaterThanEqual(offset, factor)))
        {
          continue;
        }
        ivec3 labelSample = min(labelVoxel + offset, labelSize - ivec3(1));
        if (isShownLabel(int(texelFetch(labelTexture, labelSample, 0).r * labelScale + 0.5)))
        {
          return true;
        }
      }
    }
  }
  return false;
}
)";

// Mask of the voxels that are inside the surface: voxels of shown segments, and voxels of gaps between them that are
// narrower than the smoothing (they would be crevices in the surface; smoothing would fill them, but not entirely).
const char* MaskFS = R"(
uniform vec3 gapVoxels;
void main()
{
  ivec3 voxel = currentVoxel();
  bool inside = isShownVoxel(voxel);
  for (int axis = 0; axis < 3 && !inside; axis++)
  {
    ivec3 step = ivec3(axis == 0 ? 1 : 0, axis == 1 ? 1 : 0, axis == 2 ? 1 : 0);
    int maximumGap = int(gapVoxels[axis] + 0.5);
    int plusSteps = 0;
    int minusSteps = 0;
    for (int i = 1; i <= 4; i++)
    {
      if (i > maximumGap)
      {
        break;
      }
      if (plusSteps == 0 && isShownVoxel(voxel + i * step))
      {
        plusSteps = i;
      }
      if (minusSteps == 0 && isShownVoxel(voxel - i * step))
      {
        minusSteps = i;
      }
    }
    inside = plusSteps > 0 && minusSteps > 0 && plusSteps + minusSteps - 1 <= maximumGap;
  }
  gl_FragData[0] = vec4(inside ? 1.0 : 0.0, 0.0, 0.0, 1.0);
}
)";

// Whether a grid voxel is inside the surface (from the mask)
const char* InsideFS = R"(
uniform sampler3D maskTexture;
bool isInside(ivec3 voxel)
{
  if (any(lessThan(voxel, ivec3(0))) || any(greaterThanEqual(voxel, gridSize())))
  {
    return false;
  }
  return texelFetch(maskTexture, voxel, 0).r > 0.5;
}
)";

// Seeds are stored as offsets from the voxel to its nearest boundary point, in half voxels, in an RGBA8 texture
// (renderable in OpenGL ES 3.0 without extensions, and a quarter of the memory of floating-point positions).
// Offsets are limited to +-63.5 voxels, which is more than the band where distances are computed (BandVoxels).
const char* SeedCodingFS = R"(
vec4 encodeSeed(vec3 offset)
{
  return vec4((round(2.0 * offset) + vec3(128.0)) / 255.0, 1.0);
}
vec3 decodeSeed(vec4 seed)
{
  return (round(seed.xyz * 255.0) - vec3(128.0)) * 0.5;
}
bool isValidSeed(vec4 seed)
{
  return seed.w > 0.5;
}
)";

// Seeds are points of the boundary of shown segments: voxels on both sides of the boundary place a seed
// on the nearest face that separates them from a voxel on the other side (one voxel thick structures
// have such faces on opposite sides, they must not cancel each other).
const char* SeedFS = R"(
uniform mat3 gridToPhysical;
void main()
{
  ivec3 voxel = currentVoxel();
  bool inside = isInside(voxel);
  vec3 offset = vec3(0.0);
  float nearestDistance = 1.0e30;
  for (int axis = 0; axis < 3; axis++)
  {
    ivec3 step = ivec3(axis == 0 ? 1 : 0, axis == 1 ? 1 : 0, axis == 2 ? 1 : 0);
    float faceDistance = 0.5 * length(gridToPhysical * vec3(step));
    if (faceDistance < nearestDistance && (isInside(voxel + step) != inside || isInside(voxel - step) != inside))
    {
      nearestDistance = faceDistance;
      offset = (isInside(voxel + step) != inside ? 0.5 : -0.5) * vec3(step);
    }
  }
  gl_FragData[0] = nearestDistance < 1.0e30 ? encodeSeed(offset) : vec4(0.0);
}
)";

// One jump flooding step: each voxel takes the nearest seed (in physical distance) of its 26 neighbors at stepSize.
const char* JumpFloodFS = R"(
uniform sampler3D seedTexture;
uniform int stepSize;
uniform mat3 gridToPhysical;
uniform vec3 regionMinimum;
uniform vec3 regionMaximum;
void main()
{
  ivec3 voxel = currentVoxel();
  ivec3 maxVoxel = gridSize() - ivec3(1);
  vec4 own = texelFetch(seedTexture, voxel, 0);
  vec3 bestOffset = decodeSeed(own);
  bool found = isValidSeed(own);
  float bestDistance = found ? length(gridToPhysical * bestOffset) : 1.0e30;
  for (int dz = -1; dz <= 1; dz++)
  {
    for (int dy = -1; dy <= 1; dy++)
    {
      for (int dx = -1; dx <= 1; dx++)
      {
        if (dx == 0 && dy == 0 && dz == 0)
        {
          continue;
        }
        ivec3 neighbor = clamp(voxel + ivec3(dx, dy, dz) * stepSize, ivec3(0), maxVoxel);
        if (any(lessThan(vec3(neighbor), regionMinimum)) || any(greaterThan(vec3(neighbor), regionMaximum)))
        {
          // seeds are only computed in the region
          continue;
        }
        vec4 seed = texelFetch(seedTexture, neighbor, 0);
        if (!isValidSeed(seed))
        {
          continue;
        }
        vec3 offset = vec3(neighbor - voxel) + decodeSeed(seed);
        if (any(greaterThan(abs(offset), vec3(63.5))))
        {
          continue;
        }
        float distanceToSeed = length(gridToPhysical * offset);
        if (distanceToSeed < bestDistance)
        {
          bestDistance = distanceToSeed;
          bestOffset = offset;
          found = true;
        }
      }
    }
  }
  gl_FragData[0] = found ? encodeSeed(bestOffset) : vec4(0.0);
}
)";

// Signed distance to the nearest boundary point (in mm, negative inside). Farther than the band where seeds
// are propagated, the distance is only known to be at least bandDistance.
const char* FinalizeFS = R"(
uniform sampler3D seedTexture;
uniform mat3 gridToPhysical;
uniform float bandDistance;
void main()
{
  ivec3 voxel = currentVoxel();
  vec4 seed = texelFetch(seedTexture, voxel, 0);
  float distanceToBoundary = isValidSeed(seed) ? min(length(gridToPhysical * decodeSeed(seed)), bandDistance) : bandDistance;
  gl_FragData[0] = vec4(isInside(voxel) ? -distanceToBoundary : distanceToBoundary, 0.0, 0.0, 1.0);
}
)";

// Separable Gaussian smoothing along one axis.
const char* SmoothFS = R"(
uniform sampler3D inputTexture;
uniform vec3 axis;
uniform int radius;
uniform float weights[16];
void main()
{
  ivec3 voxel = currentVoxel();
  ivec3 maxVoxel = gridSize() - ivec3(1);
  ivec3 axisStep = ivec3(axis + vec3(0.5));
  float sum = weights[0] * texelFetch(inputTexture, voxel, 0).r;
  for (int i = 1; i <= radius; i++)
  {
    sum += weights[i] * (texelFetch(inputTexture, clamp(voxel + axisStep * i, ivec3(0), maxVoxel), 0).r
      + texelFetch(inputTexture, clamp(voxel - axisStep * i, ivec3(0), maxVoxel), 0).r);
  }
  gl_FragData[0] = vec4(sum, 0.0, 0.0, 1.0);
}
)";

// Smoothing without shrinking: Gaussian smoothing (G) of a distance field moves curved surfaces inwards (by sigma^2/2 times
// the mean curvature) and removes thin structures, which "twicing" (2 G f - G G f) cancels to first order, while it still
// removes staircases (high frequencies). Small thin structures (for example painted in a single thick slice) may still
// disappear: the deepest voxels of each structure (where the original distance is a local minimum) are kept inside.
const char* CombineFS = R"(
uniform sampler3D originalTexture;
uniform sampler3D smoothedTexture;
uniform sampler3D smoothedTwiceTexture;
void main()
{
  ivec3 voxel = currentVoxel();
  ivec3 maxVoxel = gridSize() - ivec3(1);
  float smoothedDistance = 2.0 * texelFetch(smoothedTexture, voxel, 0).r - texelFetch(smoothedTwiceTexture, voxel, 0).r;
  float originalDistance = texelFetch(originalTexture, voxel, 0).r;
  if (originalDistance < 0.0 && smoothedDistance > 0.5 * originalDistance)
  {
    bool deepest = true;
    for (int dz = -1; dz <= 1 && deepest; dz++)
    {
      for (int dy = -1; dy <= 1 && deepest; dy++)
      {
        for (int dx = -1; dx <= 1 && deepest; dx++)
        {
          ivec3 neighbor = clamp(voxel + ivec3(dx, dy, dz), ivec3(0), maxVoxel);
          deepest = texelFetch(originalTexture, neighbor, 0).r >= originalDistance - 1.0e-3;
        }
      }
    }
    if (deepest)
    {
      smoothedDistance = 0.5 * originalDistance;
    }
  }
  gl_FragData[0] = vec4(smoothedDistance, 0.0, 0.0, 1.0);
}
)";

// Colors of the segments at the voxels of the grid, for coloring the surface (alpha = 0 where there is no shown segment
// near). Computing them once, rather than at every surface point of every frame, makes rendering several times faster.
const char* ColorFS = R"(
// Color and opacity of the shown label at a grid position, or alpha = 0 if there is no shown label there.
vec4 labelColorAtGrid(vec3 gridPosition)
{
  ivec3 labelSize = ivec3(labelDimensions + vec3(0.5));
  ivec3 factor = ivec3(downsampling + vec3(0.5));
  ivec3 voxel = ivec3(floor(gridPosition + vec3(0.5)));
  ivec3 labelVoxel = (voxel - ivec3(padding)) * factor;
  if (any(lessThan(voxel, ivec3(padding))) || any(greaterThanEqual(labelVoxel, labelSize)))
  {
    return vec4(0.0);
  }
  labelVoxel = min(labelVoxel + factor / 2, labelSize - ivec3(1));
  int label = int(texelFetch(labelTexture, labelVoxel, 0).r * labelScale + 0.5);
  if (label <= 0 || label >= paletteSize)
  {
    return vec4(0.0);
  }
  return texelFetch(paletteTexture, ivec2(label % 256, label / 256), 0);
}

// Color of a shown label in the labelmap voxels of a downsampled grid voxel (alpha = 0 if there is none).
// Samples the same labelmap voxels as isShownVoxel.
vec4 labelColorInBlock(ivec3 voxel)
{
  ivec3 labelSize = ivec3(labelDimensions + vec3(0.5));
  ivec3 factor = ivec3(downsampling + vec3(0.5));
  ivec3 stride = (factor + ivec3(3)) / 4;
  ivec3 labelVoxel = (voxel - ivec3(padding)) * factor;
  if (any(lessThan(voxel, ivec3(padding))) || any(greaterThanEqual(labelVoxel, labelSize)))
  {
    return vec4(0.0);
  }
  for (int k = 0; k < 4; k++)
  {
    for (int j = 0; j < 4; j++)
    {
      for (int i = 0; i < 4; i++)
      {
        ivec3 offset = ivec3(i, j, k) * stride;
        if (any(greaterThanEqual(offset, factor)))
        {
          continue;
        }
        int label = int(texelFetch(labelTexture, min(labelVoxel + offset, labelSize - ivec3(1)), 0).r * labelScale + 0.5);
        if (label > 0 && label < paletteSize)
        {
          vec4 color = texelFetch(paletteTexture, ivec2(label % 256, label / 256), 0);
          if (color.a > 0.0)
          {
            return color;
          }
        }
      }
    }
  }
  return vec4(0.0);
}

// Color of the segments at a grid position: colors of nearby voxels that are inside shown segments, weighted by their
// distance (in voxels), so that seams between segments are soft. Voxels behind thin segments get negligible weight.
vec4 segmentColor(vec3 gridPosition)
{
  ivec3 center = ivec3(floor(gridPosition + vec3(0.5)));
  vec4 sum = vec4(0.0);
  float sumWeights = 0.0;
  for (int dz = -1; dz <= 1; dz++)
  {
    for (int dy = -1; dy <= 1; dy++)
    {
      for (int dx = -1; dx <= 1; dx++)
      {
        ivec3 voxel = center + ivec3(dx, dy, dz);
        vec4 color = labelColorAtGrid(vec3(voxel));
        if (color.a > 0.0)
        {
          vec3 offset = vec3(voxel) - gridPosition;
          float weight = exp(-2.0 * dot(offset, offset));
          sum += weight * color;
          sumWeights += weight;
        }
      }
    }
  }
  if (sumWeights > 0.0)
  {
    return sum / sumWeights;
  }
  // Downsampled grid: the shown label may not be at the sampled labelmap voxel
  vec4 blockColor = labelColorInBlock(center);
  if (blockColor.a > 0.0)
  {
    return blockColor;
  }
  // Surface over a filled gap: the nearest shown voxels may be farther
  for (int dz = -2; dz <= 2; dz++)
  {
    for (int dy = -2; dy <= 2; dy++)
    {
      for (int dx = -2; dx <= 2; dx++)
      {
        ivec3 voxel = center + ivec3(dx, dy, dz);
        vec4 color = labelColorAtGrid(vec3(voxel));
        if (color.a > 0.0)
        {
          vec3 offset = vec3(voxel) - gridPosition;
          float weight = exp(-0.5 * dot(offset, offset));
          sum += weight * color;
          sumWeights += weight;
        }
      }
    }
  }
  return sumWeights > 0.0 ? sum / sumWeights : vec4(0.0);
}
void main()
{
  gl_FragData[0] = segmentColor(vec3(currentVoxel()));
}
)";

//----------------------------------------------------------------------------
// Ray casting shaders. A triangle that covers the viewport is drawn (without vertex buffers, from gl_VertexID); each
// fragment casts the ray of its pixel through the box of the grid. Rays do not depend on the clipping range of the
// camera (drawing the faces of the box would cast no rays where they are beyond the far clipping plane).

const char* SurfaceVS = R"(//VTK::System::Dec
out vec2 pixelPositionDC;
void main()
{
  vec2 corner = vec2(float((gl_VertexID & 1) * 4 - 1), float((gl_VertexID & 2) * 2 - 1));
  pixelPositionDC = corner;
  // the depth of the fragments is set by the fragment shader; near the far plane, as back faces of a box would be
  gl_Position = vec4(corner, 0.999, 1.0);
}
)";

const char* SurfaceFS = R"(//VTK::System::Dec
//VTK::Output::Dec
in vec2 pixelPositionDC;
uniform sampler3D distanceTexture;
uniform sampler3D colorTexture;
uniform sampler2D opaqueDepthTexture;
uniform mat4 MCDCMatrix;
uniform mat4 DCMCMatrix;
uniform mat4 MCVCMatrix;
uniform mat3 normalMatrix;
uniform mat3 gridToPhysical;
uniform vec3 eyePositionMC;
uniform vec3 viewDirectionMC;
uniform int parallelProjection;
uniform vec3 boxMin;
uniform vec3 boxMax;
uniform vec3 mcToGridScale;
uniform vec3 mcToGridOffset;
uniform vec3 gridDimensions;
uniform float mcPerMm;
uniform float minimumStep;
uniform float ambientIntensity;
uniform float diffuseIntensity;
uniform float specularIntensity;
uniform float specularPower;
uniform vec3 specularColor;
uniform float opacity;
uniform int numberOfClippingPlanes;
uniform vec4 clippingPlanesMC[16];
uniform int keepWhereAnyPlaneKeeps;
uniform int capClippedSurface;
uniform int clippingOutline;
uniform vec3 outlineColor;
uniform float outlineWidth;
uniform float capOpacity;
uniform int stopAtOpaqueDepth;
uniform vec2 viewportOrigin;
uniform vec2 viewportSize;
// Size of a pixel of the viewport in pixels of the image that rays are cast for (ImageSampleDistance)
uniform vec2 fullResolutionScale;
//VTK::Light::Dec
//VTK::DepthPeeling::Dec

float distanceAtGrid(vec3 gridPosition)
{
  return texture(distanceTexture, (gridPosition + vec3(0.5)) / gridDimensions).r;
}

float distanceAt(vec3 positionMC)
{
  return distanceAtGrid(positionMC * mcToGridScale + mcToGridOffset);
}

// Color and opacity of the segments at a surface point (gridPosition in grid voxels), interpolated from the colors
// precomputed at the voxels (see ColorFS). Voxels without a color (far from shown segments) are ignored.
vec4 segmentColor(vec3 gridPosition)
{
  ivec3 maxVoxel = ivec3(gridDimensions + vec3(0.5)) - ivec3(1);
  vec3 baseVoxel = floor(gridPosition);
  vec3 fraction = gridPosition - baseVoxel;
  vec4 sum = vec4(0.0);
  float sumWeights = 0.0;
  for (int corner = 0; corner < 8; corner++)
  {
    ivec3 offset = ivec3(corner & 1, (corner >> 1) & 1, corner >> 2);
    vec4 color = texelFetch(colorTexture, clamp(ivec3(baseVoxel) + offset, ivec3(0), maxVoxel), 0);
    if (color.a > 0.0)
    {
      vec3 weights = mix(vec3(1.0) - fraction, fraction, vec3(offset));
      float weight = max(weights.x * weights.y * weights.z, 1.0e-4);
      sum += weight * color;
      sumWeights += weight;
    }
  }
  return sumWeights > 0.0 ? sum / sumWeights : vec4(0.5, 0.5, 0.5, 1.0);
}

// Lighting (same model as VTK's polygonal mappers)
vec3 computeLighting(vec3 baseColor, vec3 normalVC, vec3 viewDirectionVC)
{
  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);
  //VTK::Light::Impl
  return ambientIntensity * baseColor + diffuseIntensity * diffuse * baseColor + specularIntensity * specular * specularColor;
}

// Point of the ray on the far clipping plane (t = 0); points nearer to the eye have negative t
vec3 rayOriginMC;

vec4 accumulatedColor = vec4(0.0); // premultiplied
float firstDepth = -1.0;
// Position and normal of the frontmost surface point in view coordinates (for screen-space ambient occlusion)
vec3 firstPositionVC = vec3(0.0);
vec3 firstNormalVC = vec3(0.0);

// Composite a surface point front to back. gradientMC is the gradient of a function that increases outwards.
void compositeSurface(vec3 positionMC, vec3 gradientMC, vec4 baseColor)
{
  float alpha = baseColor.a;
  if (alpha <= 0.0)
  {
    return;
  }
  vec4 positionVC = MCVCMatrix * vec4(positionMC, 1.0);
  vec3 viewDirectionVC = parallelProjection == 1 ? vec3(0.0, 0.0, 1.0) : normalize(-positionVC.xyz);
  vec3 normalVC = normalize(normalMatrix * gradientMC);
  if (dot(normalVC, viewDirectionVC) < 0.0)
  {
    normalVC = -normalVC;
  }
  vec3 color = clamp(computeLighting(baseColor.rgb, normalVC, viewDirectionVC), 0.0, 1.0);
  accumulatedColor += (1.0 - accumulatedColor.a) * vec4(color * alpha, alpha);
  if (firstDepth < 0.0)
  {
    vec4 positionDC = MCDCMatrix * vec4(positionMC, 1.0);
    firstDepth = clamp(0.5 * positionDC.z / positionDC.w + 0.5, 0.0, 1.0);
    firstPositionVC = positionVC.xyz / positionVC.w;
    firstNormalVC = normalVC;
  }
}

// Composite the surface of the segments (zero level set of the distance field) at a point
void compositeSegmentSurface(vec3 positionMC, vec3 rayDirection)
{
  vec3 gridPosition = positionMC * mcToGridScale + mcToGridOffset;
  // Half-voxel differences: structures may be only one voxel thick
  vec3 gradientGrid = vec3(
    distanceAtGrid(gridPosition + vec3(0.5, 0.0, 0.0)) - distanceAtGrid(gridPosition - vec3(0.5, 0.0, 0.0)),
    distanceAtGrid(gridPosition + vec3(0.0, 0.5, 0.0)) - distanceAtGrid(gridPosition - vec3(0.0, 0.5, 0.0)),
    distanceAtGrid(gridPosition + vec3(0.0, 0.0, 0.5)) - distanceAtGrid(gridPosition - vec3(0.0, 0.0, 0.5)));
  if (length(gradientGrid) < 1.0e-4)
  {
    gradientGrid = -rayDirection * mcToGridScale;
  }
  vec4 color = segmentColor(gridPosition);
  compositeSurface(positionMC, gradientGrid * mcToGridScale, vec4(color.rgb, color.a * opacity));
}

// Composite a point of uniform color (not lit) front to back
void compositeFlat(vec3 positionMC, vec4 color)
{
  accumulatedColor += (1.0 - accumulatedColor.a) * vec4(color.rgb * color.a, color.a);
  if (firstDepth < 0.0)
  {
    vec4 positionDC = MCDCMatrix * vec4(positionMC, 1.0);
    firstDepth = clamp(0.5 * positionDC.z / positionDC.w + 0.5, 0.0, 1.0);
    vec4 positionVC = MCVCMatrix * vec4(positionMC, 1.0);
    firstPositionVC = positionVC.xyz / positionVC.w;
    firstNormalVC = vec3(0.0, 0.0, 1.0);
  }
}

// Composite the outline of the segments on a clipping plane: the points of the plane that are closer to the surface of
// the segments (measured in the plane) than half of the outline width. Returns true if the point is on the outline.
bool compositeOutline(vec3 positionMC, vec4 plane)
{
  // Size of a pixel at the point, in millimeters (view coordinates are in millimeters)
  vec4 positionDC = MCDCMatrix * vec4(positionMC, 1.0);
  positionDC /= positionDC.w;
  vec4 neighborMC = DCMCMatrix * vec4(positionDC.x + 2.0 / (viewportSize.x * fullResolutionScale.x), positionDC.yzw);
  vec4 neighborVC = MCVCMatrix * vec4(neighborMC.xyz / neighborMC.w, 1.0);
  vec4 positionVC = MCVCMatrix * vec4(positionMC, 1.0);
  float pixelSizeMm = length(neighborVC.xyz / neighborVC.w - positionVC.xyz / positionVC.w);
  // Distance from the curve in the plane: distance from the surface divided by the in-plane part of its gradient
  // (the normal matrix maps gradients from model to view coordinates, which are in millimeters)
  vec3 gridPosition = positionMC * mcToGridScale + mcToGridOffset;
  float distanceMm = distanceAtGrid(gridPosition);
  vec3 gradientGrid = vec3(
    distanceAtGrid(gridPosition + vec3(0.5, 0.0, 0.0)) - distanceAtGrid(gridPosition - vec3(0.5, 0.0, 0.0)),
    distanceAtGrid(gridPosition + vec3(0.0, 0.5, 0.0)) - distanceAtGrid(gridPosition - vec3(0.0, 0.5, 0.0)),
    distanceAtGrid(gridPosition + vec3(0.0, 0.0, 0.5)) - distanceAtGrid(gridPosition - vec3(0.0, 0.0, 0.5)));
  vec3 gradientVC = normalMatrix * (gradientGrid * mcToGridScale);
  vec3 planeNormalVC = normalize(normalMatrix * plane.xyz);
  float inPlaneGradient = max(length(gradientVC - dot(gradientVC, planeNormalVC) * planeNormalVC), 0.2);
  if (abs(distanceMm) / inPlaneGradient > 0.5 * max(outlineWidth, fullResolutionScale.x) * pixelSizeMm)
  {
    return false;
  }
  compositeFlat(positionMC, vec4(outlineColor, opacity));
  return true;
}

// Composite the cap of the segments where a clipping plane cuts them
void compositeCap(vec3 positionMC, vec4 plane)
{
  vec3 gridPosition = positionMC * mcToGridScale + mcToGridOffset;
  vec4 color = segmentColor(gridPosition);
  compositeSurface(positionMC, plane.xyz, vec4(color.rgb, color.a * opacity * capOpacity));
}

)"
                        R"(// Composite the segments along the ray between tFrom and tTo (sphere tracing). fromPlane and toPlane are the clipping
// planes that cut the ray there (-1 if none), where the cut is capped and outlined.
void compositeKeptPart(vec3 rayDirection, float tFrom, float tTo, int fromPlane, int toPlane)
{
  if (tFrom >= tTo || accumulatedColor.a > 0.99)
  {
    return;
  }
  float t = tFrom;
  float previousDistance = distanceAt(rayOriginMC + t * rayDirection);
  bool onStartOutline = fromPlane >= 0 && clippingOutline == 1 && compositeOutline(rayOriginMC + t * rayDirection, clippingPlanesMC[fromPlane]);
  if (!onStartOutline && previousDistance <= 0.0 && fromPlane >= 0 && capClippedSurface == 1)
  {
    compositeCap(rayOriginMC + t * rayDirection, clippingPlanesMC[fromPlane]);
  }
  for (int i = 0; i < 4096; i++)
  {
    if (t >= tTo || accumulatedColor.a > 0.99)
    {
      break;
    }
    float previousT = t;
    t = min(t + max(abs(previousDistance) * mcPerMm * 0.9, minimumStep), tTo);
    float currentDistance = distanceAt(rayOriginMC + t * rayDirection);
    if ((previousDistance > 0.0) != (currentDistance > 0.0))
    {
      // Refine the crossing by bisection
      float tA = previousT;
      float tB = t;
      bool outsideA = previousDistance > 0.0;
      for (int j = 0; j < 8; j++)
      {
        float tMiddle = 0.5 * (tA + tB);
        if ((distanceAt(rayOriginMC + tMiddle * rayDirection) > 0.0) == outsideA)
        {
          tA = tMiddle;
        }
        else
        {
          tB = tMiddle;
        }
      }
      compositeSegmentSurface(rayOriginMC + 0.5 * (tA + tB) * rayDirection, rayDirection);
    }
    previousDistance = currentDistance;
  }
  bool onEndOutline = toPlane >= 0 && clippingOutline == 1 && accumulatedColor.a <= 0.99
                      && compositeOutline(rayOriginMC + tTo * rayDirection, clippingPlanesMC[toPlane]);
  if (!onEndOutline && previousDistance <= 0.0 && toPlane >= 0 && capClippedSurface == 1 && accumulatedColor.a <= 0.99)
  {
    compositeCap(rayOriginMC + tTo * rayDirection, clippingPlanesMC[toPlane]);
  }
}

void main()
{
  vec4 nearMC = DCMCMatrix * vec4(pixelPositionDC, -1.0, 1.0);
  vec4 farMC = DCMCMatrix * vec4(pixelPositionDC, 1.0, 1.0);
  rayOriginMC = farMC.xyz / farMC.w;
  vec3 rayDirection = parallelProjection == 1 ? viewDirectionMC : normalize(rayOriginMC - nearMC.xyz / nearMC.w);
  rayDirection = mix(rayDirection, vec3(1.0e-6), lessThan(abs(rayDirection), vec3(1.0e-6)));
  vec3 inverseDirection = 1.0 / rayDirection;
  vec3 t0 = (boxMin - rayOriginMC) * inverseDirection;
  vec3 t1 = (boxMax - rayOriginMC) * inverseDirection;
  vec3 tNearAxis = min(t0, t1);
  vec3 tFarAxis = max(t0, t1);
  float tNear = max(max(tNearAxis.x, tNearAxis.y), tNearAxis.z);
  float tFar = min(min(tFarAxis.x, tFarAxis.y), tFarAxis.z);
  // The part of the ray in the box (not behind the eye)
  if (tNear >= tFar)
  {
    discard;
  }
  float tRayStart = tNear;
  if (parallelProjection == 0)
  {
    tRayStart = max(tRayStart, dot(eyePositionMC - rayOriginMC, rayDirection));
  }
  float tRayEnd = tFar;

  // Translucent surfaces are not hidden by the depth test, the ray stops at opaque geometry
  if (stopAtOpaqueDepth == 1)
  {
    vec2 pixel = gl_FragCoord.xy - viewportOrigin;
    float opaqueDepth = texelFetch(opaqueDepthTexture, ivec2(pixel * fullResolutionScale), 0).r;
    if (opaqueDepth < 1.0)
    {
      vec4 opaquePositionMC = DCMCMatrix * vec4(2.0 * pixel / viewportSize - 1.0, 2.0 * opaqueDepth - 1.0, 1.0);
      float tOpaque = dot(opaquePositionMC.xyz / opaquePositionMC.w - rayOriginMC, rayDirection);
      tRayEnd = min(tRayEnd, tOpaque);
    }
  }
  if (tRayStart >= tRayEnd)
  {
    discard;
  }

  if (keepWhereAnyPlaneKeeps == 0 || numberOfClippingPlanes == 0)
  {
    // The kept region is where all plane functions are positive: an interval along the ray
    float tStart = tRayStart;
    float tEnd = tRayEnd;
    int startPlane = -1;
    int endPlane = -1;
    for (int i = 0; i < numberOfClippingPlanes; i++)
    {
      vec4 plane = clippingPlanesMC[i];
      float rate = dot(plane.xyz, rayDirection);
      float value = dot(plane.xyz, rayOriginMC) + plane.w;
      if (abs(rate) < 1.0e-12)
      {
        if (value < 0.0)
        {
          discard;
        }
        continue;
      }
      float tPlane = -value / rate;
      if (rate > 0.0 && tPlane > tStart)
      {
        tStart = tPlane;
        startPlane = i;
      }
      else if (rate < 0.0 && tPlane < tEnd)
      {
        tEnd = tPlane;
        endPlane = i;
      }
    }
    compositeKeptPart(rayDirection, tStart, tEnd, startPlane, endPlane);
  }
  else
  {
    // The kept region is where any plane function is positive: the ray is clipped only where all of them are negative,
    // an interval along the ray, and kept before and after it
    float tClipStart = -1.0e30;
    float tClipEnd = 1.0e30;
    int clipStartPlane = -1;
    int clipEndPlane = -1;
    bool clipped = true;
    for (int i = 0; i < numberOfClippingPlanes; i++)
    {
      vec4 plane = clippingPlanesMC[i];
      float rate = dot(plane.xyz, rayDirection);
      float value = dot(plane.xyz, rayOriginMC) + plane.w;
      if (abs(rate) < 1.0e-12)
      {
        clipped = clipped && value < 0.0;
        continue;
      }
      float tPlane = -value / rate;
      if (rate < 0.0 && tPlane > tClipStart)
      {
        tClipStart = tPlane;
        clipStartPlane = i;
      }
      else if (rate > 0.0 && tPlane < tClipEnd)
      {
        tClipEnd = tPlane;
        clipEndPlane = i;
      }
    }
    if (!clipped || tClipStart >= tClipEnd || tClipEnd <= tRayStart || tClipStart >= tRayEnd)
    {
      compositeKeptPart(rayDirection, tRayStart, tRayEnd, -1, -1);
    }
    else
    {
      compositeKeptPart(rayDirection, tRayStart, tClipStart, -1, clipStartPlane);
      compositeKeptPart(rayDirection, tClipEnd, tRayEnd, clipEndPlane, -1);
    }
  }
  if (accumulatedColor.a <= 0.0)
  {
    discard;
  }
  //SEG::Output::Impl
}
)";

// Output of the ray casting shader into the viewport, where the render passes (depth peeling, ambient occlusion) take it
const char* SurfaceOutputImpl = R"(gl_FragDepth = firstDepth;
  //VTK::DepthPeeling::PreColor
  gl_FragData[0] = vec4(accumulatedColor.rgb / accumulatedColor.a, accumulatedColor.a);
  //VTK::SSAO::Impl
  //VTK::DepthPeeling::Impl)";

// Output of the ray casting shader into an image of a lower resolution (ImageSampleDistance): color (premultiplied, so
// that it can be interpolated), depth (24 bits in RGB) and normal of the frontmost surface point. Only 8-bit color
// buffers are used: they are what every graphics card can render into (several 32-bit float ones together are not, on
// some mobile GPUs), and positions are computed from the depth.
const char* LowResolutionOutputImpl = R"(gl_FragData[0] = accumulatedColor;
  highp uint depthBits = uint(clamp(firstDepth, 0.0, 1.0) * 16777215.0 + 0.5);
  gl_FragData[1] = vec4(vec3(uvec3(depthBits >> 16u, depthBits >> 8u, depthBits) & uvec3(255u)) / 255.0, 1.0);
  gl_FragData[2] = vec4(0.5 * firstNormalVC + vec3(0.5), 1.0);)";

// Scales the image of a lower resolution up to the viewport: color is interpolated, the normal is that of the frontmost
// of the nearest image pixels that have a surface, and the position (and depth) is where the ray of the viewport pixel
// meets the plane of the surface there. Positions of the image pixels themselves would make every block of viewport
// pixels a flat step, which ambient occlusion takes for occluders. The output is the same as that of the ray casting
// shader, for the render passes.
const char* CompositeFS = R"(//VTK::System::Dec
//VTK::Output::Dec
in vec2 pixelPositionDC;
uniform sampler2D lowResolutionColorTexture;
uniform sampler2D lowResolutionDepthTexture;
uniform sampler2D lowResolutionNormalTexture;
uniform vec2 viewportOrigin;
uniform vec2 viewportSize;
uniform vec2 lowResolutionSize;
uniform mat4 projectionMatrix;
uniform mat4 inverseProjectionMatrix;
//VTK::DepthPeeling::Dec
void main()
{
  vec2 position = (gl_FragCoord.xy - viewportOrigin) / viewportSize * lowResolutionSize;
  vec4 color = texture(lowResolutionColorTexture, position / lowResolutionSize);
  if (color.a < 0.002)
  {
    discard;
  }
  ivec2 baseTexel = ivec2(floor(position - vec2(0.5)));
  ivec2 maxTexel = ivec2(lowResolutionSize) - ivec2(1);
  float firstDepth = 1.0;
  vec3 firstPositionVC = vec3(0.0);
  vec3 firstNormalVC = vec3(0.0, 0.0, 1.0);
  for (int i = 0; i < 4; i++)
  {
    ivec2 texel = clamp(baseTexel + ivec2(i & 1, i >> 1), ivec2(0), maxTexel);
    if (texelFetch(lowResolutionColorTexture, texel, 0).a <= 0.0)
    {
      continue;
    }
    highp uvec3 depthBytes = uvec3(texelFetch(lowResolutionDepthTexture, texel, 0).rgb * 255.0 + vec3(0.5));
    float depth = float((depthBytes.r << 16u) | (depthBytes.g << 8u) | depthBytes.b) / 16777215.0;
    if (depth <= firstDepth)
    {
      firstDepth = depth;
      // the point of the image pixel (its center) at that depth
      vec2 texelDC = 2.0 * (vec2(texel) + vec2(0.5)) / lowResolutionSize - vec2(1.0);
      vec4 positionVC = inverseProjectionMatrix * vec4(texelDC, 2.0 * depth - 1.0, 1.0);
      firstPositionVC = positionVC.xyz / positionVC.w;
      firstNormalVC = 2.0 * texelFetch(lowResolutionNormalTexture, texel, 0).xyz - vec3(1.0);
    }
  }
  // The ray of this pixel (view coordinates), and where it meets the plane of the surface; not at grazing angles,
  // where the plane says little about where the surface is
  if (firstDepth < 1.0 && length(firstNormalVC) > 0.5)
  {
    firstNormalVC = normalize(firstNormalVC);
    vec2 pixelDC = 2.0 * (gl_FragCoord.xy - viewportOrigin) / viewportSize - vec2(1.0);
    vec4 nearVC = inverseProjectionMatrix * vec4(pixelDC, -1.0, 1.0);
    vec4 farVC = inverseProjectionMatrix * vec4(pixelDC, 1.0, 1.0);
    vec3 rayStart = nearVC.xyz / nearVC.w;
    vec3 rayDirection = farVC.xyz / farVC.w - rayStart;
    float rate = dot(firstNormalVC, rayDirection);
    if (abs(rate) > 0.2 * length(rayDirection))
    {
      float t = dot(firstNormalVC, firstPositionVC - rayStart) / rate;
      if (t > 0.0 && t < 1.0)
      {
        firstPositionVC = rayStart + t * rayDirection;
        vec4 positionDC = projectionMatrix * vec4(firstPositionVC, 1.0);
        firstDepth = clamp(0.5 * positionDC.z / positionDC.w + 0.5, 0.0, 1.0);
      }
    }
  }
  gl_FragDepth = firstDepth;
  //VTK::DepthPeeling::PreColor
  gl_FragData[0] = vec4(color.rgb / color.a, color.a);
  //VTK::SSAO::Impl
  //VTK::DepthPeeling::Impl
}
)";

//----------------------------------------------------------------------------
// Apply the render passes (depth peeling, ambient occlusion) to a shader that writes the output of the mapper
void ApplyRenderPasses(const std::vector<vtkOpenGLRenderPass*>& renderPasses,
                       std::string& vertexShader,
                       std::string& geometryShader,
                       std::string& fragmentShader,
                       vtkAbstractMapper* mapper,
                       vtkActor* actor)
{
  for (vtkOpenGLRenderPass* renderPass : renderPasses)
  {
    renderPass->PreReplaceShaderValues(vertexShader, geometryShader, fragmentShader, mapper, actor);
  }
  for (vtkOpenGLRenderPass* renderPass : renderPasses)
  {
    renderPass->PostReplaceShaderValues(vertexShader, geometryShader, fragmentShader, mapper, actor);
  }
  // Screen-space ambient occlusion (shadows) reads positions and normals as vtkSSAOPass makes polygonal mappers write them
  bool ssao = std::any_of(renderPasses.begin(), renderPasses.end(), [](vtkOpenGLRenderPass* renderPass) { return renderPass->IsA("vtkSSAOPass"); });
  std::string ssaoImplementation = "gl_FragData[1] = vec4(firstPositionVC, 1.0);\n"
                                   "  gl_FragData[2] = vec4(firstNormalVC, 1.0);\n";
  vtkShaderProgram::Substitute(fragmentShader, "//VTK::SSAO::Impl", ssao ? ssaoImplementation : std::string());
}

//----------------------------------------------------------------------------
// Make the color buffers 0..count-1 of the bound framebuffer drawn into. vtkOpenGLState caches the draw buffers, but
// updates only the first one when a framebuffer is bound: the others may be those of the previously bound framebuffer,
// and the call that switches them on would be skipped. So glDrawBuffers is called here in any case.
void ActivateDrawBuffers(vtkOpenGLState* ostate, unsigned int count)
{
  unsigned int buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  ostate->vtkglDrawBuffers(count, buffers);
  glDrawBuffers(static_cast<GLsizei>(count), buffers);
}

//----------------------------------------------------------------------------
// Set the parameters of the render passes for drawing the output of the mapper. vtkSSAOPass makes only the color buffer
// of its framebuffer drawn into for mappers other than polygonal and volume mappers: positions and normals, which the
// mapper writes for ambient occlusion, would be missing where it draws (no shadows on the segments).
void SetRenderPassParameters(const std::vector<vtkOpenGLRenderPass*>& renderPasses,
                             vtkShaderProgram* program,
                             vtkAbstractMapper* mapper,
                             vtkActor* actor,
                             vtkOpenGLVertexArrayObject* vao,
                             vtkOpenGLState* ostate)
{
  bool ssao = false;
  for (vtkOpenGLRenderPass* renderPass : renderPasses)
  {
    renderPass->SetShaderParameters(program, mapper, actor, vao);
    ssao = ssao || renderPass->IsA("vtkSSAOPass");
  }
  if (ssao)
  {
    ActivateDrawBuffers(ostate, 3);
  }
}

//----------------------------------------------------------------------------
// Upload a matrix so that "matrix * vector" in GLSL is the same as in VTK
void SetUniformMatrix4x4(vtkShaderProgram* program, const char* name, vtkMatrix4x4* matrix)
{
  float values[16];
  for (int column = 0; column < 4; ++column)
  {
    for (int row = 0; row < 4; ++row)
    {
      values[column * 4 + row] = static_cast<float>(matrix->GetElement(row, column));
    }
  }
  program->SetUniformMatrix4x4(name, values);
}

//----------------------------------------------------------------------------
void SetUniformMatrix3x3(vtkShaderProgram* program, const char* name, const double matrix[3][3])
{
  float values[9];
  for (int column = 0; column < 3; ++column)
  {
    for (int row = 0; row < 3; ++row)
    {
      values[column * 3 + row] = static_cast<float>(matrix[row][column]);
    }
  }
  program->SetUniformMatrix3x3(name, values);
}

//----------------------------------------------------------------------------
// Integer vectors are passed to shaders as float vectors
void SetUniformVector3(vtkShaderProgram* program, const char* name, const int vector[3])
{
  float values[3] = { static_cast<float>(vector[0]), static_cast<float>(vector[1]), static_cast<float>(vector[2]) };
  program->SetUniform3f(name, values);
}

//----------------------------------------------------------------------------
// Extent of each label in a labelmap
struct LabelExtents
{
  vtkWeakPointer<vtkImageData> Image;
  vtkMTimeType Time{ 0 };
  /// Label values in increasing order, and the extent of each (\sa vtkOrientedImageDataResample::CalculateEffectiveExtentPerLabel)
  vtkSmartPointer<vtkIntArray> LabelValues = vtkSmartPointer<vtkIntArray>::New();
  vtkSmartPointer<vtkIntArray> Extents = vtkSmartPointer<vtkIntArray>::New();
  /// Largest label value in the labelmap (0 if there are no labels)
  int MaximumLabel() const
  {
    vtkIdType numberOfLabels = this->LabelValues->GetNumberOfValues();
    return numberOfLabels > 0 ? this->LabelValues->GetValue(numberOfLabels - 1) : 0;
  }
  /// Get the extent of a label, false if the label is not in the labelmap
  bool GetExtent(int label, int extent[6]) const
  {
    vtkIdType index = this->LabelValues->LookupValue(label);
    if (index < 0)
    {
      return false;
    }
    this->Extents->GetTypedTuple(index, extent);
    return true;
  }
};

//----------------------------------------------------------------------------
// Labelmaps are often shown by several mappers (opaque segments in one, each translucent segment in its own),
// they share the extents of the labels, computed once after each modification of the labelmap.
const LabelExtents* GetLabelExtents(vtkImageData* image)
{
  static std::vector<LabelExtents> cache;
  cache.erase(std::remove_if(cache.begin(), cache.end(), [](const LabelExtents& entry) { return entry.Image == nullptr; }), cache.end());
  auto entry = std::find_if(cache.begin(), cache.end(), [image](const LabelExtents& entry) { return entry.Image == image; });
  if (entry == cache.end())
  {
    cache.emplace_back();
    entry = cache.end() - 1;
    entry->Image = image;
  }
  if (entry->Time != image->GetMTime())
  {
    if (!vtkOrientedImageDataResample::CalculateEffectiveExtentPerLabel(image, entry->LabelValues, entry->Extents))
    {
      return nullptr;
    }
    entry->Time = image->GetMTime();
  }
  return &(*entry);
}

//----------------------------------------------------------------------------
template <class T, class OutputType>
void CopyLabels(vtkImageData* image, const int effectiveExtent[6], std::vector<OutputType>& labels)
{
  size_t numberOfVoxels = 1;
  for (int axis = 0; axis < 3; ++axis)
  {
    numberOfVoxels *= static_cast<size_t>(effectiveExtent[axis * 2 + 1] - effectiveExtent[axis * 2] + 1);
  }
  labels.resize(numberOfVoxels);
  OutputType* output = labels.data();
  for (int k = effectiveExtent[4]; k <= effectiveExtent[5]; ++k)
  {
    for (int j = effectiveExtent[2]; j <= effectiveExtent[3]; ++j)
    {
      T* voxel = static_cast<T*>(image->GetScalarPointer(effectiveExtent[0], j, k));
      for (int i = effectiveExtent[0]; i <= effectiveExtent[1]; ++i, ++voxel, ++output)
      {
        *output = *voxel > 0 ? static_cast<OutputType>(*voxel) : 0;
      }
    }
  }
}

//----------------------------------------------------------------------------
template <class T>
void CopyLabelsAs(vtkImageData* image, const int extent[6], bool asFloat, std::vector<unsigned char>& charLabels, std::vector<float>& floatLabels)
{
  if (asFloat)
  {
    CopyLabels<T, float>(image, extent, floatLabels);
  }
  else
  {
    CopyLabels<T, unsigned char>(image, extent, charLabels);
  }
}

} // namespace

//----------------------------------------------------------------------------
class vtkSegmentationLabelmapSurfaceMapper::vtkInternal
{
public:
  vtkInternal(vtkSegmentationLabelmapSurfaceMapper* external)
    : External(external)
  {
  }

  bool PrepareLabelmap();
  bool UploadTextures(vtkOpenGLRenderWindow* renWin);
  /// Compute the distance field, in the changed region (in grid coordinates) or everywhere (if changedRegion is nullptr).
  bool ComputeDistanceField(vtkOpenGLRenderWindow* renWin, const int* changedRegion);
  void RenderSurface(vtkOpenGLRenderer* ren, vtkActor* actor);
  void ReleaseGraphicsResources(vtkWindow* window);

  bool ReadyQuad(vtkOpenGLRenderWindow* renWin, vtkOpenGLQuadHelper*& quad, const std::string& fragmentShader);
  void SetGridUniforms(vtkShaderProgram* program);
  void SetInsideUniforms(vtkShaderProgram* program);
  bool DrawSlices(vtkOpenGLQuadHelper* quad, vtkTextureObject* target, const int region[6]);
  /// Compute the colors of the segments at the voxels of the grid, in a region (in grid coordinates) or everywhere
  /// (if region is nullptr).
  bool ComputeColors(vtkOpenGLRenderWindow* renWin, const int* region);
  /// Ready the program of the ray casting shader with the specified output code (and render passes applied to it, if
  /// it is rendered into the viewport).
  vtkShaderProgram* ReadySurfaceProgram(vtkOpenGLRenderer* ren,
                                        vtkActor* actor,
                                        const std::vector<vtkOpenGLRenderPass*>& renderPasses,
                                        bool lowResolution,
                                        vtkShaderProgram*& cachedProgram,
                                        std::string& cachedKey);
  /// Make the image of the lower resolution (and the framebuffer to render into it) the specified size.
  bool ReadyLowResolutionFramebuffer(vtkOpenGLRenderWindow* renWin, const int size[2]);
  vtkSmartPointer<vtkTextureObject> CreateTexture(vtkOpenGLRenderWindow* renWin, unsigned int internalFormat, int components, int vtkType, bool linear);

  vtkSegmentationLabelmapSurfaceMapper* External;

  bool CaptureOpaqueDepth(vtkOpenGLRenderer* ren);
  std::set<int> GetShownLabels() const;

  vtkSmartPointer<vtkOrientedImageData> Labelmap;
  std::map<int, std::array<double, 4>> LabelColors;

  // Labels within the effective extent of the shown labels, prepared on the CPU for upload
  vtkTimeStamp PrepareTime;
  vtkMTimeType PreparedLabelmapTime{ 0 };
  vtkOrientedImageData* PreparedLabelmap{ nullptr };
  std::set<int> PreparedLabels;
  vtkIdType PreparedMaximumNumberOfVoxels{ 0 };
  bool Empty{ true };
  int EffectiveExtent[6] = { 0, -1, 0, -1, 0, -1 };
  int LabelDimensions[3] = { 0, 0, 0 };
  int Downsampling[3] = { 1, 1, 1 };
  int GridDimensions[3] = { 0, 0, 0 };
  const int Padding{ 2 };
  double MCToGridScale[3] = { 1.0, 1.0, 1.0 };
  double MCToGridOffset[3] = { 0.0, 0.0, 0.0 };
  double GridToPhysical[3][3];
  double Bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  bool FloatLabels{ false };
  std::vector<unsigned char> LabelsUnsignedChar;
  std::vector<float> LabelsFloat;
  /// Labels before the last change, to find the region that changed
  std::vector<unsigned char> PreviousLabelsUnsignedChar;
  std::vector<float> PreviousLabelsFloat;

  // Graphics resources
  vtkWeakPointer<vtkOpenGLRenderWindow> Context;
  vtkSmartPointer<vtkTextureObject> LabelTexture;
  vtkSmartPointer<vtkTextureObject> PaletteTexture;
  vtkSmartPointer<vtkTextureObject> DistanceTexture;
  int PaletteSize{ 0 };
  vtkTimeStamp LabelTextureTime;
  std::map<int, std::array<double, 4>> UploadedLabelColors;
  vtkTimeStamp DistanceFieldTime;
  bool DistanceFieldNeedsUpdate{ true };
  /// Region of the grid where labels changed since the distance field was computed
  /// (only used if DistanceFieldNeedsUpdate is false: then the distance field is updated in this region only)
  bool ChangedRegionValid{ false };
  int ChangedRegion[6] = { 0, -1, 0, -1, 0, -1 };
  double ComputedSmoothingFactor{ -1.0 };

  // Depth of opaque geometry, which stops rays of translucent surfaces
  vtkSmartPointer<vtkTextureObject> OpaqueDepthTexture;
  vtkSmartPointer<vtkTextureObject> OpaqueDepthCopyColorTexture;
  vtkSmartPointer<vtkOpenGLFramebufferObject> OpaqueDepthFramebuffer;
  bool Failed{ false };

  vtkNew<vtkOpenGLFramebufferObject> Framebuffer;
  vtkOpenGLQuadHelper* SeedQuad{ nullptr };
  vtkOpenGLQuadHelper* JumpFloodQuad{ nullptr };
  vtkOpenGLQuadHelper* FinalizeQuad{ nullptr };
  vtkOpenGLQuadHelper* SmoothQuad{ nullptr };
  vtkOpenGLQuadHelper* CombineQuad{ nullptr };
  vtkOpenGLQuadHelper* MaskQuad{ nullptr };

  vtkOpenGLQuadHelper* ColorQuad{ nullptr };
  vtkSmartPointer<vtkTextureObject> ColorTexture;
  bool ColorsNeedUpdate{ true };

  vtkShaderProgram* SurfaceProgram{ nullptr };
  std::string SurfaceProgramKey;
  vtkNew<vtkOpenGLVertexArrayObject> SurfaceVAO;

  // Rendering at a lower resolution (ImageSampleDistance > 1): color, depth, normal
  vtkShaderProgram* LowResolutionProgram{ nullptr };
  std::string LowResolutionProgramKey;
  vtkShaderProgram* CompositeProgram{ nullptr };
  std::string CompositeProgramKey;
  vtkSmartPointer<vtkOpenGLFramebufferObject> LowResolutionFramebuffer;
  vtkSmartPointer<vtkTextureObject> LowResolutionTextures[3];
};

//----------------------------------------------------------------------------
std::set<int> vtkSegmentationLabelmapSurfaceMapper::vtkInternal::GetShownLabels() const
{
  std::set<int> labels;
  for (const auto& labelColor : this->LabelColors)
  {
    if (labelColor.first > 0 && labelColor.second[3] > 0.0)
    {
      labels.insert(labelColor.first);
    }
  }
  return labels;
}

//----------------------------------------------------------------------------
bool vtkSegmentationLabelmapSurfaceMapper::vtkInternal::PrepareLabelmap()
{
  std::set<int> shownLabels = this->GetShownLabels();
  bool sameLabelmap = this->Labelmap && this->PreparedLabelmap == this->Labelmap && this->PreparedLabelmapTime == this->Labelmap->GetMTime();
  if (sameLabelmap && this->PreparedLabels == shownLabels && this->PreparedMaximumNumberOfVoxels == this->External->MaximumNumberOfVoxels)
  {
    return !this->Empty;
  }
  // Only the labels changed (same labelmap, shown labels, and grid): the distance field may be updated where they changed
  int previousEffectiveExtent[6];
  std::copy_n(this->EffectiveExtent, 6, previousEffectiveExtent);
  bool previousFloatLabels = this->FloatLabels;
  bool labelsMayChangeInRegion = !this->Empty && this->Labelmap && this->PreparedLabelmap == this->Labelmap && this->PreparedLabels == shownLabels
                                 && this->PreparedMaximumNumberOfVoxels == this->External->MaximumNumberOfVoxels;
  this->PreviousLabelsUnsignedChar.swap(this->LabelsUnsignedChar);
  this->PreviousLabelsFloat.swap(this->LabelsFloat);
  bool fullUpdatePending = this->DistanceFieldNeedsUpdate;

  this->PreparedLabelmap = this->Labelmap;
  this->PreparedLabelmapTime = this->Labelmap ? this->Labelmap->GetMTime() : 0;
  this->PreparedLabels = shownLabels;
  this->PreparedMaximumNumberOfVoxels = this->External->MaximumNumberOfVoxels;
  this->PrepareTime.Modified();
  this->Empty = true;
  vtkMath::UninitializeBounds(this->Bounds);
  this->LabelsUnsignedChar.clear();
  this->LabelsFloat.clear();
  this->DistanceFieldNeedsUpdate = true;
  if (!this->Labelmap || !this->Labelmap->GetPointData() || !this->Labelmap->GetPointData()->GetScalars())
  {
    return false;
  }
  if (this->Labelmap->GetNumberOfScalarComponents() != 1)
  {
    vtkErrorWithObjectMacro(this->External, "PrepareLabelmap: labelmap must have a single scalar component");
    return false;
  }
  const LabelExtents* labelExtents = GetLabelExtents(this->Labelmap);
  if (!labelExtents)
  {
    vtkErrorWithObjectMacro(this->External, "PrepareLabelmap: unsupported scalar type");
    return false;
  }

  // Only the region of the shown labels is used
  this->EffectiveExtent[0] = this->EffectiveExtent[2] = this->EffectiveExtent[4] = VTK_INT_MAX;
  this->EffectiveExtent[1] = this->EffectiveExtent[3] = this->EffectiveExtent[5] = VTK_INT_MIN;
  for (int label : shownLabels)
  {
    int labelExtent[6] = { 0, -1, 0, -1, 0, -1 };
    if (!labelExtents->GetExtent(label, labelExtent))
    {
      continue;
    }
    for (int axis = 0; axis < 3; ++axis)
    {
      this->EffectiveExtent[axis * 2] = std::min(this->EffectiveExtent[axis * 2], labelExtent[axis * 2]);
      this->EffectiveExtent[axis * 2 + 1] = std::max(this->EffectiveExtent[axis * 2 + 1], labelExtent[axis * 2 + 1]);
    }
  }
  if (this->EffectiveExtent[0] > this->EffectiveExtent[1])
  {
    // no shown labels
    return false;
  }

  // Grid of the distance field: the labelmap (downsampled if needed) with padding on each side,
  // so that segments touching the labelmap boundary are capped.
  vtkNew<vtkMatrix4x4> imageToWorld;
  this->Labelmap->GetImageToWorldMatrix(imageToWorld);
  // Downsampling: increase the factor along the axis that has the smallest voxels (after downsampling)
  // until the grid is small enough. Dimensions are limited by the maximum 3D texture size of WebGL2 implementations.
  const int maximumDimension = 2048;
  double voxelSizes[3];
  for (int axis = 0; axis < 3; ++axis)
  {
    this->LabelDimensions[axis] = this->EffectiveExtent[axis * 2 + 1] - this->EffectiveExtent[axis * 2] + 1;
    this->Downsampling[axis] = 1;
    double column[3] = { imageToWorld->GetElement(0, axis), imageToWorld->GetElement(1, axis), imageToWorld->GetElement(2, axis) };
    voxelSizes[axis] = vtkMath::Norm(column);
  }
  // The downsampling factor is chosen for the extent of all labels (not only the shown ones), rounded up, so that
  // all segments of a labelmap are shown at the same resolution (whether they are opaque, translucent, or others are
  // hidden) and editing near their boundary does not change it (which would change the appearance of the whole surface).
  int allLabelsDimensions[3] = { 0, 0, 0 };
  for (int axis = 0; axis < 3; ++axis)
  {
    int minimum = VTK_INT_MAX;
    int maximum = VTK_INT_MIN;
    for (vtkIdType index = 0; index < labelExtents->Extents->GetNumberOfTuples(); ++index)
    {
      minimum = std::min(minimum, labelExtents->Extents->GetTypedComponent(index, axis * 2));
      maximum = std::max(maximum, labelExtents->Extents->GetTypedComponent(index, axis * 2 + 1));
    }
    const int sizeGranularity = 64;
    allLabelsDimensions[axis] = (maximum - minimum + 1 + sizeGranularity - 1) / sizeGranularity * sizeGranularity;
  }
  auto plannedGridDimension = [this, &allLabelsDimensions](int axis)
  { return (allLabelsDimensions[axis] + this->Downsampling[axis] - 1) / this->Downsampling[axis] + 2 * this->Padding; };
  while (true)
  {
    vtkIdType numberOfVoxels = static_cast<vtkIdType>(plannedGridDimension(0)) * plannedGridDimension(1) * plannedGridDimension(2);
    int axisToDownsample = -1;
    for (int axis = 0; axis < 3; ++axis)
    {
      if (plannedGridDimension(axis) > maximumDimension)
      {
        axisToDownsample = axis;
      }
    }
    if (axisToDownsample < 0 && numberOfVoxels <= this->External->MaximumNumberOfVoxels)
    {
      break;
    }
    if (axisToDownsample < 0)
    {
      for (int axis = 0; axis < 3; ++axis)
      {
        if (this->Downsampling[axis] < allLabelsDimensions[axis]
            && (axisToDownsample < 0 || voxelSizes[axis] * this->Downsampling[axis] < voxelSizes[axisToDownsample] * this->Downsampling[axisToDownsample]))
        {
          axisToDownsample = axis;
        }
      }
    }
    if (axisToDownsample < 0)
    {
      break;
    }
    this->Downsampling[axisToDownsample]++;
  }
  // Downsampled voxels are blocks of labelmap voxels at fixed positions (multiples of the downsampling factor),
  // so that a change of the shown labels' extent does not change which labelmap voxels are combined.
  const int* labelmapExtent = this->Labelmap->GetExtent();
  for (int axis = 0; axis < 3; ++axis)
  {
    int factor = this->Downsampling[axis];
    int alignedStart = static_cast<int>(std::floor(static_cast<double>(this->EffectiveExtent[axis * 2]) / factor)) * factor;
    this->EffectiveExtent[axis * 2] = std::max(alignedStart, labelmapExtent[axis * 2]);
    this->LabelDimensions[axis] = this->EffectiveExtent[axis * 2 + 1] - this->EffectiveExtent[axis * 2] + 1;
  }
  auto gridDimension = [this](int axis) { return (this->LabelDimensions[axis] + this->Downsampling[axis] - 1) / this->Downsampling[axis] + 2 * this->Padding; };

  this->FloatLabels = (labelExtents->MaximumLabel() > 255);
  switch (this->Labelmap->GetScalarType())
  {
    vtkTemplateMacro(CopyLabelsAs<VTK_TT>(this->Labelmap, this->EffectiveExtent, this->FloatLabels, this->LabelsUnsignedChar, this->LabelsFloat));
  }

  // Region of the grid where labels changed
  bool sameExtent = std::equal(previousEffectiveExtent, previousEffectiveExtent + 6, this->EffectiveExtent);
  labelsMayChangeInRegion = labelsMayChangeInRegion && previousFloatLabels == this->FloatLabels && sameExtent;
  if (labelsMayChangeInRegion)
  {
    int changedLabels[6] = { VTK_INT_MAX, VTK_INT_MIN, VTK_INT_MAX, VTK_INT_MIN, VTK_INT_MAX, VTK_INT_MIN };
    int dimensions[3] = { this->EffectiveExtent[1] - this->EffectiveExtent[0] + 1,
                          this->EffectiveExtent[3] - this->EffectiveExtent[2] + 1,
                          this->EffectiveExtent[5] - this->EffectiveExtent[4] + 1 };
    size_t index = 0;
    for (int k = 0; k < dimensions[2]; ++k)
    {
      for (int j = 0; j < dimensions[1]; ++j)
      {
        for (int i = 0; i < dimensions[0]; ++i, ++index)
        {
          bool changed = false;
          if (this->FloatLabels)
          {
            changed = (this->LabelsFloat[index] != this->PreviousLabelsFloat[index]);
          }
          else
          {
            changed = (this->LabelsUnsignedChar[index] != this->PreviousLabelsUnsignedChar[index]);
          }
          if (changed)
          {
            int voxel[3] = { i, j, k };
            for (int axis = 0; axis < 3; ++axis)
            {
              changedLabels[axis * 2] = std::min(changedLabels[axis * 2], voxel[axis]);
              changedLabels[axis * 2 + 1] = std::max(changedLabels[axis * 2 + 1], voxel[axis]);
            }
          }
        }
      }
    }
    if (changedLabels[0] > changedLabels[1])
    {
      // nothing changed
      if (!fullUpdatePending)
      {
        this->DistanceFieldNeedsUpdate = false;
      }
    }
    else
    {
      // label voxel l is in grid voxel l / downsampling + padding
      int changedGrid[6];
      for (int axis = 0; axis < 3; ++axis)
      {
        changedGrid[axis * 2] = changedLabels[axis * 2] / this->Downsampling[axis] + this->Padding;
        changedGrid[axis * 2 + 1] = changedLabels[axis * 2 + 1] / this->Downsampling[axis] + this->Padding;
      }
      if (!fullUpdatePending)
      {
        this->DistanceFieldNeedsUpdate = false;
        if (this->ChangedRegionValid)
        {
          for (int axis = 0; axis < 3; ++axis)
          {
            this->ChangedRegion[axis * 2] = std::min(this->ChangedRegion[axis * 2], changedGrid[axis * 2]);
            this->ChangedRegion[axis * 2 + 1] = std::max(this->ChangedRegion[axis * 2 + 1], changedGrid[axis * 2 + 1]);
          }
        }
        else
        {
          std::copy_n(changedGrid, 6, this->ChangedRegion);
          this->ChangedRegionValid = true;
        }
      }
    }
  }
  this->PreviousLabelsUnsignedChar.clear();
  this->PreviousLabelsFloat.clear();

  for (int axis = 0; axis < 3; ++axis)
  {
    this->GridDimensions[axis] = gridDimension(axis);
    // Model coordinates are labelmap IJK coordinates: mc = grid * downsampling + offset
    double offset = this->EffectiveExtent[axis * 2] - this->Padding * this->Downsampling[axis] + (this->Downsampling[axis] - 1) * 0.5;
    this->MCToGridScale[axis] = 1.0 / this->Downsampling[axis];
    this->MCToGridOffset[axis] = -offset / this->Downsampling[axis];
    this->Bounds[axis * 2] = -0.5 * this->Downsampling[axis] + offset;
    this->Bounds[axis * 2 + 1] = (this->GridDimensions[axis] - 0.5) * this->Downsampling[axis] + offset;
    for (int row = 0; row < 3; ++row)
    {
      this->GridToPhysical[row][axis] = imageToWorld->GetElement(row, axis) * this->Downsampling[axis];
    }
  }
  this->Empty = false;
  return true;
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkTextureObject> vtkSegmentationLabelmapSurfaceMapper::vtkInternal::CreateTexture(vtkOpenGLRenderWindow* renWin,
                                                                                                   unsigned int internalFormat,
                                                                                                   int components,
                                                                                                   int vtkType,
                                                                                                   bool linear)
{
  vtkSmartPointer<vtkTextureObject> texture = vtkSmartPointer<vtkTextureObject>::New();
  texture->SetContext(renWin);
  texture->SetInternalFormat(internalFormat);
  texture->SetWrapS(vtkTextureObject::ClampToEdge);
  texture->SetWrapT(vtkTextureObject::ClampToEdge);
  texture->SetWrapR(vtkTextureObject::ClampToEdge);
  texture->SetMinificationFilter(linear ? vtkTextureObject::Linear : vtkTextureObject::Nearest);
  texture->SetMagnificationFilter(linear ? vtkTextureObject::Linear : vtkTextureObject::Nearest);
  if (!texture->Allocate3D(this->GridDimensions[0], this->GridDimensions[1], this->GridDimensions[2], components, vtkType))
  {
    return nullptr;
  }
  return texture;
}

//----------------------------------------------------------------------------
bool vtkSegmentationLabelmapSurfaceMapper::vtkInternal::UploadTextures(vtkOpenGLRenderWindow* renWin)
{
  if (!this->LabelTexture || this->LabelTextureTime < this->PrepareTime)
  {
    this->LabelTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->LabelTexture->SetContext(renWin);
    this->LabelTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->LabelTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->LabelTexture->SetWrapR(vtkTextureObject::ClampToEdge);
    this->LabelTexture->SetMinificationFilter(vtkTextureObject::Nearest);
    this->LabelTexture->SetMagnificationFilter(vtkTextureObject::Nearest);
    int scalarType = this->FloatLabels ? VTK_FLOAT : VTK_UNSIGNED_CHAR;
    void* labels = this->FloatLabels ? static_cast<void*>(this->LabelsFloat.data()) : static_cast<void*>(this->LabelsUnsignedChar.data());
    bool success = this->LabelTexture->Create3DFromRaw(this->LabelDimensions[0], this->LabelDimensions[1], this->LabelDimensions[2], 1, scalarType, labels);
    if (!success)
    {
      vtkErrorWithObjectMacro(this->External, "UploadTextures: failed to create labelmap texture");
      this->LabelTexture = nullptr;
      return false;
    }
    this->LabelTextureTime.Modified();
  }

  if (!this->PaletteTexture || this->UploadedLabelColors != this->LabelColors)
  {
    // Shown labels change the distance field, colors and opacities only change the palette
    std::set<int> uploadedShownLabels;
    for (const auto& labelColor : this->UploadedLabelColors)
    {
      if (labelColor.first > 0 && labelColor.second[3] > 0.0)
      {
        uploadedShownLabels.insert(labelColor.first);
      }
    }
    if (uploadedShownLabels != this->GetShownLabels())
    {
      this->DistanceFieldNeedsUpdate = true;
    }
    int maximumLabel = this->LabelColors.empty() ? 0 : this->LabelColors.rbegin()->first;
    const int paletteWidth = 256;
    int paletteHeight = maximumLabel / paletteWidth + 1;
    std::vector<unsigned char> palette(static_cast<size_t>(paletteWidth) * paletteHeight * 4, 0);
    for (const auto& labelColor : this->LabelColors)
    {
      if (labelColor.first <= 0)
      {
        continue;
      }
      unsigned char* entry = palette.data() + static_cast<size_t>(labelColor.first) * 4;
      for (int component = 0; component < 4; ++component)
      {
        entry[component] = static_cast<unsigned char>(std::clamp(labelColor.second[component], 0.0, 1.0) * 255.0 + 0.5);
      }
      if (labelColor.second[3] > 0.0)
      {
        // shown labels are never fully transparent in the palette
        entry[3] = std::max(entry[3], static_cast<unsigned char>(1));
      }
    }
    this->PaletteTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->PaletteTexture->SetContext(renWin);
    this->PaletteTexture->SetMinificationFilter(vtkTextureObject::Nearest);
    this->PaletteTexture->SetMagnificationFilter(vtkTextureObject::Nearest);
    if (!this->PaletteTexture->Create2DFromRaw(paletteWidth, paletteHeight, 4, VTK_UNSIGNED_CHAR, palette.data()))
    {
      vtkErrorWithObjectMacro(this->External, "UploadTextures: failed to create palette texture");
      this->PaletteTexture = nullptr;
      return false;
    }
    this->PaletteSize = paletteWidth * paletteHeight;
    this->UploadedLabelColors = this->LabelColors;
    this->ColorsNeedUpdate = true;
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSegmentationLabelmapSurfaceMapper::vtkInternal::ReadyQuad(vtkOpenGLRenderWindow* renWin, vtkOpenGLQuadHelper*& quad, const std::string& fragmentShader)
{
  if (!quad)
  {
    quad = new vtkOpenGLQuadHelper(renWin, nullptr, fragmentShader.c_str(), "");
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(quad->Program);
  }
  if (!quad->Program || !quad->Program->GetCompiled())
  {
    vtkErrorWithObjectMacro(this->External, "Failed to compile distance field shader");
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
void vtkSegmentationLabelmapSurfaceMapper::vtkInternal::SetGridUniforms(vtkShaderProgram* program)
{
  SetUniformVector3(program, "gridDimensions", this->GridDimensions);
}

//----------------------------------------------------------------------------
void vtkSegmentationLabelmapSurfaceMapper::vtkInternal::SetInsideUniforms(vtkShaderProgram* program)
{
  program->SetUniformi("labelTexture", this->LabelTexture->GetTextureUnit());
  program->SetUniformi("paletteTexture", this->PaletteTexture->GetTextureUnit());
  SetUniformVector3(program, "labelDimensions", this->LabelDimensions);
  SetUniformVector3(program, "downsampling", this->Downsampling);
  program->SetUniformi("padding", this->Padding);
  program->SetUniformi("paletteSize", this->PaletteSize);
  // 8-bit labels are stored in a normalized texture
  program->SetUniformf("labelScale", this->FloatLabels ? 1.0f : 255.0f);
}

//----------------------------------------------------------------------------
bool vtkSegmentationLabelmapSurfaceMapper::vtkInternal::DrawSlices(vtkOpenGLQuadHelper* quad, vtkTextureObject* target, const int region[6])
{
  // Only the region is drawn (z: slices, x and y: scissor)
  vtkOpenGLState* ostate = this->Context->GetState();
  ostate->vtkglEnable(GL_SCISSOR_TEST);
  ostate->vtkglScissor(region[0], region[2], region[1] - region[0] + 1, region[3] - region[2] + 1);
  for (int z = region[4]; z <= region[5]; ++z)
  {
    // vtkOpenGLFramebufferObject cannot attach a slice of a 3D texture in OpenGL ES
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target->GetHandle(), 0, z);
    if (z == region[4])
    {
      GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if (status != GL_FRAMEBUFFER_COMPLETE)
      {
        vtkErrorWithObjectMacro(this->External,
                                "Rendering into floating-point 3D textures is not supported by the graphics driver"
                                " (framebuffer status: "
                                  << status << "). Segmentations cannot be displayed as binary labelmap in 3D views.");
        return false;
      }
    }
    quad->Program->SetUniformi("sliceZ", z);
    quad->Render();
  }
  vtkOpenGLStaticCheckErrorMacro("failed after rendering slices");
  return true;
}

//----------------------------------------------------------------------------
bool vtkSegmentationLabelmapSurfaceMapper::vtkInternal::ComputeDistanceField(vtkOpenGLRenderWindow* renWin, const int* changedRegion)
{
  vtkOpenGLState* ostate = renWin->GetState();
  vtkOpenGLState::ScopedglViewport savedViewport(ostate);
  vtkOpenGLState::ScopedglScissor savedScissor(ostate);
  vtkOpenGLState::ScopedglColorMask savedColorMask(ostate);
  vtkOpenGLState::ScopedglEnableDisable savedDepthTest(ostate, GL_DEPTH_TEST);
  vtkOpenGLState::ScopedglEnableDisable savedBlend(ostate, GL_BLEND);
  vtkOpenGLState::ScopedglEnableDisable savedScissorTest(ostate, GL_SCISSOR_TEST);
  vtkOpenGLState::ScopedglEnableDisable savedCullFace(ostate, GL_CULL_FACE);
  ostate->vtkglDisable(GL_DEPTH_TEST);
  ostate->vtkglDisable(GL_BLEND);
  ostate->vtkglDisable(GL_CULL_FACE);
  ostate->vtkglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  ostate->vtkglViewport(0, 0, this->GridDimensions[0], this->GridDimensions[1]);

  // Voxel sizes and smoothing (see CombineFS). The standard deviation of the Gaussian is the same in all directions in
  // physical space (SmoothingSigmaPerFactor * SmoothingFactor times the largest voxel size), so that staircases of thick slices are smoothed
  // as much as in-plane ones.
  double voxelSizes[3] = { 0.0, 0.0, 0.0 };
  for (int axis = 0; axis < 3; ++axis)
  {
    double column[3] = { this->GridToPhysical[0][axis], this->GridToPhysical[1][axis], this->GridToPhysical[2][axis] };
    voxelSizes[axis] = std::max(vtkMath::Norm(column), 1e-6);
  }
  double sigmaMm = SmoothingSigmaPerFactor * this->External->SmoothingFactor * std::max({ voxelSizes[0], voxelSizes[1], voxelSizes[2] });
  int smoothingRadius[3] = { 0, 0, 0 };
  for (int axis = 0; axis < 3 && sigmaMm > 0.0; ++axis)
  {
    smoothingRadius[axis] = std::clamp(static_cast<int>(std::ceil(3.0 * sigmaMm / voxelSizes[axis])), 1, 15);
  }

  // Regions. When only a part of the labelmap changed, only the distances near it are computed: in the working region,
  // where the passes are drawn, and the distances are updated in the inner region, which is far enough from the border
  // of the working region (where seeds and smoothing are incomplete) to be correct.
  vtkTextureObject* previous = this->DistanceTexture;
  unsigned int gridDimensions[3] = { static_cast<unsigned int>(this->GridDimensions[0]),
                                     static_cast<unsigned int>(this->GridDimensions[1]),
                                     static_cast<unsigned int>(this->GridDimensions[2]) };
  bool sameGrid = (previous != nullptr);
  for (int axis = 0; axis < 3 && sameGrid; ++axis)
  {
    unsigned int previousDimension = (axis == 0 ? previous->GetWidth() : (axis == 1 ? previous->GetHeight() : previous->GetDepth()));
    sameGrid = (previousDimension == gridDimensions[axis]);
  }
  bool fullUpdate = (changedRegion == nullptr) || !sameGrid;
  int innerRegion[6];
  int workingRegion[6];
  int maskRegion[6];
  for (int axis = 0; axis < 3; ++axis)
  {
    int margin = 2 * smoothingRadius[axis] + 4;
    int lastVoxel = this->GridDimensions[axis] - 1;
    if (fullUpdate)
    {
      innerRegion[axis * 2] = workingRegion[axis * 2] = maskRegion[axis * 2] = 0;
      innerRegion[axis * 2 + 1] = workingRegion[axis * 2 + 1] = maskRegion[axis * 2 + 1] = lastVoxel;
      continue;
    }
    innerRegion[axis * 2] = std::max(0, changedRegion[axis * 2] - margin);
    innerRegion[axis * 2 + 1] = std::min(lastVoxel, changedRegion[axis * 2 + 1] + margin);
    workingRegion[axis * 2] = std::max(0, changedRegion[axis * 2] - 3 * margin);
    workingRegion[axis * 2 + 1] = std::min(lastVoxel, changedRegion[axis * 2 + 1] + 3 * margin);
    maskRegion[axis * 2] = std::max(0, workingRegion[axis * 2] - 1);
    maskRegion[axis * 2 + 1] = std::min(lastVoxel, workingRegion[axis * 2 + 1] + 1);
  }

  // Textures: seeds (offsets to the nearest boundary point, xyz, and valid flag, w), mask, distances
  vtkSmartPointer<vtkTextureObject> seedTextures[2];
  for (int i = 0; i < 2; ++i)
  {
    seedTextures[i] = this->CreateTexture(renWin, GL_RGBA8, 4, VTK_UNSIGNED_CHAR, false);
  }
  vtkSmartPointer<vtkTextureObject> maskTexture = this->CreateTexture(renWin, GL_R8, 1, VTK_UNSIGNED_CHAR, false);
  vtkSmartPointer<vtkTextureObject> distanceTextures[4]; // original, smoothed, smoothed twice, temporary
  for (int i = 0; i < 4; ++i)
  {
    distanceTextures[i] = this->CreateTexture(renWin, GL_R16F, 1, VTK_FLOAT, true);
  }
  vtkSmartPointer<vtkTextureObject> resultTexture = fullUpdate ? this->CreateTexture(renWin, GL_R16F, 1, VTK_FLOAT, true) : this->DistanceTexture;
  bool success = seedTextures[0] && seedTextures[1] && maskTexture && resultTexture;
  for (int i = 0; i < 4; ++i)
  {
    success = success && distanceTextures[i];
  }
  if (!success)
  {
    vtkErrorWithObjectMacro(this->External, "ComputeDistanceField: failed to allocate textures");
    return false;
  }
  vtkTextureObject* original = distanceTextures[0];
  vtkTextureObject* smoothed = distanceTextures[1];
  vtkTextureObject* smoothedTwice = distanceTextures[2];
  vtkTextureObject* temporary = distanceTextures[3];

  this->Framebuffer->SetContext(renWin);
  this->Framebuffer->SaveCurrentBindingsAndBuffers();
  this->Framebuffer->Bind();
  this->Framebuffer->ActivateDrawBuffers(1);

  std::string sliceShader = SliceCommonFS;

  // Mask: shown segments and narrow gaps between them
  if (success && (success = this->ReadyQuad(renWin, this->MaskQuad, sliceShader + LabelFS + MaskFS)))
  {
    vtkShaderProgram* program = this->MaskQuad->Program;
    this->LabelTexture->Activate();
    this->PaletteTexture->Activate();
    this->SetGridUniforms(program);
    this->SetInsideUniforms(program);
    float gapVoxels[3];
    for (int axis = 0; axis < 3; ++axis)
    {
      gapVoxels[axis] = static_cast<float>(std::min(4.0, std::floor(sigmaMm / voxelSizes[axis])));
    }
    program->SetUniform3f("gapVoxels", gapVoxels);
    success = this->DrawSlices(this->MaskQuad, maskTexture, maskRegion);
    this->LabelTexture->Deactivate();
    this->PaletteTexture->Deactivate();
  }
  if (success)
  {
    maskTexture->Activate();
  }

  // Seeds
  if (success && (success = this->ReadyQuad(renWin, this->SeedQuad, sliceShader + InsideFS + SeedCodingFS + SeedFS)))
  {
    vtkShaderProgram* program = this->SeedQuad->Program;
    this->SetGridUniforms(program);
    program->SetUniformi("maskTexture", maskTexture->GetTextureUnit());
    SetUniformMatrix3x3(program, "gridToPhysical", this->GridToPhysical);
    success = this->DrawSlices(this->SeedQuad, seedTextures[0], workingRegion);
  }

  // Jump flooding: steps from the largest power of two smaller than the region size (or the band) down to 1,
  // followed by steps 2 and 1 again ("JFA+2") to correct most of the errors of the approximation.
  int sourceIndex = 0;
  if (success && (success = this->ReadyQuad(renWin, this->JumpFloodQuad, sliceShader + SeedCodingFS + JumpFloodFS)))
  {
    int maximumDimension = 1;
    for (int axis = 0; axis < 3; ++axis)
    {
      maximumDimension = std::max(maximumDimension, workingRegion[axis * 2 + 1] - workingRegion[axis * 2] + 1);
    }
    std::vector<int> steps;
    for (int step = std::min(vtkMath::NearestPowerOfTwo(maximumDimension) / 2, BandVoxels); step >= 1; step /= 2)
    {
      steps.push_back(step);
    }
    steps.push_back(2);
    steps.push_back(1);
    vtkShaderProgram* program = this->JumpFloodQuad->Program;
    this->SetGridUniforms(program);
    SetUniformMatrix3x3(program, "gridToPhysical", this->GridToPhysical);
    float regionMinimum[3] = { static_cast<float>(workingRegion[0]), static_cast<float>(workingRegion[2]), static_cast<float>(workingRegion[4]) };
    float regionMaximum[3] = { static_cast<float>(workingRegion[1]), static_cast<float>(workingRegion[3]), static_cast<float>(workingRegion[5]) };
    program->SetUniform3f("regionMinimum", regionMinimum);
    program->SetUniform3f("regionMaximum", regionMaximum);
    for (int step : steps)
    {
      if (!success)
      {
        break;
      }
      seedTextures[sourceIndex]->Activate();
      program->SetUniformi("seedTexture", seedTextures[sourceIndex]->GetTextureUnit());
      program->SetUniformi("stepSize", step);
      success = this->DrawSlices(this->JumpFloodQuad, seedTextures[1 - sourceIndex], workingRegion);
      seedTextures[sourceIndex]->Deactivate();
      sourceIndex = 1 - sourceIndex;
    }
  }

  // Signed distance
  if (success && (success = this->ReadyQuad(renWin, this->FinalizeQuad, sliceShader + InsideFS + SeedCodingFS + FinalizeFS)))
  {
    vtkShaderProgram* program = this->FinalizeQuad->Program;
    seedTextures[sourceIndex]->Activate();
    this->SetGridUniforms(program);
    program->SetUniformi("maskTexture", maskTexture->GetTextureUnit());
    program->SetUniformi("seedTexture", seedTextures[sourceIndex]->GetTextureUnit());
    SetUniformMatrix3x3(program, "gridToPhysical", this->GridToPhysical);
    program->SetUniformf("bandDistance", static_cast<float>(BandVoxels * std::min({ voxelSizes[0], voxelSizes[1], voxelSizes[2] })));
    success = this->DrawSlices(this->FinalizeQuad, original, workingRegion);
    seedTextures[sourceIndex]->Deactivate();
  }

  // Separable Gaussian of source (x: source -> target, y: target -> temporary, z: temporary -> target),
  // the source is not modified
  auto gaussian = [&](vtkTextureObject* source, vtkTextureObject* target)
  {
    vtkShaderProgram* program = this->SmoothQuad->Program;
    this->SetGridUniforms(program);
    vtkTextureObject* passes[3][2] = { { source, target }, { target, temporary }, { temporary, target } };
    for (int axis = 0; axis < 3 && success; ++axis)
    {
      double sigma = sigmaMm / voxelSizes[axis];
      int radius = smoothingRadius[axis];
      float weights[16] = { 0.0f };
      double total = 0.0;
      for (int i = 0; i <= radius; ++i)
      {
        weights[i] = static_cast<float>(std::exp(-(i * i) / (2.0 * sigma * sigma)));
        total += (i == 0 ? 1.0 : 2.0) * weights[i];
      }
      for (int i = 0; i <= radius; ++i)
      {
        weights[i] = static_cast<float>(weights[i] / total);
      }
      program->SetUniformi("radius", radius);
      program->SetUniform1fv("weights", 16, weights);
      float axisVector[3] = { axis == 0 ? 1.0f : 0.0f, axis == 1 ? 1.0f : 0.0f, axis == 2 ? 1.0f : 0.0f };
      program->SetUniform3f("axis", axisVector);
      passes[axis][0]->Activate();
      program->SetUniformi("inputTexture", passes[axis][0]->GetTextureUnit());
      success = this->DrawSlices(this->SmoothQuad, passes[axis][1], workingRegion);
      passes[axis][0]->Deactivate();
    }
  };
  if (sigmaMm > 0.0)
  {
    if (success && (success = this->ReadyQuad(renWin, this->SmoothQuad, sliceShader + SmoothFS)))
    {
      gaussian(original, smoothed);
    }
    if (success)
    {
      gaussian(smoothed, smoothedTwice);
    }
  }
  else
  {
    // without smoothing the result is the original
    smoothed = original;
    smoothedTwice = original;
  }

  // Result, in the inner region
  if (success && (success = this->ReadyQuad(renWin, this->CombineQuad, sliceShader + CombineFS)))
  {
    vtkShaderProgram* program = this->CombineQuad->Program;
    original->Activate();
    smoothed->Activate();
    smoothedTwice->Activate();
    this->SetGridUniforms(program);
    program->SetUniformi("originalTexture", original->GetTextureUnit());
    program->SetUniformi("smoothedTexture", smoothed->GetTextureUnit());
    program->SetUniformi("smoothedTwiceTexture", smoothedTwice->GetTextureUnit());
    success = this->DrawSlices(this->CombineQuad, resultTexture, innerRegion);
    original->Deactivate();
    smoothed->Deactivate();
    smoothedTwice->Deactivate();
  }

  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, 0);
  this->Framebuffer->RestorePreviousBindingsAndBuffers();
  maskTexture->Deactivate();

  // Only the result is kept
  for (vtkTextureObject* texture : { seedTextures[0].GetPointer(), seedTextures[1].GetPointer(), maskTexture.GetPointer() })
  {
    texture->ReleaseGraphicsResources(renWin);
  }
  for (int i = 0; i < 4; ++i)
  {
    distanceTextures[i]->ReleaseGraphicsResources(renWin);
  }
  if (fullUpdate && this->DistanceTexture && this->DistanceTexture != resultTexture)
  {
    this->DistanceTexture->ReleaseGraphicsResources(renWin);
  }
  this->DistanceTexture = resultTexture;

  if (success)
  {
    success = this->ComputeColors(renWin, fullUpdate ? nullptr : innerRegion);
  }
  if (success)
  {
    this->ComputedSmoothingFactor = this->External->SmoothingFactor;
    this->DistanceFieldTime.Modified();
    this->External->NumberOfDistanceFieldComputations++;
  }
  return success;
}

//----------------------------------------------------------------------------
bool vtkSegmentationLabelmapSurfaceMapper::vtkInternal::ComputeColors(vtkOpenGLRenderWindow* renWin, const int* region)
{
  bool sameGrid = (this->ColorTexture != nullptr);
  for (int axis = 0; axis < 3 && sameGrid; ++axis)
  {
    unsigned int dimension = (axis == 0 ? this->ColorTexture->GetWidth() : (axis == 1 ? this->ColorTexture->GetHeight() : this->ColorTexture->GetDepth()));
    sameGrid = (static_cast<int>(dimension) == this->GridDimensions[axis]);
  }
  if (!sameGrid)
  {
    if (this->ColorTexture)
    {
      this->ColorTexture->ReleaseGraphicsResources(renWin);
    }
    this->ColorTexture = this->CreateTexture(renWin, GL_RGBA8, 4, VTK_UNSIGNED_CHAR, false);
    if (!this->ColorTexture)
    {
      vtkErrorWithObjectMacro(this->External, "ComputeColors: failed to allocate texture");
      return false;
    }
    region = nullptr;
  }
  int wholeGrid[6] = { 0, this->GridDimensions[0] - 1, 0, this->GridDimensions[1] - 1, 0, this->GridDimensions[2] - 1 };

  vtkOpenGLState* ostate = renWin->GetState();
  vtkOpenGLState::ScopedglViewport savedViewport(ostate);
  vtkOpenGLState::ScopedglScissor savedScissor(ostate);
  vtkOpenGLState::ScopedglColorMask savedColorMask(ostate);
  vtkOpenGLState::ScopedglEnableDisable savedDepthTest(ostate, GL_DEPTH_TEST);
  vtkOpenGLState::ScopedglEnableDisable savedBlend(ostate, GL_BLEND);
  vtkOpenGLState::ScopedglEnableDisable savedScissorTest(ostate, GL_SCISSOR_TEST);
  vtkOpenGLState::ScopedglEnableDisable savedCullFace(ostate, GL_CULL_FACE);
  ostate->vtkglDisable(GL_DEPTH_TEST);
  ostate->vtkglDisable(GL_BLEND);
  ostate->vtkglDisable(GL_CULL_FACE);
  ostate->vtkglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  ostate->vtkglViewport(0, 0, this->GridDimensions[0], this->GridDimensions[1]);

  this->Framebuffer->SetContext(renWin);
  this->Framebuffer->SaveCurrentBindingsAndBuffers();
  this->Framebuffer->Bind();
  this->Framebuffer->ActivateDrawBuffers(1);
  bool success = this->ReadyQuad(renWin, this->ColorQuad, std::string(SliceCommonFS) + LabelFS + ColorFS);
  if (success)
  {
    vtkShaderProgram* program = this->ColorQuad->Program;
    this->LabelTexture->Activate();
    this->PaletteTexture->Activate();
    this->SetGridUniforms(program);
    this->SetInsideUniforms(program);
    success = this->DrawSlices(this->ColorQuad, this->ColorTexture, region ? region : wholeGrid);
    this->LabelTexture->Deactivate();
    this->PaletteTexture->Deactivate();
  }
  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, 0);
  this->Framebuffer->RestorePreviousBindingsAndBuffers();
  if (success)
  {
    this->ColorsNeedUpdate = false;
  }
  return success;
}

//----------------------------------------------------------------------------
bool vtkSegmentationLabelmapSurfaceMapper::vtkInternal::ReadyLowResolutionFramebuffer(vtkOpenGLRenderWindow* renWin, const int size[2])
{
  if (!this->LowResolutionFramebuffer)
  {
    this->LowResolutionFramebuffer = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->LowResolutionFramebuffer->SetContext(renWin);
    // color (premultiplied, interpolated when scaled up), depth (in RGB), normal
    for (int i = 0; i < 3; ++i)
    {
      vtkSmartPointer<vtkTextureObject> texture = vtkSmartPointer<vtkTextureObject>::New();
      texture->SetContext(renWin);
      texture->SetWrapS(vtkTextureObject::ClampToEdge);
      texture->SetWrapT(vtkTextureObject::ClampToEdge);
      texture->SetMinificationFilter(i == 0 ? vtkTextureObject::Linear : vtkTextureObject::Nearest);
      texture->SetMagnificationFilter(i == 0 ? vtkTextureObject::Linear : vtkTextureObject::Nearest);
      if (!texture->Allocate2D(size[0], size[1], 4, VTK_UNSIGNED_CHAR))
      {
        vtkErrorWithObjectMacro(this->External, "Failed to allocate the image for rendering at a lower resolution");
        this->LowResolutionFramebuffer = nullptr;
        return false;
      }
      this->LowResolutionTextures[i] = texture;
    }
    renWin->GetState()->PushDrawFramebufferBinding();
    this->LowResolutionFramebuffer->Bind(GL_DRAW_FRAMEBUFFER);
    for (int i = 0; i < 3; ++i)
    {
      this->LowResolutionFramebuffer->AddColorAttachment(i, this->LowResolutionTextures[i]);
    }
    renWin->GetState()->PopDrawFramebufferBinding();
  }
  for (int i = 0; i < 3; ++i)
  {
    if (static_cast<int>(this->LowResolutionTextures[i]->GetWidth()) != size[0] || static_cast<int>(this->LowResolutionTextures[i]->GetHeight()) != size[1])
    {
      this->LowResolutionTextures[i]->Resize(size[0], size[1]);
    }
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSegmentationLabelmapSurfaceMapper::vtkInternal::CaptureOpaqueDepth(vtkOpenGLRenderer* ren)
{
  // Copy the depth buffer of the framebuffer that is bound for reading, which contains the opaque geometry while
  // translucent geometry is rendered: the render framebuffer of the window, or the framebuffer of a render pass
  // that renders the opaque geometry elsewhere (for example vtkSSAOPass; depth peeling and order independent
  // translucency passes bind their framebuffers only for drawing). The render framebuffer of the window must not
  // be used, because it contains the depth of the previous frame while a pass renders into its own framebuffer.
  // Same as in vtkOpenGLGPUVolumeRayCastMapper.
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(ren->GetRenderWindow());
  int width = 0;
  int height = 0;
  int lowerLeft[2] = { 0, 0 };
  ren->GetTiledSizeAndOrigin(&width, &height, lowerLeft, lowerLeft + 1);
  if (width <= 0 || height <= 0)
  {
    return false;
  }
  if (!this->OpaqueDepthTexture)
  {
    this->OpaqueDepthTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->OpaqueDepthTexture->SetContext(renWin);
    this->OpaqueDepthTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->OpaqueDepthTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->OpaqueDepthTexture->SetMagnificationFilter(vtkTextureObject::Nearest);
    this->OpaqueDepthTexture->SetMinificationFilter(vtkTextureObject::Nearest);
    if (renWin->GetStencilCapable())
    {
      this->OpaqueDepthTexture->AllocateDepthStencil(width, height);
    }
    else
    {
#ifdef GL_ES_VERSION_3_0
      this->OpaqueDepthTexture->AllocateDepth(width, height, vtkTextureObject::Fixed24);
#else
      this->OpaqueDepthTexture->AllocateDepth(width, height, vtkTextureObject::Fixed32);
#endif
    }
    this->OpaqueDepthCopyColorTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->OpaqueDepthCopyColorTexture->SetContext(renWin);
    this->OpaqueDepthCopyColorTexture->Allocate2D(width, height, 4, VTK_UNSIGNED_CHAR);
    this->OpaqueDepthFramebuffer = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->OpaqueDepthFramebuffer->SetContext(renWin);
    renWin->GetState()->PushDrawFramebufferBinding();
    this->OpaqueDepthFramebuffer->Bind(GL_DRAW_FRAMEBUFFER);
    this->OpaqueDepthFramebuffer->AddDepthAttachment(this->OpaqueDepthTexture);
    this->OpaqueDepthFramebuffer->AddColorAttachment(0, this->OpaqueDepthCopyColorTexture);
    renWin->GetState()->PopDrawFramebufferBinding();
  }
  this->OpaqueDepthTexture->Resize(width, height);
  this->OpaqueDepthCopyColorTexture->Resize(width, height);

  vtkOpenGLState* ostate = renWin->GetState();
  ostate->PushDrawFramebufferBinding();
  this->OpaqueDepthFramebuffer->Bind(GL_DRAW_FRAMEBUFFER);
  ostate->vtkglBlitFramebuffer(lowerLeft[0], lowerLeft[1], lowerLeft[0] + width, lowerLeft[1] + height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  ostate->PopDrawFramebufferBinding();
  return true;
}

//----------------------------------------------------------------------------
vtkShaderProgram* vtkSegmentationLabelmapSurfaceMapper::vtkInternal::ReadySurfaceProgram(vtkOpenGLRenderer* ren,
                                                                                         vtkActor* actor,
                                                                                         const std::vector<vtkOpenGLRenderPass*>& renderPasses,
                                                                                         bool lowResolution,
                                                                                         vtkShaderProgram*& cachedProgram,
                                                                                         std::string& cachedKey)
{
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(ren->GetRenderWindow());
  // Lighting code depends on the lights of the renderer
  std::ostringstream lightingImplementation;
  int lightingComplexity = ren->GetLightingComplexity();
  int lightingCount = ren->GetLightingCount();
  if (lightingComplexity == 0)
  {
    lightingImplementation << "  diffuse = vec3(1.0);\n";
  }
  else if (lightingComplexity == 1)
  {
    lightingImplementation << "  float df = max(0.0, normalVC.z);\n"
                              "  diffuse = df * lightColor0;\n"
                              "  specular = pow(df, specularPower) * lightColor0;\n";
  }
  else
  {
    for (int lightIndex = 0; lightIndex < lightingCount; ++lightIndex)
    {
      lightingImplementation << "  {\n"
                                "    float df = max(0.0, dot(normalVC, -lightDirectionVC"
                             << lightIndex << "));\n"
                             << "    diffuse += df * lightColor" << lightIndex << ";\n"
                             << "    if (dot(normalVC, lightDirectionVC" << lightIndex << ") < 0.0)\n"
                             << "    {\n"
                             << "      specular += sign(df) * pow(max(0.0, dot(reflect(lightDirectionVC" << lightIndex
                             << ", normalVC), viewDirectionVC)), specularPower) * lightColor" << lightIndex << ";\n"
                             << "    }\n"
                                "  }\n";
    }
  }
  std::ostringstream programKey;
  programKey << lightingImplementation.str();
  for (vtkOpenGLRenderPass* renderPass : lowResolution ? std::vector<vtkOpenGLRenderPass*>() : renderPasses)
  {
    programKey << renderPass << ":" << renderPass->GetShaderStageMTime() << ";";
  }
  if (!cachedProgram || cachedKey != programKey.str())
  {
    std::string vertexShader = SurfaceVS;
    std::string geometryShader;
    std::string fragmentShader = SurfaceFS;
    vtkShaderProgram::Substitute(fragmentShader, "//SEG::Output::Impl", lowResolution ? LowResolutionOutputImpl : SurfaceOutputImpl);
    vtkShaderProgram::Substitute(fragmentShader, "//VTK::Light::Dec", lightingComplexity > 0 ? ren->GetLightingUniforms() : "");
    vtkShaderProgram::Substitute(fragmentShader, "//VTK::Light::Impl", lightingImplementation.str());
    if (!lowResolution)
    {
      ApplyRenderPasses(renderPasses, vertexShader, geometryShader, fragmentShader, this->External, actor);
    }
    cachedProgram = renWin->GetShaderCache()->ReadyShaderProgram(vertexShader.c_str(), fragmentShader.c_str(), geometryShader.c_str());
    cachedKey = programKey.str();
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(cachedProgram);
  }
  return (cachedProgram && cachedProgram->GetCompiled()) ? cachedProgram : nullptr;
}

//----------------------------------------------------------------------------
void vtkSegmentationLabelmapSurfaceMapper::vtkInternal::RenderSurface(vtkOpenGLRenderer* ren, vtkActor* actor)
{
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(ren->GetRenderWindow());
  vtkTextureObject* distanceTexture = this->DistanceTexture;
  if (!distanceTexture)
  {
    return;
  }

  // Render passes (depth peeling, order independent translucency) modify the shader
  std::vector<vtkOpenGLRenderPass*> renderPasses;
  vtkInformation* info = actor->GetPropertyKeys();
  if (info && info->Has(vtkOpenGLRenderPass::RenderPasses()))
  {
    int numberOfRenderPasses = info->Length(vtkOpenGLRenderPass::RenderPasses());
    for (int i = 0; i < numberOfRenderPasses; ++i)
    {
      renderPasses.push_back(static_cast<vtkOpenGLRenderPass*>(info->Get(vtkOpenGLRenderPass::RenderPasses(), i)));
    }
  }

  // Image sample distance: rays are cast for an image of a lower resolution, which is scaled up to the viewport
  vtkOpenGLState* ostate = renWin->GetState();
  int viewport[4] = { 0, 0, 1, 1 };
  ostate->vtkglGetIntegerv(GL_VIEWPORT, viewport);
  int lowResolutionSize[2] = { viewport[2], viewport[3] };
  for (int i = 0; i < 2; ++i)
  {
    lowResolutionSize[i] = std::max(1, static_cast<int>(std::ceil(viewport[2 + i] / this->External->ImageSampleDistance)));
  }
  bool lowResolution = (lowResolutionSize[0] < viewport[2] || lowResolutionSize[1] < viewport[3]);
  if (lowResolution && !this->ReadyLowResolutionFramebuffer(renWin, lowResolutionSize))
  {
    lowResolution = false;
  }

  vtkShaderProgram*& cachedProgram = lowResolution ? this->LowResolutionProgram : this->SurfaceProgram;
  std::string& cachedKey = lowResolution ? this->LowResolutionProgramKey : this->SurfaceProgramKey;
  vtkShaderProgram* program = this->ReadySurfaceProgram(ren, actor, renderPasses, lowResolution, cachedProgram, cachedKey);
  if (!program)
  {
    vtkErrorWithObjectMacro(this->External, "RenderSurface: failed to compile shader");
    this->Failed = true;
    return;
  }
  int lightingComplexity = ren->GetLightingComplexity();

  // Matrices
  vtkCamera* camera = ren->GetActiveCamera();
  vtkNew<vtkMatrix4x4> mcwc;
  actor->GetMatrix(mcwc);
  vtkNew<vtkMatrix4x4> mcvc;
  vtkMatrix4x4::Multiply4x4(camera->GetModelViewTransformMatrix(), mcwc, mcvc);
  vtkNew<vtkMatrix4x4> mcdc;
  vtkMatrix4x4::Multiply4x4(camera->GetProjectionTransformMatrix(ren->GetTiledAspectRatio(), -1, 1), mcvc, mcdc);
  vtkNew<vtkMatrix4x4> dcmc;
  vtkMatrix4x4::Invert(mcdc, dcmc);
  SetUniformMatrix4x4(program, "MCDCMatrix", mcdc);
  SetUniformMatrix4x4(program, "DCMCMatrix", dcmc);
  SetUniformMatrix4x4(program, "MCVCMatrix", mcvc);
  // Gradients are transformed by the inverse transpose
  double normalMatrix[3][3];
  for (int row = 0; row < 3; ++row)
  {
    for (int column = 0; column < 3; ++column)
    {
      normalMatrix[row][column] = mcvc->GetElement(row, column);
    }
  }
  vtkMath::Invert3x3(normalMatrix, normalMatrix);
  vtkMath::Transpose3x3(normalMatrix, normalMatrix);
  SetUniformMatrix3x3(program, "normalMatrix", normalMatrix);

  vtkNew<vtkMatrix4x4> wcmc;
  vtkMatrix4x4::Invert(mcwc, wcmc);
  double eyePosition[4] = { 0.0, 0.0, 0.0, 1.0 };
  camera->GetPosition(eyePosition);
  double eyePositionMC[4];
  wcmc->MultiplyPoint(eyePosition, eyePositionMC);
  for (int i = 0; i < 3; ++i)
  {
    eyePositionMC[i] /= eyePositionMC[3];
  }
  double viewDirection[4] = { 0.0, 0.0, 0.0, 0.0 };
  camera->GetDirectionOfProjection(viewDirection);
  double viewDirectionMC[4];
  wcmc->MultiplyPoint(viewDirection, viewDirectionMC);
  vtkMath::Normalize(viewDirectionMC);
  program->SetUniform3f("eyePositionMC", eyePositionMC);
  program->SetUniform3f("viewDirectionMC", viewDirectionMC);
  program->SetUniformi("parallelProjection", camera->GetParallelProjection() ? 1 : 0);

  double boxMin[3] = { this->Bounds[0], this->Bounds[2], this->Bounds[4] };
  double boxMax[3] = { this->Bounds[1], this->Bounds[3], this->Bounds[5] };
  program->SetUniform3f("boxMin", boxMin);
  program->SetUniform3f("boxMax", boxMax);
  program->SetUniform3f("mcToGridScale", this->MCToGridScale);
  program->SetUniform3f("mcToGridOffset", this->MCToGridOffset);
  SetUniformMatrix3x3(program, "gridToPhysical", this->GridToPhysical);
  this->SetGridUniforms(program);

  // Distances are in physical (image) space; steps are along the ray in model (IJK) space.
  double maximumVoxelSize = 0.0;
  double minimumDownsampling = VTK_DOUBLE_MAX;
  for (int axis = 0; axis < 3; ++axis)
  {
    double column[3] = { this->GridToPhysical[0][axis], this->GridToPhysical[1][axis], this->GridToPhysical[2][axis] };
    maximumVoxelSize = std::max(maximumVoxelSize, vtkMath::Norm(column) / this->Downsampling[axis]);
    minimumDownsampling = std::min(minimumDownsampling, static_cast<double>(this->Downsampling[axis]));
  }
  program->SetUniformf("mcPerMm", static_cast<float>(1.0 / std::max(maximumVoxelSize, 1e-6)));
  program->SetUniformf("minimumStep", static_cast<float>(0.2 * minimumDownsampling));

  vtkProperty* property = actor->GetProperty();
  program->SetUniformf("ambientIntensity", static_cast<float>(property->GetAmbient()));
  program->SetUniformf("diffuseIntensity", static_cast<float>(property->GetDiffuse()));
  program->SetUniformf("specularIntensity", static_cast<float>(property->GetSpecular()));
  program->SetUniformf("specularPower", static_cast<float>(property->GetSpecularPower()));
  program->SetUniform3f("specularColor", property->GetSpecularColor());
  program->SetUniformf("opacity", static_cast<float>(property->GetOpacity()));
  if (lightingComplexity > 0)
  {
    ren->UpdateLightingUniforms(program);
  }

  // Clipping planes, from world to model coordinates: n.x + d >= 0 is kept
  float clippingPlanes[16][4];
  int numberOfClippingPlanes = 0;
  vtkPlaneCollection* planes = this->External->GetClippingPlanes();
  if (planes)
  {
    for (int i = 0; i < planes->GetNumberOfItems() && numberOfClippingPlanes < 16; ++i)
    {
      vtkPlane* plane = planes->GetItem(i);
      double normal[3];
      double origin[3];
      plane->GetNormal(normal);
      plane->GetOrigin(origin);
      double planeWC[4] = { normal[0], normal[1], normal[2], -vtkMath::Dot(normal, origin) };
      // plane in model coordinates = transpose(MCWC) * plane in world coordinates
      double planeMC[4] = { 0.0, 0.0, 0.0, 0.0 };
      for (int column = 0; column < 4; ++column)
      {
        for (int row = 0; row < 4; ++row)
        {
          planeMC[column] += mcwc->GetElement(row, column) * planeWC[row];
        }
      }
      for (int component = 0; component < 4; ++component)
      {
        clippingPlanes[numberOfClippingPlanes][component] = static_cast<float>(planeMC[component]);
      }
      numberOfClippingPlanes++;
    }
  }
  program->SetUniformi("numberOfClippingPlanes", numberOfClippingPlanes);
  program->SetUniformi("keepWhereAnyPlaneKeeps", this->External->KeepWhereAnyClippingPlaneKeeps ? 1 : 0);
  if (numberOfClippingPlanes > 0)
  {
    program->SetUniform4fv("clippingPlanesMC", numberOfClippingPlanes, clippingPlanes);
  }
  program->SetUniformi("capClippedSurface", this->External->CapClippedSurface ? 1 : 0);
  program->SetUniformf("capOpacity", static_cast<float>(this->External->CapOpacity));
  program->SetUniformi("clippingOutline", this->External->ClippingOutline ? 1 : 0);
  float outlineColor[3] = { static_cast<float>(this->External->OutlineColor[0]),
                            static_cast<float>(this->External->OutlineColor[1]),
                            static_cast<float>(this->External->OutlineColor[2]) };
  program->SetUniform3f("outlineColor", outlineColor);
  program->SetUniformf("outlineWidth", static_cast<float>(this->External->OutlineWidth));

  distanceTexture->Activate();
  this->ColorTexture->Activate();
  this->PaletteTexture->Activate();
  program->SetUniformi("distanceTexture", distanceTexture->GetTextureUnit());
  program->SetUniformi("colorTexture", this->ColorTexture->GetTextureUnit());

  // Translucent surfaces: the depth test does not hide surfaces behind opaque geometry, rays stop there instead
  bool stopAtOpaqueDepth = actor->IsRenderingTranslucentPolygonalGeometry() && this->CaptureOpaqueDepth(ren);
  program->SetUniformi("stopAtOpaqueDepth", stopAtOpaqueDepth ? 1 : 0);
  // Viewport (or the image of a lower resolution): for reading the opaque depth and for the size of pixels (clipping outline)
  float viewportOrigin[2] = { static_cast<float>(viewport[0]), static_cast<float>(viewport[1]) };
  float viewportSize[2] = { static_cast<float>(viewport[2]), static_cast<float>(viewport[3]) };
  float rayImageOrigin[2] = { 0.0f, 0.0f };
  float rayImageSize[2] = { static_cast<float>(lowResolutionSize[0]), static_cast<float>(lowResolutionSize[1]) };
  float fullResolutionScale[2] = { viewportSize[0] / rayImageSize[0], viewportSize[1] / rayImageSize[1] };
  program->SetUniform2f("viewportOrigin", lowResolution ? rayImageOrigin : viewportOrigin);
  program->SetUniform2f("viewportSize", lowResolution ? rayImageSize : viewportSize);
  program->SetUniform2f("fullResolutionScale", fullResolutionScale);
  if (stopAtOpaqueDepth)
  {
    this->OpaqueDepthTexture->Activate();
    program->SetUniformi("opaqueDepthTexture", this->OpaqueDepthTexture->GetTextureUnit());
  }
  else
  {
    // all samplers must refer to a texture of the right type, even if they are not used
    program->SetUniformi("opaqueDepthTexture", this->PaletteTexture->GetTextureUnit());
  }

  vtkOpenGLState::ScopedglEnableDisable savedCullFace(ostate, GL_CULL_FACE);
  ostate->vtkglDisable(GL_CULL_FACE);
  if (!lowResolution)
  {
    SetRenderPassParameters(renderPasses, program, this->External, actor, this->SurfaceVAO, ostate);
    this->SurfaceVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    this->SurfaceVAO->Release();
  }
  else
  {
    // Rays are cast for the image of the lower resolution
    vtkOpenGLState::ScopedglViewport savedViewport(ostate);
    vtkOpenGLState::ScopedglScissor savedScissor(ostate);
    vtkOpenGLState::ScopedglColorMask savedColorMask(ostate);
    vtkOpenGLState::ScopedglClearColor savedClearColor(ostate);
    vtkOpenGLState::ScopedglEnableDisable savedDepthTest(ostate, GL_DEPTH_TEST);
    vtkOpenGLState::ScopedglEnableDisable savedBlend(ostate, GL_BLEND);
    vtkOpenGLState::ScopedglEnableDisable savedScissorTest(ostate, GL_SCISSOR_TEST);
    this->LowResolutionFramebuffer->SaveCurrentBindingsAndBuffers();
    this->LowResolutionFramebuffer->Bind();
    ActivateDrawBuffers(ostate, 3);
    ostate->vtkglDisable(GL_DEPTH_TEST);
    ostate->vtkglDisable(GL_BLEND);
    ostate->vtkglDisable(GL_SCISSOR_TEST);
    ostate->vtkglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    ostate->vtkglViewport(0, 0, lowResolutionSize[0], lowResolutionSize[1]);
    ostate->vtkglClearColor(0.0, 0.0, 0.0, 0.0);
    ostate->vtkglClear(GL_COLOR_BUFFER_BIT);
    this->SurfaceVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    this->SurfaceVAO->Release();
    this->LowResolutionFramebuffer->RestorePreviousBindingsAndBuffers();
  }
  vtkOpenGLStaticCheckErrorMacro("failed after rendering segmentation surface");

  distanceTexture->Deactivate();
  this->ColorTexture->Deactivate();
  this->PaletteTexture->Deactivate();
  if (stopAtOpaqueDepth)
  {
    this->OpaqueDepthTexture->Deactivate();
  }
  if (!lowResolution)
  {
    return;
  }

  // The image is scaled up to the viewport, through the render passes
  std::ostringstream compositeKey;
  for (vtkOpenGLRenderPass* renderPass : renderPasses)
  {
    compositeKey << renderPass << ":" << renderPass->GetShaderStageMTime() << ";";
  }
  if (!this->CompositeProgram || this->CompositeProgramKey != compositeKey.str())
  {
    std::string vertexShader = SurfaceVS;
    std::string geometryShader;
    std::string fragmentShader = CompositeFS;
    ApplyRenderPasses(renderPasses, vertexShader, geometryShader, fragmentShader, this->External, actor);
    this->CompositeProgram = renWin->GetShaderCache()->ReadyShaderProgram(vertexShader.c_str(), fragmentShader.c_str(), geometryShader.c_str());
    this->CompositeProgramKey = compositeKey.str();
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->CompositeProgram);
  }
  vtkShaderProgram* composite = this->CompositeProgram;
  if (!composite || !composite->GetCompiled())
  {
    vtkErrorWithObjectMacro(this->External, "RenderSurface: failed to compile shader");
    this->Failed = true;
    return;
  }
  const char* textureNames[3] = { "lowResolutionColorTexture", "lowResolutionDepthTexture", "lowResolutionNormalTexture" };
  for (int i = 0; i < 3; ++i)
  {
    this->LowResolutionTextures[i]->Activate();
    composite->SetUniformi(textureNames[i], this->LowResolutionTextures[i]->GetTextureUnit());
  }
  composite->SetUniform2f("viewportOrigin", viewportOrigin);
  composite->SetUniform2f("viewportSize", viewportSize);
  composite->SetUniform2f("lowResolutionSize", rayImageSize);
  vtkNew<vtkMatrix4x4> projection;
  projection->DeepCopy(camera->GetProjectionTransformMatrix(ren->GetTiledAspectRatio(), -1, 1));
  vtkNew<vtkMatrix4x4> inverseProjection;
  vtkMatrix4x4::Invert(projection, inverseProjection);
  SetUniformMatrix4x4(composite, "projectionMatrix", projection);
  SetUniformMatrix4x4(composite, "inverseProjectionMatrix", inverseProjection);
  SetRenderPassParameters(renderPasses, composite, this->External, actor, this->SurfaceVAO, ostate);
  this->SurfaceVAO->Bind();
  glDrawArrays(GL_TRIANGLES, 0, 3);
  this->SurfaceVAO->Release();
  vtkOpenGLStaticCheckErrorMacro("failed after scaling up segmentation surface");
  for (int i = 0; i < 3; ++i)
  {
    this->LowResolutionTextures[i]->Deactivate();
  }
}

//----------------------------------------------------------------------------
void vtkSegmentationLabelmapSurfaceMapper::vtkInternal::ReleaseGraphicsResources(vtkWindow* window)
{
  vtkOpenGLQuadHelper** quads[] = {
    &this->SeedQuad, &this->JumpFloodQuad, &this->FinalizeQuad, &this->SmoothQuad, &this->CombineQuad, &this->MaskQuad, &this->ColorQuad,
  };
  for (vtkOpenGLQuadHelper** quad : quads)
  {
    delete *quad;
    *quad = nullptr;
  }
  for (vtkTextureObject* texture : { this->LabelTexture.GetPointer(),
                                     this->PaletteTexture.GetPointer(),
                                     this->DistanceTexture.GetPointer(),
                                     this->ColorTexture.GetPointer(),
                                     this->LowResolutionTextures[0].GetPointer(),
                                     this->LowResolutionTextures[1].GetPointer(),
                                     this->LowResolutionTextures[2].GetPointer(),
                                     this->OpaqueDepthTexture.GetPointer(),
                                     this->OpaqueDepthCopyColorTexture.GetPointer() })
  {
    if (texture)
    {
      texture->ReleaseGraphicsResources(window);
    }
  }
  if (this->OpaqueDepthFramebuffer)
  {
    this->OpaqueDepthFramebuffer->ReleaseGraphicsResources(window);
  }
  if (this->LowResolutionFramebuffer)
  {
    this->LowResolutionFramebuffer->ReleaseGraphicsResources(window);
  }
  this->OpaqueDepthTexture = nullptr;
  this->OpaqueDepthCopyColorTexture = nullptr;
  this->OpaqueDepthFramebuffer = nullptr;
  this->LowResolutionFramebuffer = nullptr;
  for (int i = 0; i < 3; ++i)
  {
    this->LowResolutionTextures[i] = nullptr;
  }
  this->ColorTexture = nullptr;
  this->ColorsNeedUpdate = true;
  this->LabelTexture = nullptr;
  this->PaletteTexture = nullptr;
  this->DistanceTexture = nullptr;
  this->UploadedLabelColors.clear();
  this->Framebuffer->ReleaseGraphicsResources(window);
  this->SurfaceVAO->ReleaseGraphicsResources();
  this->SurfaceProgram = nullptr;
  this->SurfaceProgramKey.clear();
  this->LowResolutionProgram = nullptr;
  this->LowResolutionProgramKey.clear();
  this->CompositeProgram = nullptr;
  this->CompositeProgramKey.clear();
  this->DistanceFieldNeedsUpdate = true;
  this->Context = nullptr;
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSegmentationLabelmapSurfaceMapper);

//----------------------------------------------------------------------------
vtkSegmentationLabelmapSurfaceMapper::vtkSegmentationLabelmapSurfaceMapper()
{
  this->Internal = new vtkInternal(this);
}

//----------------------------------------------------------------------------
vtkSegmentationLabelmapSurfaceMapper::~vtkSegmentationLabelmapSurfaceMapper()
{
  // Graphics resources are released by ReleaseGraphicsResources (called when the actor is removed from the renderer)
  vtkInternal* d = this->Internal;
  vtkOpenGLQuadHelper* quads[] = { d->SeedQuad, d->JumpFloodQuad, d->FinalizeQuad, d->SmoothQuad, d->CombineQuad, d->MaskQuad, d->ColorQuad };
  for (vtkOpenGLQuadHelper* quad : quads)
  {
    delete quad;
  }
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkSegmentationLabelmapSurfaceMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SmoothingFactor: " << this->SmoothingFactor << "\n";
  os << indent << "ImageSampleDistance: " << this->ImageSampleDistance << "\n";
  os << indent << "CapClippedSurface: " << (this->CapClippedSurface ? "true" : "false") << "\n";
  os << indent << "CapOpacity: " << this->CapOpacity << "\n";
  os << indent << "KeepWhereAnyClippingPlaneKeeps: " << (this->KeepWhereAnyClippingPlaneKeeps ? "true" : "false") << "\n";
  os << indent << "ClippingOutline: " << (this->ClippingOutline ? "true" : "false") << "\n";
  os << indent << "OutlineColor: " << this->OutlineColor[0] << ", " << this->OutlineColor[1] << ", " << this->OutlineColor[2] << "\n";
  os << indent << "OutlineWidth: " << this->OutlineWidth << "\n";
  os << indent << "MaximumNumberOfVoxels: " << this->MaximumNumberOfVoxels << "\n";
  os << indent << "NumberOfDistanceFieldComputations: " << this->NumberOfDistanceFieldComputations << "\n";
}

//----------------------------------------------------------------------------
int vtkSegmentationLabelmapSurfaceMapper::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  // The labelmap is set by SetLabelmap, not through the pipeline
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//----------------------------------------------------------------------------
void vtkSegmentationLabelmapSurfaceMapper::SetLabelmap(vtkOrientedImageData* labelmap)
{
  if (this->Internal->Labelmap == labelmap)
  {
    return;
  }
  this->Internal->Labelmap = labelmap;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkOrientedImageData* vtkSegmentationLabelmapSurfaceMapper::GetLabelmap()
{
  return this->Internal->Labelmap;
}

//----------------------------------------------------------------------------
void vtkSegmentationLabelmapSurfaceMapper::SetLabelColor(int labelValue, double r, double g, double b, double opacity /*=1.0*/)
{
  std::array<double, 4> color = { r, g, b, opacity };
  auto it = this->Internal->LabelColors.find(labelValue);
  if (it != this->Internal->LabelColors.end() && it->second == color)
  {
    return;
  }
  this->Internal->LabelColors[labelValue] = color;
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkSegmentationLabelmapSurfaceMapper::IntersectWithRay(const double p1[3], const double p2[3], double t1, double t2, double& t, double xyz[3], double n[3])
{
  vtkOrientedImageData* labelmap = this->Internal->Labelmap;
  if (!this->Internal->PrepareLabelmap() || !labelmap)
  {
    return false;
  }
  std::set<int> shownLabels = this->Internal->GetShownLabels();
  const int* extent = this->Internal->EffectiveExtent;
  // Fraction of the voxel (trilinear interpolation of 0/1 values of voxels) that is in shown segments
  auto isShown = [&](int i, int j, int k)
  {
    if (i < extent[0] || i > extent[1] || j < extent[2] || j > extent[3] || k < extent[4] || k > extent[5])
    {
      return false;
    }
    return shownLabels.count(static_cast<int>(labelmap->GetScalarComponentAsDouble(i, j, k, 0))) > 0;
  };
  auto insideFraction = [&](const double ijk[3])
  {
    int base[3] = { static_cast<int>(std::floor(ijk[0])), static_cast<int>(std::floor(ijk[1])), static_cast<int>(std::floor(ijk[2])) };
    double weight[3] = { ijk[0] - base[0], ijk[1] - base[1], ijk[2] - base[2] };
    double fraction = 0.0;
    for (int corner = 0; corner < 8; ++corner)
    {
      int offset[3] = { corner & 1, (corner >> 1) & 1, (corner >> 2) & 1 };
      if (isShown(base[0] + offset[0], base[1] + offset[1], base[2] + offset[2]))
      {
        fraction += (offset[0] ? weight[0] : 1.0 - weight[0]) * (offset[1] ? weight[1] : 1.0 - weight[1]) * (offset[2] ? weight[2] : 1.0 - weight[2]);
      }
    }
    return fraction;
  };
  auto pointAt = [&](double parameter, double point[3])
  {
    for (int i = 0; i < 3; ++i)
    {
      point[i] = p1[i] + parameter * (p2[i] - p1[i]);
    }
  };

  // Limit the ray to the region of the shown segments
  double tStart = t1;
  double tEnd = t2;
  for (int axis = 0; axis < 3; ++axis)
  {
    double direction = p2[axis] - p1[axis];
    double low = extent[axis * 2] - 1.0;
    double high = extent[axis * 2 + 1] + 1.0;
    if (std::abs(direction) < 1e-12)
    {
      if (p1[axis] < low || p1[axis] > high)
      {
        return false;
      }
      continue;
    }
    double tLow = (low - p1[axis]) / direction;
    double tHigh = (high - p1[axis]) / direction;
    tStart = std::max(tStart, std::min(tLow, tHigh));
    tEnd = std::min(tEnd, std::max(tLow, tHigh));
  }
  if (tStart >= tEnd)
  {
    return false;
  }

  // March in quarter voxel steps, then refine the crossing of the 0.5 level of the inside fraction
  double rayLength = std::sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
  double tStep = 0.25 / std::max(rayLength, 1e-12);
  double point[3];
  double previousT = tStart;
  for (double currentT = tStart; currentT <= tEnd + tStep; currentT += tStep)
  {
    currentT = std::min(currentT, tEnd);
    pointAt(currentT, point);
    if (insideFraction(point) >= 0.5)
    {
      double tOutside = previousT;
      double tInside = currentT;
      for (int iteration = 0; iteration < 10; ++iteration)
      {
        double tMiddle = 0.5 * (tOutside + tInside);
        pointAt(tMiddle, point);
        (insideFraction(point) >= 0.5 ? tInside : tOutside) = tMiddle;
      }
      t = 0.5 * (tOutside + tInside);
      pointAt(t, xyz);
      // Normal: gradient of the inside fraction (pointing outwards)
      for (int axis = 0; axis < 3; ++axis)
      {
        double plus[3] = { xyz[0], xyz[1], xyz[2] };
        double minus[3] = { xyz[0], xyz[1], xyz[2] };
        plus[axis] += 0.5;
        minus[axis] -= 0.5;
        n[axis] = insideFraction(minus) - insideFraction(plus);
      }
      if (vtkMath::Normalize(n) == 0.0)
      {
        for (int axis = 0; axis < 3; ++axis)
        {
          n[axis] = p1[axis] - p2[axis];
        }
        vtkMath::Normalize(n);
      }
      return true;
    }
    previousT = currentT;
    if (currentT >= tEnd)
    {
      break;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
int vtkSegmentationLabelmapSurfaceMapper::GetShownLabelAtPosition(const double ijk[3], double maximumDistance /*=1.5*/)
{
  vtkOrientedImageData* labelmap = this->Internal->Labelmap;
  if (!labelmap || !labelmap->GetPointData() || !labelmap->GetPointData()->GetScalars())
  {
    return 0;
  }
  std::set<int> shownLabels = this->Internal->GetShownLabels();
  int extent[6];
  labelmap->GetExtent(extent);
  int radius = static_cast<int>(std::ceil(maximumDistance));
  int nearestLabel = 0;
  double nearestDistance2 = maximumDistance * maximumDistance;
  int center[3] = { static_cast<int>(std::floor(ijk[0] + 0.5)), static_cast<int>(std::floor(ijk[1] + 0.5)), static_cast<int>(std::floor(ijk[2] + 0.5)) };
  for (int k = center[2] - radius; k <= center[2] + radius; ++k)
  {
    for (int j = center[1] - radius; j <= center[1] + radius; ++j)
    {
      for (int i = center[0] - radius; i <= center[0] + radius; ++i)
      {
        if (i < extent[0] || i > extent[1] || j < extent[2] || j > extent[3] || k < extent[4] || k > extent[5])
        {
          continue;
        }
        double distance2 = (i - ijk[0]) * (i - ijk[0]) + (j - ijk[1]) * (j - ijk[1]) + (k - ijk[2]) * (k - ijk[2]);
        if (distance2 > nearestDistance2)
        {
          continue;
        }
        int label = static_cast<int>(labelmap->GetScalarComponentAsDouble(i, j, k, 0));
        if (shownLabels.count(label))
        {
          nearestLabel = label;
          nearestDistance2 = distance2;
        }
      }
    }
  }
  return nearestLabel;
}

//----------------------------------------------------------------------------
void vtkSegmentationLabelmapSurfaceMapper::RemoveAllLabelColors()
{
  if (this->Internal->LabelColors.empty())
  {
    return;
  }
  this->Internal->LabelColors.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
double* vtkSegmentationLabelmapSurfaceMapper::GetBounds()
{
  this->Internal->PrepareLabelmap();
  std::copy(this->Internal->Bounds, this->Internal->Bounds + 6, this->Bounds);
  return this->Bounds;
}

//----------------------------------------------------------------------------
void vtkSegmentationLabelmapSurfaceMapper::Render(vtkRenderer* renderer, vtkActor* actor)
{
  vtkOpenGLRenderer* ren = vtkOpenGLRenderer::SafeDownCast(renderer);
  if (!ren || ren->GetSelector())
  {
    // hardware picking is not supported
    return;
  }
  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(ren->GetRenderWindow());
  if (!renWin)
  {
    return;
  }
  if (this->Internal->Context != renWin)
  {
    if (vtkOpenGLRenderWindow* previousContext = this->Internal->Context)
    {
      // The resources belong to the context of the previous window: it must be current while they are released
      // (WebGL refuses to delete objects of another context)
      previousContext->MakeCurrent();
      this->Internal->ReleaseGraphicsResources(previousContext);
      renWin->MakeCurrent();
    }
    this->Internal->Context = renWin;
    this->Internal->Failed = false;
  }
  if (this->Internal->Failed || !this->Internal->PrepareLabelmap())
  {
    return;
  }
  if (!this->Internal->UploadTextures(renWin))
  {
    this->Internal->Failed = true;
    return;
  }
  bool fullUpdate = this->Internal->DistanceFieldNeedsUpdate || this->Internal->ComputedSmoothingFactor != this->SmoothingFactor;
  if (fullUpdate || this->Internal->ChangedRegionValid)
  {
    if (!this->Internal->ComputeDistanceField(renWin, fullUpdate ? nullptr : this->Internal->ChangedRegion))
    {
      this->Internal->Failed = true;
      return;
    }
    this->Internal->DistanceFieldNeedsUpdate = false;
    this->Internal->ChangedRegionValid = false;
  }
  if (this->Internal->ColorsNeedUpdate && !this->Internal->ComputeColors(renWin, nullptr))
  {
    this->Internal->Failed = true;
    return;
  }
  this->Internal->RenderSurface(ren, actor);
}

//----------------------------------------------------------------------------
void vtkSegmentationLabelmapSurfaceMapper::ReleaseGraphicsResources(vtkWindow* window)
{
  this->Internal->ReleaseGraphicsResources(window);
  this->Superclass::ReleaseGraphicsResources(window);
}
