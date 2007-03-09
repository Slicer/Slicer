#include "igtauroratracker.h"

// Constructors/Destructors
//  

IGTAuroraTracker::IGTAuroraTracker ( ) {
}

IGTAuroraTracker::~IGTAuroraTracker ( ) { }

//  
// Methods
//  


// Accessor methods
//  


// Other methods
//  



#include "igtimagestate.h"

// Constructors/Destructors
//  

IGTImageState::IGTImageState ( ) {
initAttributes();
}

IGTImageState::~IGTImageState ( ) { }

//  
// Methods
//  


// Accessor methods
//  


// Public static attribute accessor methods
//  


// Public attribute accessor methods
//  


// Protected static attribute accessor methods
//  


// Protected attribute accessor methods
//  


// Private static attribute accessor methods
//  


// Private attribute accessor methods
//  


/**
 * Set the value of m_ImageSizeX
 * @param new_var the new value of m_ImageSizeX
 */
void IGTImageState::setImageSizeX ( int new_var ) {
  m_ImageSizeX = new_var;
}

/**
 * Get the value of m_ImageSizeX
 * @return the value of m_ImageSizeX
 */
int IGTImageState::getImageSizeX ( ) {
  return m_ImageSizeX;
}

/**
 * Set the value of m_ImageSizeY
 * @param new_var the new value of m_ImageSizeY
 */
void IGTImageState::setImageSizeY ( int new_var ) {
  m_ImageSizeY = new_var;
}

/**
 * Get the value of m_ImageSizeY
 * @return the value of m_ImageSizeY
 */
int IGTImageState::getImageSizeY ( ) {
  return m_ImageSizeY;
}

/**
 * Set the value of m_image
 * @param new_var the new value of m_image
 */
void IGTImageState::setImage ( vtkImageData new_var ) {
  m_image = new_var;
}

/**
 * Get the value of m_image
 * @return the value of m_image
 */
vtkImageData IGTImageState::getImage ( ) {
  return m_image;
}

// Other methods
//  

void IGTImageState::initAttributes ( ) {
}

