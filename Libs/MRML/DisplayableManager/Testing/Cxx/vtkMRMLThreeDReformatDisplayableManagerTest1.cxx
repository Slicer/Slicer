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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include <vtkMRMLCameraDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLThreeDReformatDisplayableManager.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCamera.h>
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

char vtkMRMLThreeDReformatDisplayableManagerTest1EventLog[] =
  "# StreamVersion 1\n"
  "RenderEvent 0 0 0 0 0 0 0\n"
  "EnterEvent 592 493 0 0 0 0 0\n"
  "LeftButtonPressEvent 281 556 0 0 0 0 0\n"
  "StartInteractionEvent 281 556 0 0 0 0 0\n"
  "MouseMoveEvent 283 555 0 0 0 0 0\n"
  "MouseMoveEvent 292 550 0 0 0 0 0\n"
  "MouseMoveEvent 305 544 0 0 0 0 0\n"
  "MouseMoveEvent 312 539 0 0 0 0 0\n"
  "MouseMoveEvent 323 537 0 0 0 0 0\n"
  "MouseMoveEvent 336 534 0 0 0 0 0\n"
  "MouseMoveEvent 343 532 0 0 0 0 0\n"
  "MouseMoveEvent 348 530 0 0 0 0 0\n"
  "MouseMoveEvent 353 528 0 0 0 0 0\n"
  "MouseMoveEvent 357 526 0 0 0 0 0\n"
  "MouseMoveEvent 362 526 0 0 0 0 0\n"
  "MouseMoveEvent 368 524 0 0 0 0 0\n"
  "MouseMoveEvent 373 524 0 0 0 0 0\n"
  "MouseMoveEvent 376 524 0 0 0 0 0\n"
  "MouseMoveEvent 378 524 0 0 0 0 0\n"
  "MouseMoveEvent 380 524 0 0 0 0 0\n"
  "MouseMoveEvent 381 523 0 0 0 0 0\n"
  "MouseMoveEvent 382 523 0 0 0 0 0\n"
  "MouseMoveEvent 384 523 0 0 0 0 0\n"
  "MouseMoveEvent 386 523 0 0 0 0 0\n"
  "MouseMoveEvent 387 523 0 0 0 0 0\n"
  "MouseMoveEvent 388 522 0 0 0 0 0\n"
  "MouseMoveEvent 389 522 0 0 0 0 0\n"
  "MouseMoveEvent 390 522 0 0 0 0 0\n"
  "MouseMoveEvent 391 522 0 0 0 0 0\n"
  "MouseMoveEvent 393 522 0 0 0 0 0\n"
  "MouseMoveEvent 394 522 0 0 0 0 0\n"
  "MouseMoveEvent 394 521 0 0 0 0 0\n"
  "MouseMoveEvent 395 521 0 0 0 0 0\n"
  "MouseMoveEvent 396 521 0 0 0 0 0\n"
  "MouseMoveEvent 397 521 0 0 0 0 0\n"
  "MouseMoveEvent 398 521 0 0 0 0 0\n"
  "MouseMoveEvent 398 520 0 0 0 0 0\n"
  "MouseMoveEvent 400 520 0 0 0 0 0\n"
  "MouseMoveEvent 401 519 0 0 0 0 0\n"
  "MouseMoveEvent 402 517 0 0 0 0 0\n"
  "MouseMoveEvent 404 516 0 0 0 0 0\n"
  "MouseMoveEvent 405 515 0 0 0 0 0\n"
  "MouseMoveEvent 407 512 0 0 0 0 0\n"
  "MouseMoveEvent 408 511 0 0 0 0 0\n"
  "MouseMoveEvent 410 509 0 0 0 0 0\n"
  "MouseMoveEvent 411 508 0 0 0 0 0\n"
  "MouseMoveEvent 412 507 0 0 0 0 0\n"
  "MouseMoveEvent 413 507 0 0 0 0 0\n"
  "MouseMoveEvent 414 505 0 0 0 0 0\n"
  "MouseMoveEvent 415 504 0 0 0 0 0\n"
  "MouseMoveEvent 415 503 0 0 0 0 0\n"
  "MouseMoveEvent 416 501 0 0 0 0 0\n"
  "MouseMoveEvent 417 500 0 0 0 0 0\n"
  "MouseMoveEvent 419 498 0 0 0 0 0\n"
  "MouseMoveEvent 419 497 0 0 0 0 0\n"
  "MouseMoveEvent 420 496 0 0 0 0 0\n"
  "MouseMoveEvent 420 494 0 0 0 0 0\n"
  "MouseMoveEvent 421 494 0 0 0 0 0\n"
  "MouseMoveEvent 421 493 0 0 0 0 0\n"
  "MouseMoveEvent 421 492 0 0 0 0 0\n"
  "MouseMoveEvent 422 492 0 0 0 0 0\n"
  "MouseMoveEvent 422 491 0 0 0 0 0\n"
  "MouseMoveEvent 422 490 0 0 0 0 0\n"
  "MouseMoveEvent 422 489 0 0 0 0 0\n"
  "MouseMoveEvent 423 488 0 0 0 0 0\n"
  "MouseMoveEvent 425 488 0 0 0 0 0\n"
  "MouseMoveEvent 425 487 0 0 0 0 0\n"
  "MouseMoveEvent 426 486 0 0 0 0 0\n"
  "MouseMoveEvent 426 485 0 0 0 0 0\n"
  "MouseMoveEvent 426 484 0 0 0 0 0\n"
  "MouseMoveEvent 427 484 0 0 0 0 0\n"
  "MouseMoveEvent 427 483 0 0 0 0 0\n"
  "MouseMoveEvent 427 481 0 0 0 0 0\n"
  "MouseMoveEvent 428 480 0 0 0 0 0\n"
  "MouseMoveEvent 429 479 0 0 0 0 0\n"
  "MouseMoveEvent 429 478 0 0 0 0 0\n"
  "MouseMoveEvent 430 477 0 0 0 0 0\n"
  "MouseMoveEvent 431 476 0 0 0 0 0\n"
  "MouseMoveEvent 432 476 0 0 0 0 0\n"
  "MouseMoveEvent 432 475 0 0 0 0 0\n"
  "MouseMoveEvent 432 474 0 0 0 0 0\n"
  "MouseMoveEvent 434 472 0 0 0 0 0\n"
  "MouseMoveEvent 435 471 0 0 0 0 0\n"
  "MouseMoveEvent 435 469 0 0 0 0 0\n"
  "MouseMoveEvent 436 467 0 0 0 0 0\n"
  "MouseMoveEvent 437 466 0 0 0 0 0\n"
  "MouseMoveEvent 437 465 0 0 0 0 0\n"
  "MouseMoveEvent 437 464 0 0 0 0 0\n"
  "MouseMoveEvent 437 462 0 0 0 0 0\n"
  "MouseMoveEvent 437 461 0 0 0 0 0\n"
  "MouseMoveEvent 438 459 0 0 0 0 0\n"
  "MouseMoveEvent 438 457 0 0 0 0 0\n"
  "MouseMoveEvent 438 455 0 0 0 0 0\n"
  "MouseMoveEvent 439 453 0 0 0 0 0\n"
  "MouseMoveEvent 439 452 0 0 0 0 0\n"
  "MouseMoveEvent 440 451 0 0 0 0 0\n"
  "MouseMoveEvent 440 449 0 0 0 0 0\n"
  "MouseMoveEvent 440 445 0 0 0 0 0\n"
  "MouseMoveEvent 441 444 0 0 0 0 0\n"
  "MouseMoveEvent 441 441 0 0 0 0 0\n"
  "MouseMoveEvent 442 440 0 0 0 0 0\n"
  "MouseMoveEvent 442 439 0 0 0 0 0\n"
  "MouseMoveEvent 443 437 0 0 0 0 0\n"
  "MouseMoveEvent 444 435 0 0 0 0 0\n"
  "MouseMoveEvent 444 434 0 0 0 0 0\n"
  "MouseMoveEvent 445 432 0 0 0 0 0\n"
  "MouseMoveEvent 445 430 0 0 0 0 0\n"
  "MouseMoveEvent 446 428 0 0 0 0 0\n"
  "MouseMoveEvent 447 426 0 0 0 0 0\n"
  "MouseMoveEvent 447 424 0 0 0 0 0\n"
  "MouseMoveEvent 448 421 0 0 0 0 0\n"
  "MouseMoveEvent 449 419 0 0 0 0 0\n"
  "MouseMoveEvent 450 415 0 0 0 0 0\n"
  "MouseMoveEvent 450 412 0 0 0 0 0\n"
  "MouseMoveEvent 451 409 0 0 0 0 0\n"
  "MouseMoveEvent 452 407 0 0 0 0 0\n"
  "MouseMoveEvent 452 405 0 0 0 0 0\n"
  "MouseMoveEvent 453 403 0 0 0 0 0\n"
  "MouseMoveEvent 454 401 0 0 0 0 0\n"
  "MouseMoveEvent 454 399 0 0 0 0 0\n"
  "MouseMoveEvent 455 399 0 0 0 0 0\n"
  "MouseMoveEvent 455 398 0 0 0 0 0\n"
  "MouseMoveEvent 455 397 0 0 0 0 0\n"
  "MouseMoveEvent 455 396 0 0 0 0 0\n"
  "MouseMoveEvent 455 394 0 0 0 0 0\n"
  "MouseMoveEvent 455 393 0 0 0 0 0\n"
  "MouseMoveEvent 455 392 0 0 0 0 0\n"
  "MouseMoveEvent 455 391 0 0 0 0 0\n"
  "MouseMoveEvent 455 390 0 0 0 0 0\n"
  "LeftButtonReleaseEvent 455 390 0 0 0 0 0\n"
  "EndInteractionEvent 455 390 0 0 0 0 0\n"
  "RenderEvent 455 390 0 0 0 0 0\n"
  "MouseMoveEvent 455 389 0 0 0 0 0\n"
  "LeftButtonPressEvent 80 488 0 0 0 0 0\n"
  "StartInteractionEvent 80 488 0 0 0 0 0\n"
  "MouseMoveEvent 81 488 0 0 0 0 0\n"
  "MouseMoveEvent 82 487 0 0 0 0 0\n"
  "MouseMoveEvent 83 485 0 0 0 0 0\n"
  "MouseMoveEvent 84 482 0 0 0 0 0\n"
  "MouseMoveEvent 85 478 0 0 0 0 0\n"
  "MouseMoveEvent 87 473 0 0 0 0 0\n"
  "MouseMoveEvent 87 470 0 0 0 0 0\n"
  "MouseMoveEvent 89 468 0 0 0 0 0\n"
  "MouseMoveEvent 90 465 0 0 0 0 0\n"
  "MouseMoveEvent 92 463 0 0 0 0 0\n"
  "MouseMoveEvent 93 461 0 0 0 0 0\n"
  "MouseMoveEvent 94 459 0 0 0 0 0\n"
  "MouseMoveEvent 95 458 0 0 0 0 0\n"
  "MouseMoveEvent 96 456 0 0 0 0 0\n"
  "MouseMoveEvent 97 454 0 0 0 0 0\n"
  "MouseMoveEvent 97 452 0 0 0 0 0\n"
  "MouseMoveEvent 99 448 0 0 0 0 0\n"
  "MouseMoveEvent 100 446 0 0 0 0 0\n"
  "MouseMoveEvent 102 443 0 0 0 0 0\n"
  "MouseMoveEvent 102 439 0 0 0 0 0\n"
  "MouseMoveEvent 103 437 0 0 0 0 0\n"
  "MouseMoveEvent 104 435 0 0 0 0 0\n"
  "MouseMoveEvent 104 433 0 0 0 0 0\n"
  "MouseMoveEvent 105 431 0 0 0 0 0\n"
  "MouseMoveEvent 105 429 0 0 0 0 0\n"
  "MouseMoveEvent 105 427 0 0 0 0 0\n"
  "MouseMoveEvent 105 426 0 0 0 0 0\n"
  "MouseMoveEvent 105 424 0 0 0 0 0\n"
  "MouseMoveEvent 105 422 0 0 0 0 0\n"
  "MouseMoveEvent 105 420 0 0 0 0 0\n"
  "MouseMoveEvent 105 417 0 0 0 0 0\n"
  "MouseMoveEvent 105 415 0 0 0 0 0\n"
  "MouseMoveEvent 105 412 0 0 0 0 0\n"
  "MouseMoveEvent 106 409 0 0 0 0 0\n"
  "MouseMoveEvent 106 407 0 0 0 0 0\n"
  "MouseMoveEvent 106 405 0 0 0 0 0\n"
  "MouseMoveEvent 106 403 0 0 0 0 0\n"
  "MouseMoveEvent 106 401 0 0 0 0 0\n"
  "MouseMoveEvent 106 399 0 0 0 0 0\n"
  "MouseMoveEvent 106 398 0 0 0 0 0\n"
  "MouseMoveEvent 106 396 0 0 0 0 0\n"
  "MouseMoveEvent 106 394 0 0 0 0 0\n"
  "MouseMoveEvent 107 392 0 0 0 0 0\n"
  "MouseMoveEvent 107 390 0 0 0 0 0\n"
  "MouseMoveEvent 107 387 0 0 0 0 0\n"
  "MouseMoveEvent 107 384 0 0 0 0 0\n"
  "MouseMoveEvent 107 382 0 0 0 0 0\n"
  "MouseMoveEvent 107 379 0 0 0 0 0\n"
  "MouseMoveEvent 108 378 0 0 0 0 0\n"
  "MouseMoveEvent 108 376 0 0 0 0 0\n"
  "MouseMoveEvent 108 374 0 0 0 0 0\n"
  "MouseMoveEvent 108 372 0 0 0 0 0\n"
  "MouseMoveEvent 108 371 0 0 0 0 0\n"
  "MouseMoveEvent 108 370 0 0 0 0 0\n"
  "MouseMoveEvent 108 369 0 0 0 0 0\n"
  "MouseMoveEvent 108 368 0 0 0 0 0\n"
  "MouseMoveEvent 108 367 0 0 0 0 0\n"
  "MouseMoveEvent 108 365 0 0 0 0 0\n"
  "MouseMoveEvent 108 363 0 0 0 0 0\n"
  "MouseMoveEvent 108 361 0 0 0 0 0\n"
  "MouseMoveEvent 109 358 0 0 0 0 0\n"
  "MouseMoveEvent 109 354 0 0 0 0 0\n"
  "MouseMoveEvent 109 352 0 0 0 0 0\n"
  "MouseMoveEvent 109 346 0 0 0 0 0\n"
  "MouseMoveEvent 109 344 0 0 0 0 0\n"
  "MouseMoveEvent 109 343 0 0 0 0 0\n"
  "MouseMoveEvent 110 341 0 0 0 0 0\n"
  "MouseMoveEvent 110 340 0 0 0 0 0\n"
  "MouseMoveEvent 110 339 0 0 0 0 0\n"
  "MouseMoveEvent 110 338 0 0 0 0 0\n"
  "MouseMoveEvent 110 336 0 0 0 0 0\n"
  "MouseMoveEvent 110 334 0 0 0 0 0\n"
  "MouseMoveEvent 110 332 0 0 0 0 0\n"
  "MouseMoveEvent 110 328 0 0 0 0 0\n"
  "MouseMoveEvent 110 323 0 0 0 0 0\n"
  "MouseMoveEvent 110 320 0 0 0 0 0\n"
  "MouseMoveEvent 110 313 0 0 0 0 0\n"
  "MouseMoveEvent 110 309 0 0 0 0 0\n"
  "MouseMoveEvent 110 306 0 0 0 0 0\n"
  "MouseMoveEvent 110 302 0 0 0 0 0\n"
  "MouseMoveEvent 110 300 0 0 0 0 0\n"
  "MouseMoveEvent 110 298 0 0 0 0 0\n"
  "MouseMoveEvent 110 296 0 0 0 0 0\n"
  "MouseMoveEvent 111 294 0 0 0 0 0\n"
  "MouseMoveEvent 111 292 0 0 0 0 0\n"
  "MouseMoveEvent 111 290 0 0 0 0 0\n"
  "MouseMoveEvent 111 288 0 0 0 0 0\n"
  "MouseMoveEvent 112 285 0 0 0 0 0\n"
  "MouseMoveEvent 112 283 0 0 0 0 0\n"
  "MouseMoveEvent 113 278 0 0 0 0 0\n"
  "MouseMoveEvent 113 275 0 0 0 0 0\n"
  "MouseMoveEvent 114 273 0 0 0 0 0\n"
  "MouseMoveEvent 114 271 0 0 0 0 0\n"
  "MouseMoveEvent 115 269 0 0 0 0 0\n"
  "MouseMoveEvent 114 268 0 0 0 0 0\n"
  "MouseMoveEvent 115 267 0 0 0 0 0\n"
  "MouseMoveEvent 115 266 0 0 0 0 0\n"
  "MouseMoveEvent 115 264 0 0 0 0 0\n"
  "MouseMoveEvent 115 262 0 0 0 0 0\n"
  "MouseMoveEvent 116 262 0 0 0 0 0\n"
  "MouseMoveEvent 117 261 0 0 0 0 0\n"
  "MouseMoveEvent 117 259 0 0 0 0 0\n"
  "MouseMoveEvent 117 257 0 0 0 0 0\n"
  "LeftButtonReleaseEvent 117 257 0 0 0 0 0\n"
  "EndInteractionEvent 117 257 0 0 0 0 0\n"
  "RenderEvent 117 257 0 0 0 0 0\n"
  "MouseMoveEvent 184 322 0 0 0 0 0\n"
  "RenderEvent 188 328 0 0 0 0 0\n"
  "LeftButtonPressEvent 188 328 0 0 0 0 0\n"
  "RenderEvent 188 328 0 0 0 0 0\n"
  "MouseMoveEvent 189 330 0 0 0 0 0\n"
  "MouseMoveEvent 195 332 0 0 0 0 0\n"
  "MouseMoveEvent 197 334 0 0 0 0 0\n"
  "MouseMoveEvent 203 336 0 0 0 0 0\n"
  "MouseMoveEvent 220 339 0 0 0 0 0\n"
  "MouseMoveEvent 223 339 0 0 0 0 0\n"
  "MouseMoveEvent 229 339 0 0 0 0 0\n"
  "MouseMoveEvent 233 338 0 0 0 0 0\n"
  "MouseMoveEvent 234 337 0 0 0 0 0\n"
  "MouseMoveEvent 241 336 0 0 0 0 0\n"
  "MouseMoveEvent 248 332 0 0 0 0 0\n"
  "MouseMoveEvent 254 331 0 0 0 0 0\n"
  "MouseMoveEvent 260 329 0 0 0 0 0\n"
  "MouseMoveEvent 267 324 0 0 0 0 0\n"
  "MouseMoveEvent 275 321 0 0 0 0 0\n"
  "MouseMoveEvent 277 319 0 0 0 0 0\n"
  "MouseMoveEvent 282 318 0 0 0 0 0\n"
  "MouseMoveEvent 285 317 0 0 0 0 0\n"
  "MouseMoveEvent 287 315 0 0 0 0 0\n"
  "MouseMoveEvent 288 315 0 0 0 0 0\n"
  "MouseMoveEvent 289 314 0 0 0 0 0\n"
  "MouseMoveEvent 290 309 0 0 0 0 0\n"
  "MouseMoveEvent 306 292 0 0 0 0 0\n"
  "MouseMoveEvent 327 269 0 0 0 0 0\n"
  "MouseMoveEvent 335 265 0 0 0 0 0\n"
  "MouseMoveEvent 337 265 0 0 0 0 0\n"
  "MouseMoveEvent 338 265 0 0 0 0 0\n"
  "MouseMoveEvent 337 265 0 0 0 0 0\n"
  "RenderEvent 337 265 0 0 0 0 0\n"
  "LeftButtonReleaseEvent 337 265 0 0 0 0 0\n"
  "RenderEvent 337 265 0 0 0 0 0\n"
  "RenderEvent 302 298 0 0 0 0 0\n"
  "LeftButtonPressEvent 302 298 0 0 0 0 0\n"
  "RenderEvent 302 298 0 0 0 0 0\n"
  "MouseMoveEvent 303 298 0 0 0 0 0\n"
  "MouseMoveEvent 304 298 0 0 0 0 0\n"
  "MouseMoveEvent 305 298 0 0 0 0 0\n"
  "MouseMoveEvent 307 299 0 0 0 0 0\n"
  "MouseMoveEvent 309 299 0 0 0 0 0\n"
  "MouseMoveEvent 310 299 0 0 0 0 0\n"
  "MouseMoveEvent 311 298 0 0 0 0 0\n"
  "MouseMoveEvent 315 297 0 0 0 0 0\n"
  "MouseMoveEvent 317 294 0 0 0 0 0\n"
  "MouseMoveEvent 317 293 0 0 0 0 0\n"
  "MouseMoveEvent 319 288 0 0 0 0 0\n"
  "MouseMoveEvent 322 283 0 0 0 0 0\n"
  "MouseMoveEvent 323 279 0 0 0 0 0\n"
  "MouseMoveEvent 323 278 0 0 0 0 0\n"
  "MouseMoveEvent 323 274 0 0 0 0 0\n"
  "MouseMoveEvent 323 273 0 0 0 0 0\n"
  "MouseMoveEvent 323 272 0 0 0 0 0\n"
  "MouseMoveEvent 323 270 0 0 0 0 0\n"
  "MouseMoveEvent 323 266 0 0 0 0 0\n"
  "MouseMoveEvent 322 261 0 0 0 0 0\n"
  "MouseMoveEvent 323 258 0 0 0 0 0\n"
  "MouseMoveEvent 323 255 0 0 0 0 0\n"
  "MouseMoveEvent 323 253 0 0 0 0 0\n"
  "MouseMoveEvent 322 251 0 0 0 0 0\n"
  "MouseMoveEvent 322 248 0 0 0 0 0\n"
  "LeftButtonReleaseEvent 322 248 0 0 0 0 0\n"
  "RenderEvent 322 248 0 0 0 0 0\n"
  "MouseMoveEvent 320 247 0 0 0 0 0\n"
  "RenderEvent 205 272 0 0 0 0 0\n"
  "LeftButtonPressEvent 205 272 0 0 0 0 0\n"
  "MouseMoveEvent 205 274 0 0 0 0 0\n"
  "MouseMoveEvent 205 277 0 0 0 0 0\n"
  "MouseMoveEvent 205 278 0 0 0 0 0\n"
  "MouseMoveEvent 206 281 0 0 0 0 0\n"
  "MouseMoveEvent 207 286 0 0 0 0 0\n"
  "MouseMoveEvent 207 287 0 0 0 0 0\n"
  "MouseMoveEvent 208 287 0 0 0 0 0\n"
  "MouseMoveEvent 208 288 0 0 0 0 0\n"
  "MouseMoveEvent 209 289 0 0 0 0 0\n"
  "MouseMoveEvent 210 290 0 0 0 0 0\n"
  "MouseMoveEvent 211 291 0 0 0 0 0\n"
  "MouseMoveEvent 211 292 0 0 0 0 0\n"
  "MouseMoveEvent 212 293 0 0 0 0 0\n"
  "MouseMoveEvent 213 294 0 0 0 0 0\n"
  "MouseMoveEvent 213 295 0 0 0 0 0\n"
  "MouseMoveEvent 214 297 0 0 0 0 0\n"
  "MouseMoveEvent 215 297 0 0 0 0 0\n"
  "MouseMoveEvent 216 299 0 0 0 0 0\n"
  "MouseMoveEvent 216 300 0 0 0 0 0\n"
  "MouseMoveEvent 219 305 0 0 0 0 0\n"
  "MouseMoveEvent 220 307 0 0 0 0 0\n"
  "MouseMoveEvent 221 309 0 0 0 0 0\n"
  "MouseMoveEvent 222 312 0 0 0 0 0\n"
  "MouseMoveEvent 223 315 0 0 0 0 0\n"
  "MouseMoveEvent 223 316 0 0 0 0 0\n"
  "MouseMoveEvent 224 317 0 0 0 0 0\n"
  "MouseMoveEvent 225 321 0 0 0 0 0\n"
  "MouseMoveEvent 225 322 0 0 0 0 0\n"
  "MouseMoveEvent 226 325 0 0 0 0 0\n"
  "MouseMoveEvent 226 326 0 0 0 0 0\n"
  "MouseMoveEvent 226 327 0 0 0 0 0\n"
  "MouseMoveEvent 227 330 0 0 0 0 0\n"
  "MouseMoveEvent 227 331 0 0 0 0 0\n"
  "MouseMoveEvent 227 335 0 0 0 0 0\n"
  "MouseMoveEvent 227 337 0 0 0 0 0\n"
  "MouseMoveEvent 228 338 0 0 0 0 0\n"
  "MouseMoveEvent 228 339 0 0 0 0 0\n"
  "MouseMoveEvent 228 343 0 0 0 0 0\n"
  "MouseMoveEvent 228 344 0 0 0 0 0\n"
  "MouseMoveEvent 228 345 0 0 0 0 0\n"
  "MouseMoveEvent 228 347 0 0 0 0 0\n"
  "MouseMoveEvent 228 348 0 0 0 0 0\n"
  "MouseMoveEvent 228 350 0 0 0 0 0\n"
  "MouseMoveEvent 228 351 0 0 0 0 0\n"
  "MouseMoveEvent 228 353 0 0 0 0 0\n"
  "MouseMoveEvent 228 354 0 0 0 0 0\n"
  "MouseMoveEvent 228 355 0 0 0 0 0\n"
  "MouseMoveEvent 228 356 0 0 0 0 0\n"
  "MouseMoveEvent 229 357 0 0 0 0 0\n"
  "MouseMoveEvent 229 358 0 0 0 0 0\n"
  "MouseMoveEvent 230 359 0 0 0 0 0\n"
  "MouseMoveEvent 232 360 0 0 0 0 0\n"
  "MouseMoveEvent 232 361 0 0 0 0 0\n"
  "MouseMoveEvent 235 363 0 0 0 0 0\n"
  "MouseMoveEvent 235 364 0 0 0 0 0\n"
  "MouseMoveEvent 237 365 0 0 0 0 0\n"
  "MouseMoveEvent 237 366 0 0 0 0 0\n"
  "MouseMoveEvent 237 368 0 0 0 0 0\n"
  "MouseMoveEvent 239 369 0 0 0 0 0\n"
  "MouseMoveEvent 239 371 0 0 0 0 0\n"
  "MouseMoveEvent 240 372 0 0 0 0 0\n"
  "MouseMoveEvent 240 373 0 0 0 0 0\n"
  "MouseMoveEvent 241 373 0 0 0 0 0\n"
  "MouseMoveEvent 242 374 0 0 0 0 0\n"
  "MouseMoveEvent 243 375 0 0 0 0 0\n"
  "MouseMoveEvent 244 376 0 0 0 0 0\n"
  "MouseMoveEvent 245 376 0 0 0 0 0\n"
  "MouseMoveEvent 245 377 0 0 0 0 0\n"
  "MouseMoveEvent 246 377 0 0 0 0 0\n"
  "MouseMoveEvent 247 378 0 0 0 0 0\n"
  "MouseMoveEvent 248 379 0 0 0 0 0\n"
  "MouseMoveEvent 248 380 0 0 0 0 0\n"
  "MouseMoveEvent 249 381 0 0 0 0 0\n"
  "MouseMoveEvent 249 382 0 0 0 0 0\n"
  "MouseMoveEvent 250 382 0 0 0 0 0\n"
  "MouseMoveEvent 250 384 0 0 0 0 0\n"
  "MouseMoveEvent 251 385 0 0 0 0 0\n"
  "MouseMoveEvent 252 386 0 0 0 0 0\n"
  "MouseMoveEvent 252 388 0 0 0 0 0\n"
  "MouseMoveEvent 252 389 0 0 0 0 0\n"
  "MouseMoveEvent 252 390 0 0 0 0 0\n"
  "MouseMoveEvent 253 391 0 0 0 0 0\n"
  "MouseMoveEvent 252 391 0 0 0 0 0\n"
  "MouseMoveEvent 252 392 0 0 0 0 0\n"
  "MouseMoveEvent 253 393 0 0 0 0 0\n"
  "MouseMoveEvent 253 394 0 0 0 0 0\n"
  "MouseMoveEvent 253 395 0 0 0 0 0\n"
  "MouseMoveEvent 252 395 0 0 0 0 0\n"
  "MouseMoveEvent 252 396 0 0 0 0 0\n"
  "MouseMoveEvent 252 397 0 0 0 0 0\n"
  "MouseMoveEvent 252 398 0 0 0 0 0\n"
  "MouseMoveEvent 252 400 0 0 0 0 0\n"
  "MouseMoveEvent 252 401 0 0 0 0 0\n"
  "MouseMoveEvent 252 402 0 0 0 0 0\n"
  "MouseMoveEvent 251 409 0 0 0 0 0\n"
  "MouseMoveEvent 250 410 0 0 0 0 0\n"
  "MouseMoveEvent 249 410 0 0 0 0 0\n"
  "MouseMoveEvent 247 410 0 0 0 0 0\n"
  "MouseMoveEvent 241 409 0 0 0 0 0\n"
  "MouseMoveEvent 239 409 0 0 0 0 0\n"
  "MouseMoveEvent 238 409 0 0 0 0 0\n"
  "MouseMoveEvent 238 408 0 0 0 0 0\n"
  "MouseMoveEvent 234 406 0 0 0 0 0\n"
  "MouseMoveEvent 232 403 0 0 0 0 0\n"
  "MouseMoveEvent 232 402 0 0 0 0 0\n"
  "MouseMoveEvent 230 400 0 0 0 0 0\n"
  "MouseMoveEvent 228 397 0 0 0 0 0\n"
  "MouseMoveEvent 228 396 0 0 0 0 0\n"
  "MouseMoveEvent 227 394 0 0 0 0 0\n"
  "MouseMoveEvent 226 392 0 0 0 0 0\n"
  "MouseMoveEvent 226 391 0 0 0 0 0\n"
  "MouseMoveEvent 225 390 0 0 0 0 0\n"
  "MouseMoveEvent 225 388 0 0 0 0 0\n"
  "MouseMoveEvent 224 385 0 0 0 0 0\n"
  "MouseMoveEvent 224 384 0 0 0 0 0\n"
  "MouseMoveEvent 224 383 0 0 0 0 0\n"
  "MouseMoveEvent 223 381 0 0 0 0 0\n"
  "MouseMoveEvent 223 380 0 0 0 0 0\n"
  "MouseMoveEvent 223 378 0 0 0 0 0\n"
  "MouseMoveEvent 223 375 0 0 0 0 0\n"
  "MouseMoveEvent 222 372 0 0 0 0 0\n"
  "MouseMoveEvent 222 370 0 0 0 0 0\n"
  "MouseMoveEvent 221 366 0 0 0 0 0\n"
  "MouseMoveEvent 219 364 0 0 0 0 0\n"
  "MouseMoveEvent 219 363 0 0 0 0 0\n"
  "MouseMoveEvent 218 363 0 0 0 0 0\n"
  "RenderEvent 218 363 0 0 0 0 0\n"
  "LeftButtonReleaseEvent 218 363 0 0 0 0 0\n"
  "RenderEvent 218 363 0 0 0 0 0\n"
  "RenderEvent 204 368 0 0 0 0 0\n"
  "LeftButtonPressEvent 204 368 0 0 0 0 0\n"
  "RenderEvent 204 368 0 0 0 0 0\n"
  "MouseMoveEvent 205 368 0 0 0 0 0\n"
  "MouseMoveEvent 208 368 0 0 0 0 0\n"
  "MouseMoveEvent 214 368 0 0 0 0 0\n"
  "MouseMoveEvent 214 367 0 0 0 0 0\n"
  "MouseMoveEvent 215 367 0 0 0 0 0\n"
  "MouseMoveEvent 216 367 0 0 0 0 0\n"
  "MouseMoveEvent 217 367 0 0 0 0 0\n"
  "MouseMoveEvent 218 367 0 0 0 0 0\n"
  "MouseMoveEvent 219 366 0 0 0 0 0\n"
  "MouseMoveEvent 220 365 0 0 0 0 0\n"
  "MouseMoveEvent 221 364 0 0 0 0 0\n"
  "MouseMoveEvent 223 364 0 0 0 0 0\n"
  "MouseMoveEvent 224 362 0 0 0 0 0\n"
  "MouseMoveEvent 225 361 0 0 0 0 0\n"
  "MouseMoveEvent 226 360 0 0 0 0 0\n"
  "MouseMoveEvent 228 358 0 0 0 0 0\n"
  "MouseMoveEvent 228 356 0 0 0 0 0\n"
  "MouseMoveEvent 231 354 0 0 0 0 0\n"
  "MouseMoveEvent 232 353 0 0 0 0 0\n"
  "MouseMoveEvent 233 353 0 0 0 0 0\n"
  "MouseMoveEvent 233 352 0 0 0 0 0\n"
  "MouseMoveEvent 233 351 0 0 0 0 0\n"
  "MouseMoveEvent 234 349 0 0 0 0 0\n"
  "MouseMoveEvent 235 344 0 0 0 0 0\n"
  "MouseMoveEvent 235 342 0 0 0 0 0\n"
  "MouseMoveEvent 236 341 0 0 0 0 0\n"
  "MouseMoveEvent 236 339 0 0 0 0 0\n"
  "MouseMoveEvent 236 337 0 0 0 0 0\n"
  "MouseMoveEvent 238 335 0 0 0 0 0\n"
  "MouseMoveEvent 238 334 0 0 0 0 0\n"
  "MouseMoveEvent 238 333 0 0 0 0 0\n"
  "MouseMoveEvent 238 332 0 0 0 0 0\n"
  "MouseMoveEvent 238 330 0 0 0 0 0\n"
  "MouseMoveEvent 238 329 0 0 0 0 0\n"
  "MouseMoveEvent 239 325 0 0 0 0 0\n"
  "MouseMoveEvent 239 323 0 0 0 0 0\n"
  "MouseMoveEvent 240 322 0 0 0 0 0\n"
  "MouseMoveEvent 240 320 0 0 0 0 0\n"
  "MouseMoveEvent 245 313 0 0 0 0 0\n"
  "MouseMoveEvent 247 309 0 0 0 0 0\n"
  "MouseMoveEvent 249 306 0 0 0 0 0\n"
  "MouseMoveEvent 252 302 0 0 0 0 0\n"
  "MouseMoveEvent 254 299 0 0 0 0 0\n"
  "MouseMoveEvent 256 296 0 0 0 0 0\n"
  "MouseMoveEvent 258 294 0 0 0 0 0\n"
  "MouseMoveEvent 259 293 0 0 0 0 0\n"
  "MouseMoveEvent 259 291 0 0 0 0 0\n"
  "MouseMoveEvent 259 287 0 0 0 0 0\n"
  "MouseMoveEvent 262 277 0 0 0 0 0\n"
  "MouseMoveEvent 264 271 0 0 0 0 0\n"
  "MouseMoveEvent 265 268 0 0 0 0 0\n"
  "MouseMoveEvent 265 267 0 0 0 0 0\n"
  "MouseMoveEvent 266 266 0 0 0 0 0\n"
  "MouseMoveEvent 266 264 0 0 0 0 0\n"
  "MouseMoveEvent 267 262 0 0 0 0 0\n"
  "MouseMoveEvent 269 259 0 0 0 0 0\n"
  "MouseMoveEvent 271 254 0 0 0 0 0\n"
  "MouseMoveEvent 272 253 0 0 0 0 0\n"
  "MouseMoveEvent 272 252 0 0 0 0 0\n"
  "MouseMoveEvent 272 251 0 0 0 0 0\n"
  "MouseMoveEvent 272 253 0 0 0 0 0\n"
  "MouseMoveEvent 271 256 0 0 0 0 0\n"
  "MouseMoveEvent 265 274 0 0 0 0 0\n"
  "MouseMoveEvent 256 293 0 0 0 0 0\n"
  "MouseMoveEvent 256 294 0 0 0 0 0\n"
  "MouseMoveEvent 256 296 0 0 0 0 0\n"
  "MouseMoveEvent 255 300 0 0 0 0 0\n"
  "MouseMoveEvent 254 304 0 0 0 0 0\n"
  "MouseMoveEvent 253 305 0 0 0 0 0\n"
  "MouseMoveEvent 253 307 0 0 0 0 0\n"
  "MouseMoveEvent 252 311 0 0 0 0 0\n"
  "MouseMoveEvent 252 314 0 0 0 0 0\n"
  "MouseMoveEvent 250 317 0 0 0 0 0\n"
  "MouseMoveEvent 248 320 0 0 0 0 0\n"
  "MouseMoveEvent 245 326 0 0 0 0 0\n"
  "MouseMoveEvent 242 331 0 0 0 0 0\n"
  "MouseMoveEvent 241 333 0 0 0 0 0\n"
  "MouseMoveEvent 240 334 0 0 0 0 0\n"
  "MouseMoveEvent 239 338 0 0 0 0 0\n"
  "MouseMoveEvent 238 339 0 0 0 0 0\n"
  "MouseMoveEvent 237 341 0 0 0 0 0\n"
  "MouseMoveEvent 237 342 0 0 0 0 0\n"
  "MouseMoveEvent 236 343 0 0 0 0 0\n"
  "MouseMoveEvent 233 347 0 0 0 0 0\n"
  "MouseMoveEvent 232 348 0 0 0 0 0\n"
  "MouseMoveEvent 230 352 0 0 0 0 0\n"
  "MouseMoveEvent 228 356 0 0 0 0 0\n"
  "MouseMoveEvent 227 357 0 0 0 0 0\n"
  "MouseMoveEvent 223 362 0 0 0 0 0\n"
  "MouseMoveEvent 219 367 0 0 0 0 0\n"
  "MouseMoveEvent 218 367 0 0 0 0 0\n"
  "LeftButtonReleaseEvent 218 367 0 0 0 0 0\n"
  "RenderEvent 143 113 0 0 0 0 0\n"
  "LeftButtonPressEvent 143 113 0 0 0 0 0\n"
  "StartInteractionEvent 143 113 0 0 0 0 0\n"
  "MouseMoveEvent 144 113 0 0 0 0 0\n"
  "MouseMoveEvent 146 114 0 0 0 0 0\n"
  "MouseMoveEvent 147 114 0 0 0 0 0\n"
  "MouseMoveEvent 148 114 0 0 0 0 0\n"
  "MouseMoveEvent 149 115 0 0 0 0 0\n"
  "MouseMoveEvent 150 115 0 0 0 0 0\n"
  "MouseMoveEvent 152 115 0 0 0 0 0\n"
  "MouseMoveEvent 156 115 0 0 0 0 0\n"
  "MouseMoveEvent 159 116 0 0 0 0 0\n"
  "MouseMoveEvent 162 116 0 0 0 0 0\n"
  "MouseMoveEvent 166 116 0 0 0 0 0\n"
  "MouseMoveEvent 169 116 0 0 0 0 0\n"
  "MouseMoveEvent 175 116 0 0 0 0 0\n"
  "MouseMoveEvent 180 116 0 0 0 0 0\n"
  "MouseMoveEvent 186 115 0 0 0 0 0\n"
  "MouseMoveEvent 189 116 0 0 0 0 0\n"
  "MouseMoveEvent 193 116 0 0 0 0 0\n"
  "MouseMoveEvent 196 116 0 0 0 0 0\n"
  "MouseMoveEvent 200 116 0 0 0 0 0\n"
  "MouseMoveEvent 202 116 0 0 0 0 0\n"
  "MouseMoveEvent 204 115 0 0 0 0 0\n"
  "MouseMoveEvent 206 115 0 0 0 0 0\n"
  "MouseMoveEvent 207 115 0 0 0 0 0\n"
  "MouseMoveEvent 209 115 0 0 0 0 0\n"
  "MouseMoveEvent 211 115 0 0 0 0 0\n"
  "MouseMoveEvent 212 115 0 0 0 0 0\n"
  "MouseMoveEvent 213 115 0 0 0 0 0\n"
  "MouseMoveEvent 214 115 0 0 0 0 0\n"
  "MouseMoveEvent 215 115 0 0 0 0 0\n"
  "MouseMoveEvent 216 114 0 0 0 0 0\n"
  "MouseMoveEvent 218 114 0 0 0 0 0\n"
  "MouseMoveEvent 220 114 0 0 0 0 0\n"
  "MouseMoveEvent 221 115 0 0 0 0 0\n"
  "MouseMoveEvent 224 114 0 0 0 0 0\n"
  "MouseMoveEvent 225 114 0 0 0 0 0\n"
  "MouseMoveEvent 227 113 0 0 0 0 0\n"
  "MouseMoveEvent 230 113 0 0 0 0 0\n"
  "MouseMoveEvent 234 113 0 0 0 0 0\n"
  "MouseMoveEvent 239 111 0 0 0 0 0\n"
  "MouseMoveEvent 242 111 0 0 0 0 0\n"
  "MouseMoveEvent 249 110 0 0 0 0 0\n"
  "MouseMoveEvent 251 110 0 0 0 0 0\n"
  "MouseMoveEvent 253 110 0 0 0 0 0\n"
  "MouseMoveEvent 257 110 0 0 0 0 0\n"
  "MouseMoveEvent 260 110 0 0 0 0 0\n"
  "MouseMoveEvent 262 109 0 0 0 0 0\n"
  "MouseMoveEvent 266 109 0 0 0 0 0\n"
  "MouseMoveEvent 268 108 0 0 0 0 0\n"
  "MouseMoveEvent 270 108 0 0 0 0 0\n"
  "MouseMoveEvent 272 108 0 0 0 0 0\n"
  "MouseMoveEvent 273 108 0 0 0 0 0\n"
  "MouseMoveEvent 274 108 0 0 0 0 0\n"
  "MouseMoveEvent 275 108 0 0 0 0 0\n"
  "MouseMoveEvent 276 108 0 0 0 0 0\n"
  "MouseMoveEvent 277 108 0 0 0 0 0\n"
  "MouseMoveEvent 278 108 0 0 0 0 0\n"
  "MouseMoveEvent 279 108 0 0 0 0 0\n"
  "MouseMoveEvent 280 108 0 0 0 0 0\n"
  "MouseMoveEvent 281 108 0 0 0 0 0\n"
  "MouseMoveEvent 282 109 0 0 0 0 0\n"
  "MouseMoveEvent 283 109 0 0 0 0 0\n"
  "MouseMoveEvent 284 109 0 0 0 0 0\n"
  "MouseMoveEvent 285 109 0 0 0 0 0\n"
  "MouseMoveEvent 286 109 0 0 0 0 0\n"
  "MouseMoveEvent 287 109 0 0 0 0 0\n"
  "MouseMoveEvent 288 109 0 0 0 0 0\n"
  "MouseMoveEvent 289 109 0 0 0 0 0\n"
  "MouseMoveEvent 291 109 0 0 0 0 0\n"
  "MouseMoveEvent 292 109 0 0 0 0 0\n"
  "MouseMoveEvent 295 110 0 0 0 0 0\n"
  "MouseMoveEvent 296 110 0 0 0 0 0\n"
  "MouseMoveEvent 297 110 0 0 0 0 0\n"
  "MouseMoveEvent 298 110 0 0 0 0 0\n"
  "MouseMoveEvent 299 110 0 0 0 0 0\n"
  "MouseMoveEvent 300 110 0 0 0 0 0\n"
  "MouseMoveEvent 301 110 0 0 0 0 0\n"
  "MouseMoveEvent 302 110 0 0 0 0 0\n"
  "LeftButtonReleaseEvent 302 110 0 0 0 0 0\n"
  "EndInteractionEvent 302 110 0 0 0 0 0\n"
  "RenderEvent 302 110 0 0 0 0 0\n"
  "MouseMoveEvent 299 112 0 0 0 0 0\n"
  "MouseMoveEvent 395 486 0 0 0 0 0\n"
  "LeftButtonPressEvent 395 486 0 0 0 0 0\n"
  "StartInteractionEvent 395 486 0 0 0 0 0\n"
  "MouseMoveEvent 394 486 0 0 0 0 0\n"
  "MouseMoveEvent 392 485 0 0 0 0 0\n"
  "MouseMoveEvent 389 483 0 0 0 0 0\n"
  "MouseMoveEvent 387 482 0 0 0 0 0\n"
  "MouseMoveEvent 383 480 0 0 0 0 0\n"
  "MouseMoveEvent 380 479 0 0 0 0 0\n"
  "MouseMoveEvent 377 477 0 0 0 0 0\n"
  "MouseMoveEvent 373 476 0 0 0 0 0\n"
  "MouseMoveEvent 370 475 0 0 0 0 0\n"
  "MouseMoveEvent 367 474 0 0 0 0 0\n"
  "MouseMoveEvent 363 473 0 0 0 0 0\n"
  "MouseMoveEvent 358 473 0 0 0 0 0\n"
  "MouseMoveEvent 355 473 0 0 0 0 0\n"
  "MouseMoveEvent 346 474 0 0 0 0 0\n"
  "MouseMoveEvent 340 474 0 0 0 0 0\n"
  "MouseMoveEvent 332 474 0 0 0 0 0\n"
  "MouseMoveEvent 321 474 0 0 0 0 0\n"
  "MouseMoveEvent 312 474 0 0 0 0 0\n"
  "MouseMoveEvent 302 476 0 0 0 0 0\n"
  "MouseMoveEvent 294 476 0 0 0 0 0\n"
  "MouseMoveEvent 287 476 0 0 0 0 0\n"
  "MouseMoveEvent 280 476 0 0 0 0 0\n"
  "MouseMoveEvent 269 476 0 0 0 0 0\n"
  "MouseMoveEvent 263 476 0 0 0 0 0\n"
  "MouseMoveEvent 257 476 0 0 0 0 0\n"
  "MouseMoveEvent 251 476 0 0 0 0 0\n"
  "MouseMoveEvent 245 478 0 0 0 0 0\n"
  "MouseMoveEvent 240 478 0 0 0 0 0\n"
  "MouseMoveEvent 234 476 0 0 0 0 0\n"
  "MouseMoveEvent 227 476 0 0 0 0 0\n"
  "MouseMoveEvent 222 474 0 0 0 0 0\n"
  "MouseMoveEvent 218 474 0 0 0 0 0\n"
  "MouseMoveEvent 215 473 0 0 0 0 0\n"
  "MouseMoveEvent 213 472 0 0 0 0 0\n"
  "MouseMoveEvent 212 471 0 0 0 0 0\n"
  "MouseMoveEvent 210 471 0 0 0 0 0\n"
  "MouseMoveEvent 208 470 0 0 0 0 0\n"
  "MouseMoveEvent 204 469 0 0 0 0 0\n"
  "MouseMoveEvent 202 468 0 0 0 0 0\n"
  "MouseMoveEvent 200 466 0 0 0 0 0\n"
  "MouseMoveEvent 198 465 0 0 0 0 0\n"
  "MouseMoveEvent 196 464 0 0 0 0 0\n"
  "MouseMoveEvent 194 463 0 0 0 0 0\n"
  "MouseMoveEvent 193 462 0 0 0 0 0\n"
  "MouseMoveEvent 192 462 0 0 0 0 0\n"
  "MouseMoveEvent 191 462 0 0 0 0 0\n"
  "MouseMoveEvent 191 461 0 0 0 0 0\n"
  "MouseMoveEvent 190 461 0 0 0 0 0\n"
  "MouseMoveEvent 189 461 0 0 0 0 0\n"
  "MouseMoveEvent 188 461 0 0 0 0 0\n"
  "MouseMoveEvent 185 461 0 0 0 0 0\n"
  "MouseMoveEvent 183 461 0 0 0 0 0\n"
  "MouseMoveEvent 181 461 0 0 0 0 0\n"
  "MouseMoveEvent 179 461 0 0 0 0 0\n"
  "MouseMoveEvent 176 460 0 0 0 0 0\n"
  "MouseMoveEvent 174 460 0 0 0 0 0\n"
  "MouseMoveEvent 172 460 0 0 0 0 0\n"
  "MouseMoveEvent 170 460 0 0 0 0 0\n"
  "MouseMoveEvent 169 460 0 0 0 0 0\n"
  "MouseMoveEvent 168 460 0 0 0 0 0\n"
  "MouseMoveEvent 167 460 0 0 0 0 0\n"
  "LeftButtonReleaseEvent 167 460 0 0 0 0 0\n"
  "EndInteractionEvent 167 460 0 0 0 0 0\n"
  "RenderEvent 167 460 0 0 0 0 0\n"
  "MouseMoveEvent 167 458 0 0 0 0 0\n"
  "MouseMoveEvent 592 375 0 0 0 0 0\n"
  "LeaveEvent 611 377 0 0 0 0 0\n"
  "EnterEvent 599 426 0 0 0 0 0\n"
  "MouseMoveEvent 599 426 0 0 0 0 0\n"
  "MouseMoveEvent 594 431 0 0 0 0 0\n"
  "MouseMoveEvent 590 432 0 0 0 0 0\n"
  "MouseMoveEvent 587 435 0 0 0 0 0\n"
  "MouseMoveEvent 584 438 0 0 0 0 0\n"
  "MouseMoveEvent 582 439 0 0 0 0 0\n"
  "MouseMoveEvent 581 440 0 0 0 0 0\n"
  "MouseMoveEvent 581 441 0 0 0 0 0\n"
  "MouseMoveEvent 581 442 0 0 0 0 0\n"
  "MouseMoveEvent 581 443 0 0 0 0 0\n"
  "MouseMoveEvent 581 444 0 0 0 0 0\n"
  "MouseMoveEvent 581 446 0 0 0 0 0\n"
  "MouseMoveEvent 581 447 0 0 0 0 0\n"
  "MouseMoveEvent 582 448 0 0 0 0 0\n"
  "MouseMoveEvent 582 449 0 0 0 0 0\n"
  "MouseMoveEvent 583 450 0 0 0 0 0\n"
  "MouseMoveEvent 584 451 0 0 0 0 0\n"
  "MouseMoveEvent 585 452 0 0 0 0 0\n"
  "MouseMoveEvent 586 452 0 0 0 0 0\n"
  "MouseMoveEvent 586 453 0 0 0 0 0\n"
  "MouseMoveEvent 587 453 0 0 0 0 0\n"
  "MouseMoveEvent 587 454 0 0 0 0 0\n"
  "MouseMoveEvent 588 454 0 0 0 0 0\n"
  "MouseMoveEvent 588 456 0 0 0 0 0\n"
  "MouseMoveEvent 589 457 0 0 0 0 0\n"
  "MouseMoveEvent 589 459 0 0 0 0 0\n"
  "MouseMoveEvent 589 461 0 0 0 0 0\n"
  "MouseMoveEvent 589 462 0 0 0 0 0\n"
  "MouseMoveEvent 588 463 0 0 0 0 0\n"
  "MouseMoveEvent 588 465 0 0 0 0 0\n"
  "MouseMoveEvent 587 467 0 0 0 0 0\n"
  "MouseMoveEvent 586 468 0 0 0 0 0\n"
  "MouseMoveEvent 586 472 0 0 0 0 0\n"
  "MouseMoveEvent 585 474 0 0 0 0 0\n"
  "MouseMoveEvent 585 475 0 0 0 0 0\n"
  "MouseMoveEvent 585 477 0 0 0 0 0\n"
  "MouseMoveEvent 585 478 0 0 0 0 0\n"
  "MouseMoveEvent 585 479 0 0 0 0 0\n"
  "MouseMoveEvent 585 481 0 0 0 0 0\n"
  "MouseMoveEvent 585 482 0 0 0 0 0\n"
  "MouseMoveEvent 585 483 0 0 0 0 0\n"
  "MouseMoveEvent 585 484 0 0 0 0 0\n"
  "MouseMoveEvent 585 485 0 0 0 0 0\n"
  "MouseMoveEvent 585 487 0 0 0 0 0\n"
  "MouseMoveEvent 585 489 0 0 0 0 0\n"
  "MouseMoveEvent 586 491 0 0 0 0 0\n"
  "MouseMoveEvent 586 493 0 0 0 0 0\n"
  "MouseMoveEvent 586 494 0 0 0 0 0\n"
  "MouseMoveEvent 586 495 0 0 0 0 0\n"
  "MouseMoveEvent 586 497 0 0 0 0 0\n"
  "MouseMoveEvent 586 498 0 0 0 0 0\n"
  "MouseMoveEvent 586 499 0 0 0 0 0\n"
  "MouseMoveEvent 586 502 0 0 0 0 0\n"
  "MouseMoveEvent 586 504 0 0 0 0 0\n"
  "MouseMoveEvent 586 505 0 0 0 0 0\n"
  "MouseMoveEvent 586 506 0 0 0 0 0\n"
  "MouseMoveEvent 586 507 0 0 0 0 0\n"
  "MouseMoveEvent 586 508 0 0 0 0 0\n"
  "MouseMoveEvent 586 514 0 0 0 0 0\n"
  "MouseMoveEvent 586 517 0 0 0 0 0\n"
  "MouseMoveEvent 586 519 0 0 0 0 0\n"
  "MouseMoveEvent 586 521 0 0 0 0 0\n"
  "MouseMoveEvent 586 522 0 0 0 0 0\n"
  "MouseMoveEvent 586 523 0 0 0 0 0\n"
  "MouseMoveEvent 585 524 0 0 0 0 0\n"
  "MouseMoveEvent 585 525 0 0 0 0 0\n"
  "MouseMoveEvent 585 526 0 0 0 0 0\n"
  "MouseMoveEvent 584 526 0 0 0 0 0\n"
  "MouseMoveEvent 584 527 0 0 0 0 0\n"
  "MouseMoveEvent 583 527 0 0 0 0 0\n"
  "MouseMoveEvent 582 528 0 0 0 0 0\n"
  "MouseMoveEvent 581 528 0 0 0 0 0\n"
  "MouseMoveEvent 580 529 0 0 0 0 0\n"
  "MouseMoveEvent 579 529 0 0 0 0 0\n"
  "MouseMoveEvent 579 531 0 0 0 0 0\n"
  "MouseMoveEvent 577 532 0 0 0 0 0\n"
  "MouseMoveEvent 575 534 0 0 0 0 0\n"
  "MouseMoveEvent 575 535 0 0 0 0 0\n"
  "MouseMoveEvent 574 537 0 0 0 0 0\n"
  "MouseMoveEvent 573 538 0 0 0 0 0\n"
  "MouseMoveEvent 572 539 0 0 0 0 0\n"
  "MouseMoveEvent 570 541 0 0 0 0 0\n"
  "MouseMoveEvent 569 542 0 0 0 0 0\n"
  "MouseMoveEvent 567 543 0 0 0 0 0\n"
  "MouseMoveEvent 566 546 0 0 0 0 0\n"
  "MouseMoveEvent 566 548 0 0 0 0 0\n"
  "MouseMoveEvent 565 550 0 0 0 0 0\n"
  "MouseMoveEvent 565 552 0 0 0 0 0\n"
  "MouseMoveEvent 565 553 0 0 0 0 0\n"
  "MouseMoveEvent 565 555 0 0 0 0 0\n"
  "MouseMoveEvent 564 557 0 0 0 0 0\n"
  "MouseMoveEvent 564 559 0 0 0 0 0\n"
  "MouseMoveEvent 564 560 0 0 0 0 0\n"
  "MouseMoveEvent 564 561 0 0 0 0 0\n"
  "MouseMoveEvent 563 561 0 0 0 0 0\n"
  "MouseMoveEvent 563 562 0 0 0 0 0\n"
  "MouseMoveEvent 563 564 0 0 0 0 0\n"
  "MouseMoveEvent 563 565 0 0 0 0 0\n"
  "MouseMoveEvent 562 567 0 0 0 0 0\n"
  "MouseMoveEvent 562 569 0 0 0 0 0\n"
  "MouseMoveEvent 562 571 0 0 0 0 0\n"
  "MouseMoveEvent 562 572 0 0 0 0 0\n"
  "MouseMoveEvent 563 573 0 0 0 0 0\n"
  "MouseMoveEvent 563 576 0 0 0 0 0\n"
  "MouseMoveEvent 563 577 0 0 0 0 0\n"
  "MouseMoveEvent 565 579 0 0 0 0 0\n"
  "MouseMoveEvent 566 581 0 0 0 0 0\n"
  "MouseMoveEvent 567 585 0 0 0 0 0\n"
  "MouseMoveEvent 568 586 0 0 0 0 0\n"
  "MouseMoveEvent 569 587 0 0 0 0 0\n"
  "MouseMoveEvent 569 589 0 0 0 0 0\n"
  "MouseMoveEvent 570 590 0 0 0 0 0\n"
  "MouseMoveEvent 571 591 0 0 0 0 0\n"
  "MouseMoveEvent 571 592 0 0 0 0 0\n"
  "MouseMoveEvent 571 593 0 0 0 0 0\n"
  "MouseMoveEvent 571 594 0 0 0 0 0\n"
  "MouseMoveEvent 572 594 0 0 0 0 0\n"
  "MouseMoveEvent 572 595 0 0 0 0 0\n"
  "MouseMoveEvent 573 595 0 0 0 0 0\n"
  "MouseMoveEvent 574 595 0 0 0 0 0\n"
  "MouseMoveEvent 574 596 0 0 0 0 0\n"
  "MouseMoveEvent 575 598 0 0 0 0 0\n"
  "MouseMoveEvent 576 599 0 0 0 0 0\n"
  "LeaveEvent 576 600 0 0 0 0 0\n"
  "EnterEvent 593 598 0 0 0 0 0\n"
  "MouseMoveEvent 593 598 0 0 0 0 0\n"
  "MouseMoveEvent 598 595 0 0 0 0 0\n"
  "LeaveEvent 604 592 0 0 0 0 0\n"
  ;

namespace
{

//----------------------------------------------------------------------------
class vtkAbortCommand: public vtkCommand
{
public:
  static vtkAbortCommand *New(){ return new vtkAbortCommand; }
  void Execute (vtkObject* vtkNotUsed(caller),
                        unsigned long vtkNotUsed(eventId),
                        void* vtkNotUsed(callData)) override
    {
    this->SetAbortFlag(1);
    }
};

class vtkRenderCallback : public vtkCommand
{
public:
  static vtkRenderCallback *New()
    {
    return new vtkRenderCallback;
    }
  void Execute(vtkObject *vtkNotUsed(caller), unsigned long vtkNotUsed(eventId), void* vtkNotUsed(callData)) override
    {
    this->RenderWindow->Render();
    }
  vtkRenderCallback()  = default;
  vtkRenderWindow *RenderWindow{nullptr};
};

};

//----------------------------------------------------------------------------
int vtkMRMLThreeDReformatDisplayableManagerTest1(int argc, char* argv[])
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindow->SetSize(600, 600);
  renderWindow->SetMultiSamples(0); // Ensure to have the same test image everywhere

  renderWindow->AddRenderer(renderer.GetPointer());
  renderWindow->SetInteractor(renderWindowInteractor.GetPointer());

  // Set Interactor Style
  vtkNew<vtkMRMLThreeDViewInteractorStyle> iStyle;
  renderWindowInteractor->SetInteractorStyle(iStyle.GetPointer());

  renderWindow->Render();

  // MRML scene
  vtkMRMLScene* scene = vtkMRMLScene::New();

  // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
  vtkMRMLApplicationLogic* applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);
  // Add ViewNode
  vtkNew<vtkMRMLViewNode> viewNode;
  scene->AddNode(viewNode.GetPointer());

  vtkNew<vtkMRMLDisplayableManagerGroup> displayableManagerGroup;
  displayableManagerGroup->SetRenderer(renderer.GetPointer());
  displayableManagerGroup->SetMRMLDisplayableNode(viewNode.GetPointer());

  vtkNew<vtkRenderCallback> renderCallback;
  renderCallback->RenderWindow = renderWindow;
  displayableManagerGroup->AddObserver(vtkCommand::UpdateEvent, renderCallback);

  iStyle->SetDisplayableManagers(displayableManagerGroup);

  vtkNew<vtkMRMLThreeDReformatDisplayableManager> reformatDisplayableManager;
  reformatDisplayableManager->SetMRMLApplicationLogic(applicationLogic);
  displayableManagerGroup->AddDisplayableManager(reformatDisplayableManager.GetPointer());

  // Need to add vtkMRMLCameraDisplayableManager, as this processes camera manipulation events
  vtkNew<vtkMRMLCameraDisplayableManager> cameraDisplayableManager;
  cameraDisplayableManager->SetMRMLApplicationLogic(applicationLogic);
  displayableManagerGroup->AddDisplayableManager(cameraDisplayableManager.GetPointer());

  // Camera displayable manager sets default camera position/orientation.
  // Move camera back far enough to see the reformat widgets.
  renderer->GetActiveCamera()->SetPosition(0, 0, -500.);
  renderer->GetActiveCamera()->SetViewUp(0, 1., 0);

  // Visible when added
  vtkNew<vtkMRMLSliceNode> sliceNodeRed;
  // TODO: This color should be taken into account, not the layout name
  sliceNodeRed->SetLayoutColor(vtkMRMLAbstractViewNode::GetRedColor());
  sliceNodeRed->SetWidgetVisible(1);

  scene->AddNode(sliceNodeRed.GetPointer());

  // Locked to Camera
  vtkNew<vtkMRMLSliceNode> sliceNodeYellow;
  sliceNodeYellow->SetLayoutColor(vtkMRMLAbstractViewNode::GetYellowColor());
  sliceNodeYellow->SetWidgetVisible(1);
  sliceNodeYellow->SetWidgetNormalLockedToCamera(1);

  scene->AddNode(sliceNodeYellow.GetPointer());

  // Delayed Visibility
  vtkNew<vtkMRMLSliceNode> sliceNodeGreen;
  sliceNodeGreen->SetLayoutColor(vtkMRMLAbstractViewNode::GetGreenColor());
  sliceNodeGreen->SetWidgetNormalLockedToCamera(1);
  sliceNodeGreen->SetSliceOffset(-20);

  scene->AddNode(sliceNodeGreen.GetPointer());

  sliceNodeGreen->SetWidgetVisible(1);

  // Deleted slice
  vtkNew<vtkMRMLSliceNode> sliceNodeDeleted;
  sliceNodeDeleted->SetLayoutColor(1., 1., 1.);
  sliceNodeDeleted->SetSliceOffset(300);
  sliceNodeDeleted->SetWidgetVisible(1);

  scene->AddNode(sliceNodeDeleted.GetPointer());
  scene->RemoveNode(sliceNodeDeleted.GetPointer());

  // Imported slice
  vtkNew<vtkMRMLSliceNode> sliceNodeImported;
  sliceNodeImported->SetLayoutColor(0.5, 0.3, 0.4);
  sliceNodeImported->SetSliceOffset(10.);
  sliceNodeImported->SetWidgetVisible(1);

  // Simulate a scene loading
  scene->StartState(vtkMRMLScene::ImportState);
  scene->AddNode(sliceNodeImported.GetPointer());
  scene->EndState(vtkMRMLScene::ImportState);

  // Restored slice
  vtkNew<vtkMRMLSliceNode> sliceNodeRestored;
  sliceNodeRestored->SetLayoutColor(0.8, 0.8, 0.8);
  sliceNodeRestored->SetSliceOffset(-10.);
  sliceNodeRestored->SetWidgetVisible(1);

  // Simulate a scene restore
  scene->StartState(vtkMRMLScene::RestoreState);
  scene->AddNode(sliceNodeRestored.GetPointer());
  scene->EndState(vtkMRMLScene::RestoreState);

  // TODO: Automatically move the camera (simulating movements)
  // to have a good screenshot.

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
      recorder->SetInputString(vtkMRMLThreeDReformatDisplayableManagerTest1EventLog);
      recorder->Play();
      }
    }

  int retval = vtkRegressionTestImageThreshold(renderWindow.GetPointer(), 85.0);
  if ( record || retval == vtkRegressionTester::DO_INTERACTOR)
    {
    displayableManagerGroup->GetInteractor()->Initialize();
    displayableManagerGroup->GetInteractor()->Start();
    }

  if (record || screenshot)
    {
    vtkNew<vtkWindowToImageFilter> windowToImageFilter;
    windowToImageFilter->SetInput(renderWindow.GetPointer());
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 1)
    windowToImageFilter->SetScale(1, 1); //set the resolution of the output image
#else
    windowToImageFilter->SetMagnification(1); //set the resolution of the output image
#endif
    windowToImageFilter->Update();

    vtkNew<vtkTesting> testHelper;
    testHelper->AddArguments(argc, const_cast<const char **>(argv));

    vtkStdString screenshootFilename = testHelper->GetDataRoot();
    screenshootFilename += "/Baseline/vtkMRMLThreeDReformatDisplayableManagerTest1.png";
    vtkNew<vtkPNGWriter> writer;
    writer->SetFileName(screenshootFilename.c_str());
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();
    std::cout << "Saved screenshot: " << screenshootFilename << std::endl;
    }

  reformatDisplayableManager->SetMRMLApplicationLogic(nullptr);
  applicationLogic->Delete();
  scene->Delete();

  return !retval;
}

