#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayPortal.h>

#include <vtkm/cont/DeviceAdapterAlgorithm.h>

#ifndef VTKM_DEVICE_ADAPTER
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_SERIAL
#endif

typedef VTKM_DEFAULT_DEVICE_ADAPTER_TAG DeviceAdapter;

namespace vtkm {
namespace internal {

template<typename P>
class ArrayPortalStreaming
{
public:
  typedef P PortalType;
  typedef typename PortalType::ValueType ValueType;

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  ArrayPortalStreaming() : InputPortal(), BlockIndex(0), BlockSize(0), CurBlockSize(0) {  }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  ArrayPortalStreaming(const PortalType &inputPortal, vtkm::Id blockIndex, 
                       vtkm::Id blockSize, vtkm::Id curBlockSize) : 
                       InputPortal(inputPortal), BlockIndex(blockIndex), 
                       BlockSize(blockSize), CurBlockSize(curBlockSize) {  }

  VTKM_SUPPRESS_EXEC_WARNINGS
  template<typename OtherP>
  VTKM_EXEC_CONT_EXPORT
  ArrayPortalStreaming(const ArrayPortalStreaming<OtherP> &src) : 
                       InputPortal(src.GetPortal()),
                       BlockIndex(src.GetBlockIndex()),
                       BlockSize(src.GetBlockSize()),
                       CurBlockSize(src.GetCurBlockSize()) {  }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    return this->CurBlockSize; 
  }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  ValueType Get(vtkm::Id index) const {
    return this->InputPortal.Get(BlockIndex*BlockSize + index);
  }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  void Set(vtkm::Id index, const ValueType &value) const {
    this->InputPortal.Set(BlockIndex*BlockSize + index, value);
  }

  VTKM_EXEC_CONT_EXPORT
  const PortalType &GetPortal() const { return this->InputPortal; }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  void SetBlockSize(vtkm::Id blockSize) { BlockSize = blockSize; }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  void SetBlockIndex(vtkm::Id blockIndex) { BlockIndex = blockIndex; }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  void SetCurBlockSize(vtkm::Id curBlockSize) { CurBlockSize = curBlockSize; }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  vtkm::Id GetBlockSize() { return this->BlockSize; }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  vtkm::Id GetBlockIndex() { return this->BlockIndex; }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  vtkm::Id GetCurBlockSize() { return this->CurBlockSize; }

private:
  PortalType InputPortal;
  vtkm::Id BlockSize;
  vtkm::Id BlockIndex;
  vtkm::Id CurBlockSize;
};

}
}

template<typename ArrayHandleInputType>
struct StorageTagStreaming { };

namespace vtkm {
namespace cont {
namespace internal {

template<typename ArrayHandleInputType>
class Storage<
    typename ArrayHandleInputType::ValueType,
    StorageTagStreaming<ArrayHandleInputType> >
{
public:
  typedef typename ArrayHandleInputType::ValueType ValueType;

  typedef vtkm::internal::ArrayPortalStreaming<
      typename ArrayHandleInputType::PortalControl> PortalType;
  typedef vtkm::internal::ArrayPortalStreaming<
      typename ArrayHandleInputType::PortalConstControl> PortalConstType;

  VTKM_CONT_EXPORT
  Storage() : Valid(false) { }

  VTKM_CONT_EXPORT
  Storage(const ArrayHandleInputType inputArray, vtkm::Id blockSize, 
          vtkm::Id blockIndex, vtkm::Id curBlockSize) : 
          InputArray(inputArray), BlockSize(blockSize), 
          BlockIndex(blockIndex), CurBlockSize(curBlockSize), Valid(true) { }

  VTKM_CONT_EXPORT
  PortalType GetPortal() {
    VTKM_ASSERT(this->Valid);
    return PortalType(this->InputArray.GetPortalControl(), 
        BlockSize, BlockIndex, CurBlockSize);
  }

  VTKM_CONT_EXPORT
  PortalConstType GetPortalConst() const {
    VTKM_ASSERT(this->Valid);
    return PortalConstType(this->InputArray.GetPortalConstControl(), 
        BlockSize, BlockIndex, CurBlockSize);
  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    VTKM_ASSERT(this->Valid);
    return CurBlockSize; 
  }

  VTKM_CONT_EXPORT
  void Allocate(vtkm::Id numberOfValues) const {
    // Do nothing, since we only allocate a streaming array once at the beginning
  }

  VTKM_CONT_EXPORT
  void AllocateFullArray(vtkm::Id numberOfValues) {
    VTKM_ASSERT(this->Valid);
    this->InputArray.Allocate(numberOfValues);
  }

  VTKM_CONT_EXPORT
  void Shrink(vtkm::Id numberOfValues) {
    VTKM_ASSERT(this->Valid);
    this->InputArray.Shrink(numberOfValues);
  }

  VTKM_CONT_EXPORT
  void ReleaseResources() {
    VTKM_ASSERT(this->Valid);
    this->InputArray.ReleaseResources();
  }

  VTKM_CONT_EXPORT
  const ArrayHandleInputType &GetArray() const {
    VTKM_ASSERT(this->Valid);
    return this->InputArray;
  }

private:
  ArrayHandleInputType InputArray;
  bool Valid;
  vtkm::Id BlockSize;
  vtkm::Id BlockIndex;
  vtkm::Id CurBlockSize;
};

}
}
}

namespace vtkm {
namespace cont {

template<typename ArrayHandleInputType>
class ArrayHandleStreaming
    : public vtkm::cont::ArrayHandle<
        typename ArrayHandleInputType::ValueType,
        StorageTagStreaming<ArrayHandleInputType> >
{
public:
  VTKM_ARRAY_HANDLE_SUBCLASS(
      ArrayHandleStreaming,
      (ArrayHandleStreaming<ArrayHandleInputType>),
      (vtkm::cont::ArrayHandle<
         typename ArrayHandleInputType::ValueType,
         StorageTagStreaming<ArrayHandleInputType> >));

private:
  typedef vtkm::cont::internal::Storage<ValueType,StorageTag> StorageType;

public:
  VTKM_CONT_EXPORT
  ArrayHandleStreaming(const ArrayHandleInputType &inputArray,
                       const vtkm::Id blockIndex, const vtkm::Id blockSize, 
                       const vtkm::Id curBlockSize)
     : Superclass(StorageType(inputArray, blockIndex, blockSize, curBlockSize)) 
  { 
    this->GetPortalConstControl().SetBlockIndex(blockIndex);
    this->GetPortalConstControl().SetBlockSize(blockSize);
    this->GetPortalConstControl().SetCurBlockSize(curBlockSize);
  }

  VTKM_CONT_EXPORT
  void AllocateFullArray(vtkm::Id numberOfValues) {
    this->ReleaseResourcesExecutionInternal();
    this->Internals->ControlArray.AllocateFullArray(numberOfValues);
    this->Internals->ControlArrayValid = true;
  }

};

}
}
