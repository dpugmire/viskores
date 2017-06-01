//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 Sandia Corporation.
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#include <vtkm/CellShape.h>

#include <vtkm/cont/CellSetStructured.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/DataSetFieldAdd.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DynamicArrayHandle.h>
#include <vtkm/VectorAnalysis.h>

#include <vtkm/cont/serial/DeviceAdapterSerial.h>
#include <vtkm/cont/MultiBlock.h>
#include <vtkm/exec/ConnectivityStructured.h>

#include <vtkm/cont/testing/Testing.h>
#include <vtkm/cont/testing/MakeTestDataSet.h>

#include <vtkm/worklet/DispatcherMapTopology.h>
#include <vtkm/worklet/WorkletMapTopology.h>
#include <vtkm/worklet/FieldStatistics.h>

#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>
#include <vtkm/worklet/AverageByKey.h>
#include <vtkm/filter/FilterField.h>
namespace vtkm {
namespace filter {

class DivideField : public vtkm::filter::FilterField<DivideField>
{
public:
  VTKM_CONT
  DivideField()
  {}

  VTKM_CONT
  void SetDividerValue(vtkm::Id value){ this->DividerValue = value; }

  template<typename T, typename StorageType, typename DerivedPolicy, typename DeviceAdapter>
  VTKM_CONT
  vtkm::filter::ResultField DoExecute(const vtkm::cont::DataSet& input,
                                        const vtkm::cont::ArrayHandle<T, StorageType>& fieldata,
                                        const vtkm::filter::FieldMetadata& fieldMeta,
                                        const vtkm::filter::PolicyBase<DerivedPolicy>& policy,
                                        const DeviceAdapter& tag)
  { 
    vtkm::cont::Field output;
    output.SetData(fieldata);
    typedef vtkm::cont::ArrayHandleConstant<vtkm::Id> ConstIdArray;
    ConstIdArray constArray(this->DividerValue, fieldata.GetNumberOfValues());
    vtkm::worklet::DispatcherMapField<vtkm::worklet::DivideWorklet> dispatcher;
    vtkm::worklet::DispatcherMapField<vtkm::worklet::FieldStatistics<vtkm::Float64, VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::SubtractConst> dispatcher2(vtkm::worklet::FieldStatistics<vtkm::Float64, VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::SubtractConst(0.5));
    //dispatcher.Invoke(fieldata,constArray,output); 
    dispatcher2.Invoke(fieldata,output);
    return vtkm::filter::ResultField(input,output.GetData(),std::string("pointvar"),vtkm::cont::Field::ASSOC_POINTS);
  }
private:
  vtkm::Id DividerValue;
};

template<>
class FilterTraits<DivideField>
{ //currently the Clip filter only works on scalar data.
public:
  typedef TypeListTagScalarAll InputFieldTypeList;
};


}
}

const std::vector<vtkm::filter::ResultField> MultiBlock_WorkletTest();

void TestMultiBlock_Worklet()
{
  std::cout << std::endl;
  std::cout << "--TestDataSet Uniform and Rectilinear--" << std::endl << std::endl;
  std::vector<vtkm::filter::ResultField> results=MultiBlock_WorkletTest();
  for(std::size_t j=0; j<results.size(); j++)
  {
    results[j].GetField().PrintSummary(std::cout);
    for(std::size_t i=0; i<results[j].GetField().GetData().GetNumberOfValues(); i++)
    { 
      vtkm::cont::ArrayHandle<vtkm::Float64, vtkm::cont::StorageTagBasic> array;
      results[j].GetField().GetData().CopyTo(array);
      //VTKM_TEST_ASSERT(array.GetPortalConstControl().Get(i) == vtkm::Float64(j/2.0), "result incorrect");
    }

  }
}

/*namespace vtkm {
namespace worklet {

class Threshold : public vtkm::worklet::WorkletMapField
{
public:
  typedef void ControlSignature(FieldIn<Scalar> InputField, FieldOut<Scalar> FilterResult);
  typedef void ExecutionSignature (_1 , _2);
  //typedef _1 InputDomain;

  template <typename T,typename H>
  VTKM_EXEC
  void operator()( const T Input,  H Out) const
  {  
    if(Input > 5)
    {   Out=1 ; }
    else
    {   Out=0 ;}
    return ;
  }
};

}
}*/
template <typename T>
vtkm::cont::MultiBlock UniformMultiBlockBuilder()
{
  vtkm::cont::DataSetBuilderUniform dataSetBuilder;
  vtkm::cont::DataSet dataSet;
  vtkm::cont::DataSetFieldAdd dsf;
  vtkm::Vec<T,3> origin(0);
  vtkm::Vec<T,3> spacing(1);
  vtkm::cont::MultiBlock Blocks;
  for (vtkm::Id trial = 0; trial < 7; trial++)
  {
    vtkm::Id3 dimensions(10, 10, 10);
    vtkm::Id numPoints = dimensions[0] * dimensions[1];
    vtkm::Id numCells = (dimensions[0]-1) * (dimensions[1]-1);
    std::vector<T> varP2D(static_cast<std::size_t>(numPoints));
    for (std::size_t i = 0; i < static_cast<std::size_t>(numPoints); i++)
    {
      varP2D[i] = static_cast<T>(trial);
    }
    std::vector<T> varC2D(static_cast<std::size_t>(numCells));
    for (std::size_t i = 0; i < static_cast<std::size_t>(numCells); i++)
    {
      varC2D[i] = static_cast<T>(trial);
    }
    dataSet = dataSetBuilder.Create(vtkm::Id2(dimensions[0], dimensions[1]),
                                    vtkm::Vec<T,2>(origin[0], origin[1]),
                                    vtkm::Vec<T,2>(spacing[0], spacing[1]));
    dsf.AddPointField(dataSet, "pointvar", varP2D);
    dsf.AddCellField(dataSet, "cellvar", varC2D);
    Blocks.AddBlock(dataSet);
  }
  return Blocks;
}

template<typename FilterType, typename SpecsType>
std::vector<vtkm::filter::ResultField> Apply(vtkm::cont::MultiBlock MB, FilterType filter, SpecsType specs)
{
  std::vector<vtkm::filter::ResultField> results;
  for(std::size_t j=0; j<MB.GetNumberOfBlocks(); j++)
  {
    vtkm::filter::ResultField result = filter.Execute(MB.GetBlock(j), std::string(specs));
    results.push_back(result);
  }

  return results;
}


const std::vector<vtkm::filter::ResultField> MultiBlock_WorkletTest()
{
  vtkm::cont::DynamicArrayHandle output;
  
  vtkm::cont::testing::MakeTestDataSet testDataSet;
  vtkm::cont::MultiBlock Blocks=UniformMultiBlockBuilder<vtkm::Float64>();
  std::vector<vtkm::filter::ResultField> results;

  vtkm::filter::DivideField divider;
  divider.SetDividerValue(2);
  //results = Apply(Blocks,divider,"pointvar");
  results = divider.Execute(Blocks, std::string("pointvar"));
  /*for(std::size_t j=100; j<Blocks.GetNumberOfBlocks(); j++)
  {   
    divider.SetDividerValue(2);
    vtkm::filter::ResultField result = divider.Execute(Blocks.GetBlock(j), std::string("pointvar"));
    results.push_back(result); 
  }*/

  return results;
}



int UnitTestMultiBlock_Worklet(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(TestMultiBlock_Worklet);
}
