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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkErrorCode.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWindowToImageFilter.h>

// STD includes

#include "vtkMRMLCoreTestingMacros.h"

char vtkMRMLCameraDisplayableManagerTest1EventLog[] =
"# StreamVersion 1\n"
"EnterEvent 585 173 0 0 0 0 0\n"
"MouseMoveEvent 585 173 0 0 0 0 0\n"
"MouseMoveEvent 560 178 0 0 0 0 0\n"
"MouseMoveEvent 536 185 0 0 0 0 0\n"
"MouseMoveEvent 513 194 0 0 0 0 0\n"
"MouseMoveEvent 494 204 0 0 0 0 0\n"
"MouseMoveEvent 479 213 0 0 0 0 0\n"
"MouseMoveEvent 466 221 0 0 0 0 0\n"
"MouseMoveEvent 459 229 0 0 0 0 0\n"
"MouseMoveEvent 453 237 0 0 0 0 0\n"
"MouseMoveEvent 447 242 0 0 0 0 0\n"
"MouseMoveEvent 447 251 0 0 0 0 0\n"
"MouseMoveEvent 447 257 0 0 0 0 0\n"
"MouseMoveEvent 447 263 0 0 0 0 0\n"
"MouseMoveEvent 446 264 0 0 0 0 0\n"
"MouseMoveEvent 446 267 0 0 0 0 0\n"
"MouseMoveEvent 444 271 0 0 0 0 0\n"
"MouseMoveEvent 444 272 0 0 0 0 0\n"
"MouseMoveEvent 444 273 0 0 0 0 0\n"
"MouseMoveEvent 444 274 0 0 0 0 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelBackwardEvent 444 275 0 0 0 0 0\n"
"StartInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"EndInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelBackwardEvent 444 275 0 0 0 1 0\n"
"StartInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"EndInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelBackwardEvent 444 275 0 0 0 0 0\n"
"StartInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"EndInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelBackwardEvent 444 275 0 0 0 1 0\n"
"StartInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"EndInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelBackwardEvent 444 275 0 0 0 0 0\n"
"StartInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"EndInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 1 0\n"
"StartInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"EndInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 0 0\n"
"StartInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"EndInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 1 0\n"
"StartInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"EndInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 0 0\n"
"StartInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"EndInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 1 0\n"
"StartInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"EndInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 0 0\n"
"StartInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"EndInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 1 0\n"
"StartInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"EndInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 0 0\n"
"StartInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"EndInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 1 0\n"
"StartInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"EndInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 0 0\n"
"StartInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"EndInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 0 0\n"
"StartInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"EndInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 1 0\n"
"StartInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"EndInteractionEvent 444 275 0 0 0 1 0\n"
"RenderEvent 444 275 0 0 0 1 0\n"
"MouseMoveEvent 444 275 0 0 0 0 0\n"
"MouseWheelForwardEvent 444 275 0 0 0 0 0\n"
"StartInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"EndInteractionEvent 444 275 0 0 0 0 0\n"
"RenderEvent 444 275 0 0 0 0 0\n"
"MouseMoveEvent 442 274 0 0 0 0 0\n"
"MouseMoveEvent 436 274 0 0 0 0 0\n"
"MouseMoveEvent 425 272 0 0 0 0 0\n"
"MouseMoveEvent 416 269 0 0 0 0 0\n"
"MouseMoveEvent 406 265 0 0 0 0 0\n"
"MouseMoveEvent 392 261 0 0 0 0 0\n"
"MouseMoveEvent 377 259 0 0 0 0 0\n"
"MouseMoveEvent 360 259 0 0 0 0 0\n"
"MouseMoveEvent 343 259 0 0 0 0 0\n"
"MouseMoveEvent 328 259 0 0 0 0 0\n"
"MouseMoveEvent 323 259 0 0 0 0 0\n"
"MouseMoveEvent 314 259 0 0 0 0 0\n"
"MouseMoveEvent 300 258 0 0 0 0 0\n"
"MouseMoveEvent 287 258 0 0 0 0 0\n"
"MouseMoveEvent 275 258 0 0 0 0 0\n"
"MouseMoveEvent 264 258 0 0 0 0 0\n"
"MouseMoveEvent 257 258 0 0 0 0 0\n"
"MouseMoveEvent 250 258 0 0 0 0 0\n"
"MouseMoveEvent 249 258 0 0 0 0 0\n"
"MouseMoveEvent 246 259 0 0 0 0 0\n"
"MouseMoveEvent 242 260 0 0 0 0 0\n"
"MouseMoveEvent 240 260 0 0 0 0 0\n"
"LeftButtonPressEvent 240 260 0 0 0 1 0\n"
"StartInteractionEvent 240 260 0 0 0 1 0\n"
"MouseMoveEvent 241 260 0 0 0 0 0\n"
"MouseMoveEvent 243 260 0 0 0 0 0\n"
"MouseMoveEvent 246 259 0 0 0 0 0\n"
"MouseMoveEvent 249 259 0 0 0 0 0\n"
"MouseMoveEvent 252 259 0 0 0 0 0\n"
"MouseMoveEvent 254 258 0 0 0 0 0\n"
"MouseMoveEvent 257 258 0 0 0 0 0\n"
"MouseMoveEvent 259 258 0 0 0 0 0\n"
"MouseMoveEvent 263 258 0 0 0 0 0\n"
"MouseMoveEvent 266 257 0 0 0 0 0\n"
"MouseMoveEvent 270 256 0 0 0 0 0\n"
"MouseMoveEvent 273 254 0 0 0 0 0\n"
"MouseMoveEvent 277 252 0 0 0 0 0\n"
"MouseMoveEvent 281 251 0 0 0 0 0\n"
"MouseMoveEvent 285 250 0 0 0 0 0\n"
"MouseMoveEvent 291 248 0 0 0 0 0\n"
"MouseMoveEvent 297 244 0 0 0 0 0\n"
"MouseMoveEvent 303 240 0 0 0 0 0\n"
"MouseMoveEvent 310 237 0 0 0 0 0\n"
"MouseMoveEvent 314 234 0 0 0 0 0\n"
"MouseMoveEvent 319 230 0 0 0 0 0\n"
"MouseMoveEvent 326 226 0 0 0 0 0\n"
"MouseMoveEvent 333 222 0 0 0 0 0\n"
"MouseMoveEvent 340 218 0 0 0 0 0\n"
"MouseMoveEvent 346 212 0 0 0 0 0\n"
"MouseMoveEvent 351 211 0 0 0 0 0\n"
"MouseMoveEvent 356 208 0 0 0 0 0\n"
"MouseMoveEvent 362 205 0 0 0 0 0\n"
"MouseMoveEvent 366 201 0 0 0 0 0\n"
"MouseMoveEvent 369 200 0 0 0 0 0\n"
"MouseMoveEvent 373 197 0 0 0 0 0\n"
"MouseMoveEvent 376 196 0 0 0 0 0\n"
"MouseMoveEvent 379 193 0 0 0 0 0\n"
"MouseMoveEvent 381 192 0 0 0 0 0\n"
"MouseMoveEvent 384 190 0 0 0 0 0\n"
"MouseMoveEvent 385 188 0 0 0 0 0\n"
"MouseMoveEvent 388 186 0 0 0 0 0\n"
"MouseMoveEvent 390 184 0 0 0 0 0\n"
"MouseMoveEvent 392 183 0 0 0 0 0\n"
"MouseMoveEvent 393 182 0 0 0 0 0\n"
"MouseMoveEvent 393 181 0 0 0 0 0\n"
"MouseMoveEvent 394 181 0 0 0 0 0\n"
"LeftButtonReleaseEvent 394 181 0 0 0 0 0\n"
"EndInteractionEvent 394 181 0 0 0 0 0\n"
"RenderEvent 394 181 0 0 0 0 0\n"
"MouseMoveEvent 393 181 0 0 0 0 0\n"
"MouseMoveEvent 392 181 0 0 0 0 0\n"
"MouseMoveEvent 391 181 0 0 0 0 0\n"
"MouseMoveEvent 390 181 0 0 0 0 0\n"
"MouseMoveEvent 389 181 0 0 0 0 0\n"
"MouseMoveEvent 388 181 0 0 0 0 0\n"
"MouseMoveEvent 387 181 0 0 0 0 0\n"
"MouseMoveEvent 386 181 0 0 0 0 0\n"
"MouseMoveEvent 384 181 0 0 0 0 0\n"
"MouseMoveEvent 382 182 0 0 0 0 0\n"
"MouseMoveEvent 379 182 0 0 0 0 0\n"
"MouseMoveEvent 376 183 0 0 0 0 0\n"
"MouseMoveEvent 374 184 0 0 0 0 0\n"
"MouseMoveEvent 371 185 0 0 0 0 0\n"
"MouseMoveEvent 368 186 0 0 0 0 0\n"
"MouseMoveEvent 365 188 0 0 0 0 0\n"
"MouseMoveEvent 362 189 0 0 0 0 0\n"
"MouseMoveEvent 359 190 0 0 0 0 0\n"
"MouseMoveEvent 357 192 0 0 0 0 0\n"
"MouseMoveEvent 353 194 0 0 0 0 0\n"
"MouseMoveEvent 352 194 0 0 0 0 0\n"
"MouseMoveEvent 349 195 0 0 0 0 0\n"
"MouseMoveEvent 345 197 0 0 0 0 0\n"
"MouseMoveEvent 341 198 0 0 0 0 0\n"
"MouseMoveEvent 339 199 0 0 0 0 0\n"
"MouseMoveEvent 339 200 0 0 0 0 0\n"
"MouseWheelForwardEvent 339 200 0 0 0 0 0\n"
"StartInteractionEvent 339 200 0 0 0 0 0\n"
"RenderEvent 339 200 0 0 0 0 0\n"
"EndInteractionEvent 339 200 0 0 0 0 0\n"
"RenderEvent 339 200 0 0 0 0 0\n"
"MouseMoveEvent 339 200 0 0 0 0 0\n"
"MouseWheelForwardEvent 339 200 0 0 0 1 0\n"
"StartInteractionEvent 339 200 0 0 0 1 0\n"
"RenderEvent 339 200 0 0 0 1 0\n"
"EndInteractionEvent 339 200 0 0 0 1 0\n"
"RenderEvent 339 200 0 0 0 1 0\n"
"MouseMoveEvent 339 200 0 0 0 0 0\n"
"MouseWheelForwardEvent 339 200 0 0 0 0 0\n"
"StartInteractionEvent 339 200 0 0 0 0 0\n"
"RenderEvent 339 200 0 0 0 0 0\n"
"EndInteractionEvent 339 200 0 0 0 0 0\n"
"RenderEvent 339 200 0 0 0 0 0\n"
"MouseMoveEvent 339 200 0 0 0 0 0\n"
"MouseWheelForwardEvent 339 200 0 0 0 1 0\n"
"StartInteractionEvent 339 200 0 0 0 1 0\n"
"RenderEvent 339 200 0 0 0 1 0\n"
"EndInteractionEvent 339 200 0 0 0 1 0\n"
"RenderEvent 339 200 0 0 0 1 0\n"
"MouseMoveEvent 339 200 0 0 0 0 0\n"
"MouseWheelForwardEvent 339 200 0 0 0 0 0\n"
"StartInteractionEvent 339 200 0 0 0 0 0\n"
"RenderEvent 339 200 0 0 0 0 0\n"
"EndInteractionEvent 339 200 0 0 0 0 0\n"
"RenderEvent 339 200 0 0 0 0 0\n"
"MouseMoveEvent 338 200 0 0 0 0 0\n"
"MouseMoveEvent 336 200 0 0 0 0 0\n"
"MouseMoveEvent 333 200 0 0 0 0 0\n"
"MouseMoveEvent 331 200 0 0 0 0 0\n"
"MouseMoveEvent 328 200 0 0 0 0 0\n"
"MouseMoveEvent 327 200 0 0 0 0 0\n"
"MouseMoveEvent 326 200 0 0 0 0 0\n"
"MouseMoveEvent 325 200 0 0 0 0 0\n"
"MouseMoveEvent 323 200 0 0 0 0 0\n"
"MouseWheelBackwardEvent 323 200 0 0 0 1 0\n"
"StartInteractionEvent 323 200 0 0 0 1 0\n"
"RenderEvent 323 200 0 0 0 1 0\n"
"EndInteractionEvent 323 200 0 0 0 1 0\n"
"RenderEvent 323 200 0 0 0 1 0\n"
"MouseMoveEvent 322 200 0 0 0 0 0\n"
"MouseMoveEvent 321 200 0 0 0 0 0\n"
"MouseMoveEvent 320 200 0 0 0 0 0\n"
"MouseWheelBackwardEvent 320 200 0 0 0 0 0\n"
"StartInteractionEvent 320 200 0 0 0 0 0\n"
"RenderEvent 320 200 0 0 0 0 0\n"
"EndInteractionEvent 320 200 0 0 0 0 0\n"
"RenderEvent 320 200 0 0 0 0 0\n"
"MouseMoveEvent 320 201 0 0 0 0 0\n"
"MouseMoveEvent 320 203 0 0 0 0 0\n"
"MouseWheelBackwardEvent 320 203 0 0 0 1 0\n"
"StartInteractionEvent 320 203 0 0 0 1 0\n"
"RenderEvent 320 203 0 0 0 1 0\n"
"EndInteractionEvent 320 203 0 0 0 1 0\n"
"RenderEvent 320 203 0 0 0 1 0\n"
"MouseMoveEvent 319 204 0 0 0 0 0\n"
"MouseMoveEvent 318 206 0 0 0 0 0\n"
"MouseWheelBackwardEvent 318 206 0 0 0 0 0\n"
"StartInteractionEvent 318 206 0 0 0 0 0\n"
"RenderEvent 318 206 0 0 0 0 0\n"
"EndInteractionEvent 318 206 0 0 0 0 0\n"
"RenderEvent 318 206 0 0 0 0 0\n"
"MouseMoveEvent 318 208 0 0 0 0 0\n"
"MouseMoveEvent 317 211 0 0 0 0 0\n"
"MouseWheelBackwardEvent 317 211 0 0 0 1 0\n"
"StartInteractionEvent 317 211 0 0 0 1 0\n"
"RenderEvent 317 211 0 0 0 1 0\n"
"EndInteractionEvent 317 211 0 0 0 1 0\n"
"RenderEvent 317 211 0 0 0 1 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseWheelBackwardEvent 317 214 0 0 0 0 0\n"
"StartInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"EndInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseWheelBackwardEvent 317 214 0 0 0 1 0\n"
"StartInteractionEvent 317 214 0 0 0 1 0\n"
"RenderEvent 317 214 0 0 0 1 0\n"
"EndInteractionEvent 317 214 0 0 0 1 0\n"
"RenderEvent 317 214 0 0 0 1 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseWheelBackwardEvent 317 214 0 0 0 0 0\n"
"StartInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"EndInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseWheelBackwardEvent 317 214 0 0 0 1 0\n"
"StartInteractionEvent 317 214 0 0 0 1 0\n"
"RenderEvent 317 214 0 0 0 1 0\n"
"EndInteractionEvent 317 214 0 0 0 1 0\n"
"RenderEvent 317 214 0 0 0 1 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseWheelBackwardEvent 317 214 0 0 0 0 0\n"
"StartInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"EndInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseWheelBackwardEvent 317 214 0 0 0 1 0\n"
"StartInteractionEvent 317 214 0 0 0 1 0\n"
"RenderEvent 317 214 0 0 0 1 0\n"
"EndInteractionEvent 317 214 0 0 0 1 0\n"
"RenderEvent 317 214 0 0 0 1 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseWheelBackwardEvent 317 214 0 0 0 0 0\n"
"StartInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"EndInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseWheelBackwardEvent 317 214 0 0 0 1 0\n"
"StartInteractionEvent 317 214 0 0 0 1 0\n"
"RenderEvent 317 214 0 0 0 1 0\n"
"EndInteractionEvent 317 214 0 0 0 1 0\n"
"RenderEvent 317 214 0 0 0 1 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseWheelBackwardEvent 317 214 0 0 0 0 0\n"
"StartInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"EndInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseWheelBackwardEvent 317 214 0 0 0 1 0\n"
"StartInteractionEvent 317 214 0 0 0 1 0\n"
"RenderEvent 317 214 0 0 0 1 0\n"
"EndInteractionEvent 317 214 0 0 0 1 0\n"
"RenderEvent 317 214 0 0 0 1 0\n"
"MouseMoveEvent 317 214 0 0 0 0 0\n"
"MouseWheelForwardEvent 317 214 0 0 0 0 0\n"
"StartInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"EndInteractionEvent 317 214 0 0 0 0 0\n"
"RenderEvent 317 214 0 0 0 0 0\n"
"MouseMoveEvent 317 216 0 0 0 0 0\n"
"MouseMoveEvent 318 221 0 0 0 0 0\n"
"MouseMoveEvent 323 231 0 0 0 0 0\n"
"MouseMoveEvent 328 241 0 0 0 0 0\n"
"MouseMoveEvent 333 253 0 0 0 0 0\n"
"MouseMoveEvent 338 266 0 0 0 0 0\n"
"MouseMoveEvent 343 280 0 0 0 0 0\n"
"MouseMoveEvent 348 294 0 0 0 0 0\n"
"MouseMoveEvent 353 309 0 0 0 0 0\n"
"MouseMoveEvent 358 324 0 0 0 0 0\n"
"MouseMoveEvent 367 339 0 0 0 0 0\n"
"MouseMoveEvent 377 354 0 0 0 0 0\n"
"MouseMoveEvent 386 367 0 0 0 0 0\n"
"MouseMoveEvent 394 378 0 0 0 0 0\n"
"MouseMoveEvent 402 389 0 0 0 0 0\n"
"MouseMoveEvent 410 398 0 0 0 0 0\n"
"MouseMoveEvent 421 409 0 0 0 0 0\n"
"MouseMoveEvent 432 418 0 0 0 0 0\n"
"MouseMoveEvent 442 426 0 0 0 0 0\n"
"MouseMoveEvent 451 433 0 0 0 0 0\n"
"MouseMoveEvent 459 438 0 0 0 0 0\n"
"MouseMoveEvent 464 442 0 0 0 0 0\n"
"MouseMoveEvent 465 443 0 0 0 0 0\n"
"MouseMoveEvent 466 444 0 0 0 0 0\n"
"MouseMoveEvent 468 444 0 0 0 0 0\n"
"MouseMoveEvent 473 442 0 0 0 0 0\n"
"MouseMoveEvent 485 439 0 0 0 0 0\n"
"MouseMoveEvent 504 434 0 0 0 0 0\n"
"MouseMoveEvent 525 427 0 0 0 0 0\n"
"MouseMoveEvent 548 420 0 0 0 0 0\n"
"MouseMoveEvent 579 411 0 0 0 0 0\n"
"LeaveEvent 616 400 0 0 0 0 0\n";

namespace
{
//----------------------------------------------------------------------------
/// Read \a filename into string \a output
/// Note also that End-of-line will be removed
bool readFileIntoString(const char* filename, std::string& output)
{
  std::ifstream istream(filename);
  if( !istream )
    {
    cerr << "Could not open input file:" << filename << endl;
    return false;
    }

  std::string line;
  while(std::getline(istream,line))
    {
    output+=line;
    }

  return true;
}
}

//----------------------------------------------------------------------------
class vtkRenderRequestCallback : public vtkCommand
{
public:
  static vtkRenderRequestCallback *New()
    { return new vtkRenderRequestCallback; }
  void SetRenderer(vtkRenderer *renderer)
    { this->Renderer =  renderer; }
  int GetRenderRequestCount()
    { return this->RenderRequestCount; }
  void Execute(vtkObject*, unsigned long , void* ) override
    {
    this->Renderer->GetRenderWindow()->Render();
    this->RenderRequestCount++;
    //std::cout << "RenderRequestCount [" << this->RenderRequestCount << "]" << std::endl;
    }
protected:
  vtkRenderRequestCallback() = default;
  vtkRenderer * Renderer{nullptr};
  int           RenderRequestCount{0};
};

//----------------------------------------------------------------------------
int vtkMRMLCameraDisplayableManagerTest1(int argc, char* argv[])
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> rr;
  vtkNew<vtkRenderWindow> rw;
  vtkNew<vtkRenderWindowInteractor> ri;
  rw->SetSize(600, 600);

  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere

  rw->AddRenderer(rr.GetPointer());
  rw->SetInteractor(ri.GetPointer());

  // Set Interactor Style
  vtkNew<vtkMRMLThreeDViewInteractorStyle> iStyle;
  ri->SetInteractorStyle(iStyle.GetPointer());

  // MRML scene
  vtkNew<vtkMRMLScene> scene;

  // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene.GetPointer());

  // Add ViewNode
  vtkNew<vtkMRMLViewNode> viewNode;
  vtkMRMLNode * nodeAdded = scene->AddNode(viewNode.GetPointer());
  if (!nodeAdded)
    {
    std::cerr << "Failed to add vtkMRMLViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  // Factory
  vtkNew<vtkMRMLThreeDViewDisplayableManagerFactory> factory;

  // Check if GetRegisteredDisplayableManagerCount returns 0
  if (factory->GetRegisteredDisplayableManagerCount() != 0)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 0" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  factory->RegisterDisplayableManager("vtkMRMLCameraDisplayableManager");
  factory->RegisterDisplayableManager("vtkMRMLViewDisplayableManager");

  // Check if GetRegisteredDisplayableManagerCount returns 2
  if (factory->GetRegisteredDisplayableManagerCount() != 2)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 2" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  /*// Check if GetDisplayableManagerByClassName works as expected
  vtkMRMLCameraDisplayableManager * cameraDM2 =
      vtkMRMLCameraDisplayableManager::SafeDownCast(
          factory->GetDisplayableManagerByClassName("vtkMRMLCameraDisplayableManager"));
  if (cameraDM2 != cameraDM)
    {
    std::cerr << "Failed to retrieve vtkMRMLCameraDisplayableManager using "
        << "GetDisplayableManagerByClassName" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLViewDisplayableManager * viewDM2 =
      vtkMRMLViewDisplayableManager::SafeDownCast(
          factory->GetDisplayableManagerByClassName("vtkMRMLViewDisplayableManager"));
  if (viewDM2 != viewDM)
    {
    std::cerr << "Failed to retrieve vtkMRMLViewDisplayableManager using "
        << "GetDisplayableManagerByClassName" << std::endl;
    return EXIT_FAILURE;
    }
    */

  vtkSmartPointer<vtkMRMLDisplayableManagerGroup> displayableManagerGroup =
      vtkSmartPointer<vtkMRMLDisplayableManagerGroup>::Take(factory->InstantiateDisplayableManagers(rr.GetPointer()));

  if (!displayableManagerGroup)
    {
    std::cerr << "Failed to instantiate Displayable Managers using "
        << "InstantiateDisplayableManagers" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if GetDisplayableManagerCount returns 2
  if (displayableManagerGroup->GetDisplayableManagerCount() != 2)
    {
    std::cerr << "Check displayableManagerGroup->GetDisplayableManagerCount()" << std::endl;
    std::cerr << "Expected DisplayableManagerCount: 2" << std::endl;
    std::cerr << "Current DisplayableManagerCount:"
      << displayableManagerGroup->GetDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  // RenderRequest Callback
  vtkNew<vtkRenderRequestCallback> renderRequestCallback;
  renderRequestCallback->SetRenderer(rr.GetPointer());
  displayableManagerGroup->AddObserver(vtkCommand::UpdateEvent, renderRequestCallback.GetPointer());

  // Assign ViewNode
  displayableManagerGroup->SetMRMLDisplayableNode(viewNode.GetPointer());

  // Check if RenderWindowInteractor has NOT been changed
  if (displayableManagerGroup->GetInteractor() != ri.GetPointer())
    {
    std::cerr << "Expected RenderWindowInteractor:" << ri.GetPointer() << std::endl;
    std::cerr << "Current RenderWindowInteractor:"
        << displayableManagerGroup->GetInteractor() << std::endl;
    return EXIT_FAILURE;
    }

  // Interactor style should be vtkMRMLThreeDViewInteractorStyle
  vtkInteractorObserver * currentInteractoryStyle = ri->GetInteractorStyle();
  if (!vtkMRMLThreeDViewInteractorStyle::SafeDownCast(currentInteractoryStyle))
    {
    std::cerr << "Expected interactorStyle: vtkMRMLThreeDViewInteractorStyle" << std::endl;
    std::cerr << "Current RenderWindowInteractor: "
      << (currentInteractoryStyle ? currentInteractoryStyle->GetClassName() : "Null") << std::endl;
    return EXIT_FAILURE;
    }

  // Save current scene
  vtkNew<vtkTesting> testHelper;
  testHelper->AddArguments(argc, const_cast<const char **>(argv));
  vtkStdString savedScene = testHelper->GetTempDirectory();
  savedScene += "/vtkMRMLCameraDisplayableManagerTest1_saved.mrml";
  scene->Commit(savedScene);
  if (scene->GetErrorCode() != vtkErrorCode::NoError)
    {
    std::cerr << "Failed to save current scene into: " << savedScene << std::endl;
    return EXIT_FAILURE;
    }

  // Compare saved scene with baseline
  vtkStdString baselineScene = testHelper->GetDataRoot();
  baselineScene += "/Data/vtkMRMLCameraDisplayableManagerTest1.mrml";

  // Read baseline scene into string
  std::string baselineSceneAsString;
  bool ret = readFileIntoString(baselineScene.c_str(), baselineSceneAsString);
  if (!ret || baselineSceneAsString.size() == 0)
    {
    std::cerr << "Failed to read baseline scene into string: " << baselineScene << std::endl;
    return EXIT_FAILURE;
    }

  std::string savedSceneAsString;
  ret = readFileIntoString(savedScene.c_str(), savedSceneAsString);
  if (!ret || savedSceneAsString.size() == 0)
    {
    std::cerr << "Failed to read saved scene into string: " << savedScene << std::endl;
    return EXIT_FAILURE;
    }

  if (savedSceneAsString != baselineSceneAsString)
    {
    std::cerr << "Baseline scene and Saved scene are different !\n"
              << "Baseline scene\n"
              << "[BASELINE]\n" << baselineSceneAsString << "\n[/BASELINE]\n"
              << "[SAVED]\n" << savedSceneAsString << "\n[/SAVED]"<< std::endl;
    return EXIT_FAILURE;
    }

  // Import baseline scene
  scene->SetURL(baselineScene);
  if (scene->GetURL() != baselineScene)
    {
    std::cerr << "Failed to set URL: " << baselineScene << std::endl;
    return EXIT_FAILURE;
    }
  scene->Connect();
  if (scene->GetErrorCode() != vtkErrorCode::NoError)
    {
    std::cerr << "Failed to import baseline scene: " << baselineScene << std::endl;
    return EXIT_FAILURE;
    }

  // Event recorder
  bool disableReplay = false, record = false, screenshot = false;
  for (int i = 0; i < argc; i++)
    {
    disableReplay |= (strcmp("--DisableReplay", argv[i]) == 0);
    record        |= (strcmp("--Record", argv[i]) == 0);
    screenshot    |= (strcmp("--Screenshot", argv[i]) == 0);
    }
  vtkNew<vtkInteractorEventRecorder> recorder;
  recorder->SetInteractor(displayableManagerGroup->GetInteractor());
  if (!disableReplay)
    {
    if (record)
      {
      std::cout << "Recording ..." << std::endl;
      recorder->SetFileName("vtkInteractorEventRecorder.log");
      recorder->On();
      recorder->Record();
      }
    else
      {
      // Play
      recorder->ReadFromInputStringOn();
      recorder->SetInputString(vtkMRMLCameraDisplayableManagerTest1EventLog);
      recorder->Play();
      }
    }
  recorder->SetInteractor(nullptr);

  int retval = vtkRegressionTestImageThreshold(rw.GetPointer(), 85.0);
  if ( record || retval == vtkRegressionTester::DO_INTERACTOR)
    {
    displayableManagerGroup->GetInteractor()->Initialize();
    displayableManagerGroup->GetInteractor()->Start();
    std::cout << "Current RenderRequestCount: "
        << renderRequestCallback->GetRenderRequestCount() << std::endl;
    }

  if (record || screenshot)
    {
    vtkNew<vtkWindowToImageFilter> windowToImageFilter;
    windowToImageFilter->SetInput(rw.GetPointer());
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 1)
    windowToImageFilter->SetScale(1, 1); //set the resolution of the output image
#else
    windowToImageFilter->SetMagnification(1); //set the resolution of the output image
#endif
    windowToImageFilter->Update();

    vtkStdString screenshootFilename = testHelper->GetDataRoot();
    screenshootFilename += "/Baseline/vtkMRMLCameraDisplayableManagerTest1.png";
    vtkNew<vtkPNGWriter> writer;
    writer->SetFileName(screenshootFilename.c_str());
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();
    std::cout << "Saved screenshot: " << screenshootFilename << std::endl;
    }

  return !retval;
}

