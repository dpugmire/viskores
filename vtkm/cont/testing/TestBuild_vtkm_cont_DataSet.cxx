//mark that we are including headers as test for completeness.
//This is used by headers that include thrust to properly define a proper
//device backend / system
#define VTKM_TEST_HEADER_BUILD

#define BOOST_SP_DISABLE_THREADS

#include <vtkm/cont/DataSet.h>

int Test_Build_For_DataSet()
{
  return 0;
}
