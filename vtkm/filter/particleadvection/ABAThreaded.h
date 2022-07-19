//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#ifndef vtk_m_filter_ABAThreaded_h
#define vtk_m_filter_ABAThreaded_h

#include <vtkm/cont/PartitionedDataSet.h>
#include <vtkm/filter/particleadvection/ABA.h>
#include <vtkm/filter/particleadvection/BoundsMap.h>
#include <vtkm/filter/particleadvection/DSI.h>
#include <vtkm/filter/particleadvection/ParticleMessenger.h>

#include <thread>

namespace vtkm
{
namespace filter
{
namespace particleadvection
{

template <typename DSIType, template <typename> class ResultType, typename ParticleType>
class ABAThreaded : public ABA<DSIType, ResultType, ParticleType>
{
public:
  ABAThreaded(const vtkm::filter::particleadvection::BoundsMap& bm, std::vector<DSIType*>& blocks)
    : ABA<DSIType, ResultType, ParticleType>(bm, blocks)
    , Done(false)
    , WorkerActivate(false)
  {
    //For threaded algorithm, the particles go out of scope in the Work method.
    //When this happens, they are destructed by the time the Manage thread gets them.
    //Set the copy flag so the std::vector is copied into the ArrayHandle
    for (auto block : this->Blocks)
      block->SetCopySeedFlag(true);
  }

  void Go() override
  {
    vtkm::Id nLocal = static_cast<vtkm::Id>(this->Active.size() + this->Inactive.size());
    this->ComputeTotalNumParticles(nLocal);

    std::vector<std::thread> workerThreads;
    workerThreads.push_back(std::thread(ABAThreaded::Worker, this));
    this->Manage();

    //This will only work for 1 thread. For > 1, the Blocks will need a mutex.
    VTKM_ASSERT(workerThreads.size() == 1);
    for (auto& t : workerThreads)
      t.join();
  }

protected:
  bool GetActiveParticles(std::vector<ParticleType>& particles, vtkm::Id& blockId) override
  {
    std::lock_guard<std::mutex> lock(this->Mutex);
    bool val = this->ABA<DSIType, ResultType, ParticleType>::GetActiveParticles(particles, blockId);
    this->WorkerActivate = val;
    return val;
  }

  void UpdateActive(const std::vector<ParticleType>& particles,
                    const std::unordered_map<vtkm::Id, std::vector<vtkm::Id>>& idsMap) override
  {
    if (!particles.empty())
    {
      std::lock_guard<std::mutex> lock(this->Mutex);
      this->ABA<DSIType, ResultType, ParticleType>::UpdateActive(particles, idsMap);

      //Let workers know there is new work
      this->WorkerActivateCondition.notify_all();
      this->WorkerActivate = true;
    }
  }

  bool CheckDone()
  {
    std::lock_guard<std::mutex> lock(this->Mutex);
    return this->Done;
  }

  void SetDone()
  {
    std::lock_guard<std::mutex> lock(this->Mutex);
    this->Done = true;
    this->WorkerActivateCondition.notify_all();
  }

  static void Worker(ABAThreaded* algo) { algo->Work(); }

  void WorkerWait()
  {
    std::unique_lock<std::mutex> lock(this->Mutex);
    this->WorkerActivateCondition.wait(lock, [this] { return WorkerActivate || Done; });
  }

  void UpdateWorkerResult(vtkm::Id blockId, DSIHelperInfoType& b)
  {
    std::lock_guard<std::mutex> lock(this->Mutex);
    auto& it = this->WorkerResults[blockId];
    it.push_back(b);
  }

  void Work()
  {
    while (!this->CheckDone())
    {
      std::vector<ParticleType> v;
      vtkm::Id blockId = -1;
      if (this->GetActiveParticles(v, blockId))
      {
        const auto& block = this->GetDataSet(blockId);
        DSIHelperInfoType bb =
          DSIHelperInfo<ParticleType>(v, this->BoundsMap, this->ParticleBlockIDsMap);
        block->Advect(bb, this->StepSize, this->NumberOfSteps);
        this->UpdateWorkerResult(blockId, bb);
      }
      else
        this->WorkerWait();
    }
  }

  void Manage()
  {
    vtkm::filter::particleadvection::ParticleMessenger<ParticleType> messenger(
      this->Comm, this->BoundsMap, 1, 128);

    while (this->TotalNumTerminatedParticles < this->TotalNumParticles)
    {
      std::unordered_map<vtkm::Id, std::vector<DSIHelperInfoType>> workerResults;
      this->GetWorkerResults(workerResults);

      vtkm::Id numTerm = 0;
      for (auto& it : workerResults)
      {
        for (auto& r : it.second)
          numTerm += this->UpdateResult(r.Get<DSIHelperInfo<ParticleType>>());
      }

      vtkm::Id numTermMessages = 0;
      this->Communicate(messenger, numTerm, numTermMessages);

      this->TotalNumTerminatedParticles += (numTerm + numTermMessages);
      if (this->TotalNumTerminatedParticles > this->TotalNumParticles)
        throw vtkm::cont::ErrorFilterExecution("Particle count error");
    }

    //Let the workers know that we are done.
    this->SetDone();
  }

  bool GetBlockAndWait(const vtkm::Id& numLocalTerm) override
  {
    std::lock_guard<std::mutex> lock(this->Mutex);

    return (this->ABA<DSIType, ResultType, ParticleType>::GetBlockAndWait(numLocalTerm) &&
            !this->WorkerActivate && this->WorkerResults.empty());
  }

  void GetWorkerResults(std::unordered_map<vtkm::Id, std::vector<DSIHelperInfoType>>& results)
  {
    results.clear();

    std::lock_guard<std::mutex> lock(this->Mutex);
    if (!this->WorkerResults.empty())
    {
      results = this->WorkerResults;
      this->WorkerResults.clear();
    }
  }

  std::atomic<bool> Done;
  std::mutex Mutex;
  bool WorkerActivate;
  std::condition_variable WorkerActivateCondition;
  std::unordered_map<vtkm::Id, std::vector<DSIHelperInfoType>> WorkerResults;
};

}
}
}

//#include <vtkm/filter/particleadvection/ABA.hxx>

#endif //vtk_m_filter_ABAThreaded_h
