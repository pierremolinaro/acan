#ifndef _SET_TEMPLATE_DEFINED
#define _SET_TEMPLATE_DEFINED

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <stdlib.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

template <typename T> class Set {
//--- Default constructor
  public: Set (void) ;

//--- Handle copy
  public: Set (const Set &inOperand) ;
  public: Set & operator = (const Set & inOperand) ;

//--- Destructor
  public: ~Set (void) ;

//--- Accessors
  public : inline size_t count (void) const { return mCount ; }
  public : inline bool empty (void) const { return mCount == 0 ; }
  public : inline size_t capacity (void) const { return mCapacity ; }
  public : T valueAtIndex (const size_t inIndex) const ;

//--- Set operations
  public : inline void clear (void) { mCount = 0 ; }
  public : void insert( const T & inItem) ;

//--- Equality test
  public: bool operator == (const Set<T> & inOperand) const ;
  public: bool inline operator != (const Set<T> & inOperand) const { return !(*this == inOperand) ; }

//--- Private properties
  private: T * mArray = NULL ;
  private: size_t mCapacity = 0 ;
  private: size_t mCount = 0 ;
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

template <typename T> Set<T>::Set (void) {
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

template <typename T> Set<T>::Set (const Set & inOperand) {
  mCount = inOperand.mCount ;
  mCapacity = inOperand.mCapacity ;
  mArray = new T [mCount] ;
  for (size_t i = 0 ; i < mCount ; i++) {
    mArray[i] = inOperand.mArray [i];
  }
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

template <typename T> Set<T>::~Set() {
  delete [] mArray ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

template <typename T> Set<T> & Set<T>::operator = (const Set & inOperand) {
  if (this != & inOperand) {
    if (mCapacity <= inOperand.mCount) {
      delete [] mArray ; mArray = new T [inOperand.mCount] ;
      mCapacity = inOperand.mCount ;
    }
    for (size_t i = 0 ; i < inOperand.mCount ; i++) {
      mArray [i] = inOperand.mArray [i] ;
    }
  }
  return *this ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

template <typename T> void Set<T>::insert (const T & inItem) {
  size_t insertionIndex = 0 ;
  bool loop = true ;
  bool found = false ;
  for (size_t i = 0 ; (i < mCount) && loop ; i++) {
    if (mArray [i] < inItem) {
      insertionIndex ++ ;
    }else if (mArray [i] == inItem) {
      loop = false ;
      found = true ;
    }else{
      loop = false ;
    }
  }
  if (!found) {
    if (mCount == mCapacity) {
      mCapacity ++ ;
      T * newArray = new T [mCapacity] ;
      for (size_t i = 0 ; i < mCount ; i++) {
        newArray [i] = mArray [i] ;
      }
      delete [] mArray ; mArray = newArray ;
    }
    for (size_t i = mCount ; i>insertionIndex ; i--) {
      mArray [i] = mArray [i-1] ;
    }
    mArray [insertionIndex] = inItem ;
    mCount ++ ;
  }
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

template <typename T> bool Set<T>::operator == (const Set<T> & inOperand) const {
  bool equal = mCount == inOperand.mCount ;
  for (size_t i=0 ; (i<mCount) && equal ; i++) {
    equal = mArray [i] == inOperand.mArray [i] ;
  }
  return equal ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

template <typename T> T Set<T>::valueAtIndex (const size_t inIndex) const {
  return mArray [inIndex] ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
