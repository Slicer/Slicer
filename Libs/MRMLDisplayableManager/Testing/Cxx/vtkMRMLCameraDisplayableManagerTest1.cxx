/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerFactory.h"
#include "vtkMRMLViewDisplayableManager.h"
#include "vtkMRMLCameraDisplayableManager.h"
#include "vtkDisplayableManagerInteractorStyle.h"

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h> 
#include <vtkSmartPointer.h>
#include <vtkErrorCode.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

// STD includes
#include <cstdlib>
#include <iostream>
#include <iterator>

#include "TestingMacros.h"

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

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
    cerr << "Couldn´t open input file:" << filename << endl;
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
  virtual void Execute(vtkObject*, unsigned long , void* )
    {
    this->Renderer->GetRenderWindow()->Render();
    this->RenderRequestCount++;
    //std::cout << "RenderRequestCount [" << this->RenderRequestCount << "]" << std::endl;
    }
protected:
  vtkRenderRequestCallback():Renderer(0), RenderRequestCount(0){}
  vtkRenderer * Renderer;
  int           RenderRequestCount;
};

//----------------------------------------------------------------------------
int vtkMRMLCameraDisplayableManagerTest1(int argc, char* argv[])
{
  // Renderer, RenderWindow and Interactor
  vtkRenderer* rr = vtkRenderer::New();
  vtkRenderWindow* rw = vtkRenderWindow::New();
  vtkRenderWindowInteractor* ri = vtkRenderWindowInteractor::New();
  rw->SetSize(600, 600);

  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere

  rw->AddRenderer(rr);
  rw->SetInteractor(ri);

  // Set Interactor Style
  vtkDisplayableManagerInteractorStyle * iStyle = vtkDisplayableManagerInteractorStyle::New();
  ri->SetInteractorStyle(iStyle);
  iStyle->Delete();
  //iStyle->DebugOn();

  // MRML scene
  vtkMRMLScene* scene = vtkMRMLScene::New();

  // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
  vtkMRMLApplicationLogic* applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);
  // Pass through event handling once without observing the scene
  // allows any dependent nodes to be created
  applicationLogic->ProcessMRMLEvents(scene, vtkCommand::ModifiedEvent, 0);
  applicationLogic->SetAndObserveMRMLScene(scene);

  // Add ViewNode
  vtkMRMLViewNode * viewNode = vtkMRMLViewNode::New();
  vtkMRMLNode * nodeAdded = scene->AddNode(viewNode);
  viewNode->Delete();
  if (!nodeAdded)
    {
    std::cerr << "Failed to add vtkMRMLViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  // Factory
  vtkMRMLDisplayableManagerFactory* factory = vtkMRMLDisplayableManagerFactory::New();
  factory->Initialize(rr);

  // RenderRequest Callback
  vtkRenderRequestCallback * renderRequestCallback = vtkRenderRequestCallback::New();
  renderRequestCallback->SetRenderer(rr);
  factory->AddObserver(vtkCommand::UpdateEvent, renderRequestCallback);

  // Register Displayable Managers
  vtkMRMLCameraDisplayableManager * cameraNodeDM = vtkMRMLCameraDisplayableManager::New();
  factory->RegisterDisplayableManager(cameraNodeDM);
  cameraNodeDM->Delete();
  //cameraNodeDM->DebugOn();

  vtkMRMLViewDisplayableManager * viewNodeDM = vtkMRMLViewDisplayableManager::New();
  factory->RegisterDisplayableManager(viewNodeDM);
  viewNodeDM->Delete();

  // Check if GetDisplayableManagerByClassName works as expected
  vtkMRMLCameraDisplayableManager * cameraNodeDM2 =
      vtkMRMLCameraDisplayableManager::SafeDownCast(
          factory->GetDisplayableManagerByClassName("vtkMRMLCameraDisplayableManager"));
  if (cameraNodeDM2 != cameraNodeDM)
    {
    std::cerr << "Failed to retrieve vtkMRMLCameraDisplayableManager using "
        << "GetDisplayableManagerByClassName" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLViewDisplayableManager * viewNodeDM2 =
      vtkMRMLViewDisplayableManager::SafeDownCast(
          factory->GetDisplayableManagerByClassName("vtkMRMLViewDisplayableManager"));
  if (viewNodeDM2 != viewNodeDM)
    {
    std::cerr << "Failed to retrieve vtkMRMLViewDisplayableManager using "
        << "GetDisplayableManagerByClassName" << std::endl;
    return EXIT_FAILURE;
    }

  // Assign ViewNode
  factory->SetMRMLViewNode(viewNode);

  // Check if RenderWindowInteractor has NOT been changed
  if (factory->GetInteractor() != ri)
    {
    std::cerr << "Expected RenderWindowInteractor:" << ri << std::endl;
    std::cerr << "Current RenderWindowInteractor:" << factory->GetInteractor() << std::endl;
    return EXIT_FAILURE;
    }

  // Interactor style should be vtkDisplayableManagerInteractorStyle
  vtkInteractorObserver * currentInteractoryStyle = ri->GetInteractorStyle();
  if (!vtkDisplayableManagerInteractorStyle::SafeDownCast(currentInteractoryStyle))
    {
    std::cerr << "Expected interactorStyle: vtkDisplayableManagerInteractorStyle" << std::endl;
    std::cerr << "Current RenderWindowInteractor: "
      << (currentInteractoryStyle ? currentInteractoryStyle->GetClassName() : "Null") << std::endl;
    return EXIT_FAILURE;
    }

  // Save current scene
  VTK_CREATE(vtkTesting, testHelper);
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
  scene->Import();
  if (scene->GetErrorCode() != vtkErrorCode::NoError)
    {
    std::cerr << "Failed to import baseline scene: " << baselineScene << std::endl;
    return EXIT_FAILURE;
    }

  // Since there is no interaction, exactly "1" RenderRequest is expected
  int renderRequestCount = renderRequestCallback->GetRenderRequestCount();
  if (renderRequestCount != 1)
    {
    std::cerr << "Expected RenderRequestCount: 1" << std::endl
              << "Current RenderRequestCount: " <<  renderRequestCount << std::endl;
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
  vtkInteractorEventRecorder * recorder = vtkInteractorEventRecorder::New();
  recorder->SetInteractor(factory->GetInteractor());
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

  int retval = vtkRegressionTestImageThreshold(rw, 80.0);
  if ( record || retval == vtkRegressionTester::DO_INTERACTOR)
    {
    factory->GetInteractor()->Initialize();
    factory->GetInteractor()->Start();
    }

  if (record || screenshot)
    {
    VTK_CREATE(vtkWindowToImageFilter, windowToImageFilter) ;
    windowToImageFilter->SetInput(rw);
    windowToImageFilter->SetMagnification(1); //set the resolution of the output image
    windowToImageFilter->Update();

    vtkStdString screenshootFilename = testHelper->GetDataRoot();
    screenshootFilename += "/Baseline/vtkMRMLCameraDisplayableManagerTest1.png";
    VTK_CREATE(vtkPNGWriter, writer);
    writer->SetFileName(screenshootFilename.c_str());
    writer->SetInput(windowToImageFilter->GetOutput());
    writer->Write();
    std::cout << "Saved screenshot: " << screenshootFilename << std::endl;
    }

  recorder->Delete();
  renderRequestCallback->Delete();
  factory->Delete();
  applicationLogic->Delete();
  scene->Delete();
  rr->Delete();
  rw->Delete();
  ri->Delete();

  return !retval;
}

