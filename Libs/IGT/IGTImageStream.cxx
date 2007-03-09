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



#include "igtimagestream.h"

// Constructors/Destructors
//  

IGTImageStream::IGTImageStream ( ) {
initAttributes();
}

IGTImageStream::~IGTImageStream ( ) { }

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
 * Set the value of m_ImageBuffer
 * @param new_var the new value of m_ImageBuffer
 */
void IGTImageStream::setImageBuffer ( IGTImageState new_var ) {
  m_ImageBuffer = new_var;
}

/**
 * Get the value of m_ImageBuffer
 * @return the value of m_ImageBuffer
 */
IGTImageState IGTImageStream::getImageBuffer ( ) {
  return m_ImageBuffer;
}

// Other methods
//  


/**
 */
void IGTImageStream::obtain_patient_to_image_matrix ( ) {

}

void IGTImageStream::initAttributes ( ) {
}

