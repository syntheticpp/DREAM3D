

#include <stdlib.h>


#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/ManagedArrayOfArrays.hpp"

int main(int argc, char** argv)
{



  ManagedArrayOfArrays<int>::Pointer array = ManagedArrayOfArrays<int>::CreateArray(10, "test");

  {
    ManagedArrayOfArrays<int>::Data_t& ptr = *(array->GetPointer(0));
    assert(ptr.count == 0);
    assert(ptr.data == NULL);

    ptr.count = 10;
    ptr.data = (int*)(malloc(sizeof(int) * ptr.count));
  }


  {
    ManagedArrayOfArrays<int>::Data_t& ptr = *(array->GetPointer(0));
    assert(ptr.count == 10);
    assert(ptr.data != NULL);


  }

  return EXIT_SUCCESS;
}