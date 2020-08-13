//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#ifndef vtk_m_filter_ParticleAdvector_h
#define vtk_m_filter_ParticleAdvector_h

#include <vtkm/cont/Algorithm.h>
#include <vtkm/cont/ArrayCopy.h>
#include <vtkm/cont/ArrayHandleTransform.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/ParticleArrayCopy.h>
#include <vtkm/filter/particleadvection/DataSetIntegrator.h>
#include <vtkm/filter/particleadvection/ParticleMessenger.h>

#include <map>
#include <vector>

namespace vtkm
{
namespace filter
{

//
// Base class for particle advector
//

template <typename ResultType>
class VTKM_ALWAYS_EXPORT ParticleAdvectorBase
{
public:
  ParticleAdvectorBase(const vtkm::filter::BoundsMap& bm,
                       const std::vector<vtkm::filter::DataSetIntegrator>& blocks)
    : Blocks(blocks)
    , BoundsMap(bm)
    , NumberOfSteps(0)
    , NumRanks(this->Comm.size())
    , Rank(this->Comm.rank())
    , StepSize(0)
  {
  }

  void SetStepSize(vtkm::FloatDefault stepSize) { this->StepSize = stepSize; }
  void SetNumberOfSteps(vtkm::Id numSteps) { this->NumberOfSteps = numSteps; }

  virtual void SetSeeds(const vtkm::cont::ArrayHandle<vtkm::Massless>& seeds) = 0;
  virtual void Go() = 0;
  virtual vtkm::cont::PartitionedDataSet GetOutput() = 0;

protected:
  virtual bool GetActiveParticles(std::vector<vtkm::Massless>& particles) = 0;

  inline vtkm::Id ComputeTotalNumParticles(vtkm::Id numLocal) const;

  inline const vtkm::filter::DataSetIntegrator& GetDataSet(vtkm::Id id) const;
  virtual void StoreResult(const ResultType& vtkmNotUsed(res), vtkm::Id vtkmNotUsed(blockId)) {}

  inline void UpdateResult(const ResultType& res,
                           vtkm::Id blockId,
                           std::vector<vtkm::Massless>& I,
                           std::vector<vtkm::Massless>& T,
                           std::vector<vtkm::Massless>& A);

  std::vector<vtkm::filter::DataSetIntegrator> Blocks;
  vtkm::filter::BoundsMap BoundsMap;
  vtkmdiy::mpi::communicator Comm = vtkm::cont::EnvironmentTracker::GetCommunicator();
  vtkm::Id NumberOfSteps;
  vtkm::Id NumRanks;
  vtkm::Id Rank;
  vtkm::FloatDefault StepSize;
  std::map<vtkm::Id, std::vector<vtkm::Id>> ParticleBlockIDsMap;
};

//
// Base algorithm for particle advection.
// This is a single-threaded algorithm.
//

template <typename ResultType>
class VTKM_ALWAYS_EXPORT PABaseAlgorithm : public ParticleAdvectorBase<ResultType>
{
public:
  PABaseAlgorithm(const vtkm::filter::BoundsMap& bm,
                  const std::vector<vtkm::filter::DataSetIntegrator>& blocks)
    : ParticleAdvectorBase<ResultType>(bm, blocks)
  {
  }

  void SetSeeds(const vtkm::cont::ArrayHandle<vtkm::Massless>& seeds) override
  {
    this->ParticleBlockIDsMap.clear();

    vtkm::Id n = seeds.GetNumberOfValues();
    auto portal = seeds.ReadPortal();
    for (vtkm::Id i = 0; i < n; i++)
    {
      vtkm::Massless p = portal.Get(i);
      std::vector<vtkm::Id> blockIDs = this->BoundsMap.FindBlocks(p.Pos);
      if (!blockIDs.empty() && this->BoundsMap.FindRank(blockIDs[0]) == this->Rank)
      {
        this->Active.push_back(p);
        this->ParticleBlockIDsMap[p.ID] = blockIDs;
      }
    }
  }

  void Go() override
  {
    vtkm::filter::ParticleMessenger messenger(this->Comm, this->BoundsMap, 1, 128);

    vtkm::Id nLocal = static_cast<vtkm::Id>(this->Active.size() + this->Inactive.size());
    vtkm::Id totalNumSeeds = this->ComputeTotalNumParticles(nLocal);

    vtkm::Id N = 0;
    while (N < totalNumSeeds)
    {
      std::vector<vtkm::Massless> v, I, T, A;

      vtkm::Id blockId = -1;
      if (GetActiveParticles(v))
      {
        blockId = this->ParticleBlockIDsMap[v[0].ID][0];
        auto& block = this->GetDataSet(blockId);
        std::cout << "block_" << blockId << ".Advect() " << v.size() << std::endl;
        ResultType r;
        block.Advect(v, this->StepSize, this->NumberOfSteps, r);

        auto portal = r.Particles.ReadPortal();
        for (int i = 0; i < r.Particles.GetNumberOfValues(); i++)
        {
          std::cout << "    " << i << ": " << portal.Get(i).ID
                    << " Status= " << portal.Get(i).Status << std::endl;
        }
        this->UpdateResult(r, blockId, I, T, A);

        if (!A.empty())
          this->Active.insert(this->Active.end(), A.begin(), A.end());
      }

      std::vector<vtkm::Massless> incoming;
      std::map<vtkm::Id, std::vector<vtkm::Id>> incomingBlockIDsMap;
      vtkm::Id myTerm = static_cast<vtkm::Id>(T.size());
      vtkm::Id numTermMessages = 0;
      messenger.Exchange(
        I, this->ParticleBlockIDsMap, myTerm, incoming, incomingBlockIDsMap, numTermMessages);

      if (!incoming.empty())
      {
        this->Active.insert(this->Active.end(), incoming.begin(), incoming.end());
        for (const auto& it : incomingBlockIDsMap)
          this->ParticleBlockIDsMap[it.first] = it.second;
      }

      if (!T.empty())
      {
        std::cout << "Terminations: " << T.size() << " in block " << blockId << std::endl;
        auto& it = this->Terminated[blockId];
        it.insert(it.end(), T.begin(), T.end());
      }

      N += (myTerm + numTermMessages);
      if (N > totalNumSeeds)
        throw "Particle count error";
    }

    std::cout << "Done: " << this->Rank << " Terminated= " << this->Terminated.size() << std::endl;
  }

protected:
  bool GetActiveParticles(std::vector<vtkm::Massless>& particles) override
  {
    particles.clear();
    if (this->Active.empty())
      return false;

    vtkm::Id workingBlockID = this->ParticleBlockIDsMap[this->Active.front().ID][0];
    auto it = this->Active.begin();
    while (it != this->Active.end())
    {
      auto p = *it;
      if (workingBlockID == this->ParticleBlockIDsMap[p.ID][0])
      {
        particles.push_back(p);
        it = this->Active.erase(it);
      }
      else
        it++;
    }

    return !particles.empty();
  }

protected:
  std::vector<vtkm::Massless> Active;
  std::vector<vtkm::Massless> Inactive;
  std::map<vtkm::Id, std::vector<vtkm::Massless>> Terminated;
};


class VTKM_ALWAYS_EXPORT ParticleAdvectionAlgorithm
  : public PABaseAlgorithm<vtkm::worklet::ParticleAdvectionResult<vtkm::Massless>>
{
public:
  ParticleAdvectionAlgorithm(const vtkm::filter::BoundsMap& bm,
                             const std::vector<vtkm::filter::DataSetIntegrator>& blocks)
    : PABaseAlgorithm<vtkm::worklet::ParticleAdvectionResult<vtkm::Massless>>(bm, blocks)
  {
  }

  vtkm::cont::PartitionedDataSet GetOutput() override
  {
    vtkm::cont::PartitionedDataSet output;

    for (const auto& it : this->Terminated)
    {
      if (it.second.empty())
        continue;

      auto particles = vtkm::cont::make_ArrayHandle(it.second, vtkm::CopyFlag::Off);
      vtkm::cont::ArrayHandle<vtkm::Vec3f> pos;
      vtkm::cont::ParticleArrayCopy(particles, pos);

      vtkm::cont::DataSet ds;
      vtkm::cont::CoordinateSystem outCoords("coordinates", pos);
      ds.AddCoordinateSystem(outCoords);

      //Create vertex cell set
      vtkm::Id numPoints = pos.GetNumberOfValues();
      vtkm::cont::CellSetSingleType<> cells;
      vtkm::cont::ArrayHandleIndex conn(numPoints);
      vtkm::cont::ArrayHandle<vtkm::Id> connectivity;

      vtkm::cont::ArrayCopy(conn, connectivity);
      cells.Fill(numPoints, vtkm::CELL_SHAPE_VERTEX, 1, connectivity);
      ds.SetCellSet(cells);

      output.AppendPartition(ds);
    }

    return output;
  }

protected:
};


class VTKM_ALWAYS_EXPORT StreamlineAlgorithm
  : public PABaseAlgorithm<vtkm::worklet::StreamlineResult<vtkm::Massless>>
{
public:
  StreamlineAlgorithm(const vtkm::filter::BoundsMap& bm,
                      const std::vector<vtkm::filter::DataSetIntegrator>& blocks)
    : PABaseAlgorithm<vtkm::worklet::StreamlineResult<vtkm::Massless>>(bm, blocks)
  {
  }

  vtkm::cont::PartitionedDataSet GetOutput() override
  {
    vtkm::cont::PartitionedDataSet output;

    for (const auto& it : this->Results)
    {
      std::size_t nResults = it.second.size();
      if (nResults == 0)
        continue;

      vtkm::cont::DataSet ds;
      //Easy case with one result.
      if (nResults == 1)
      {
        ds.AddCoordinateSystem(vtkm::cont::CoordinateSystem("coordinates", it.second[0].Positions));
        ds.SetCellSet(it.second[0].PolyLines);
      }
      else
      {
        //Append all the results into one data set.
        vtkm::cont::ArrayHandle<vtkm::Vec3f> appendPts;
        std::vector<vtkm::Id> posOffsets(nResults);

        const auto& res0 = it.second[0];
        vtkm::Id totalNumCells = res0.PolyLines.GetNumberOfCells();
        vtkm::Id totalNumPts = res0.Positions.GetNumberOfValues();

        posOffsets[0] = 0;
        for (std::size_t i = 1; i < nResults; i++)
        {
          const auto& res = it.second[i];
          posOffsets[i] = totalNumPts;
          totalNumPts += res.Positions.GetNumberOfValues();
          totalNumCells += res.PolyLines.GetNumberOfCells();
        }

        //Append all the points together.
        appendPts.Allocate(totalNumPts);
        for (std::size_t i = 0; i < nResults; i++)
        {
          const auto& res = it.second[i];
          // copy all values into appendPts starting at offset.
          vtkm::cont::Algorithm::CopySubRange(
            res.Positions, 0, res.Positions.GetNumberOfValues(), appendPts, posOffsets[i]);
        }
        vtkm::cont::CoordinateSystem outputCoords =
          vtkm::cont::CoordinateSystem("coordinates", appendPts);
        ds.AddCoordinateSystem(outputCoords);

        //Create polylines.
        std::vector<vtkm::Id> numPtsPerCell(static_cast<std::size_t>(totalNumCells));
        std::size_t off = 0;
        for (std::size_t i = 0; i < nResults; i++)
        {
          const auto& res = it.second[i];
          vtkm::Id nCells = res.PolyLines.GetNumberOfCells();
          for (vtkm::Id j = 0; j < nCells; j++)
            numPtsPerCell[off++] = static_cast<vtkm::Id>(res.PolyLines.GetNumberOfPointsInCell(j));
        }

        auto numPointsPerCellArray =
          vtkm::cont::make_ArrayHandle(numPtsPerCell, vtkm::CopyFlag::Off);

        vtkm::cont::ArrayHandle<vtkm::Id> cellIndex;
        vtkm::Id connectivityLen =
          vtkm::cont::Algorithm::ScanExclusive(numPointsPerCellArray, cellIndex);
        vtkm::cont::ArrayHandleCounting<vtkm::Id> connCount(0, 1, connectivityLen);
        vtkm::cont::ArrayHandle<vtkm::Id> connectivity;
        vtkm::cont::ArrayCopy(connCount, connectivity);

        vtkm::cont::ArrayHandle<vtkm::UInt8> cellTypes;
        auto polyLineShape = vtkm::cont::make_ArrayHandleConstant<vtkm::UInt8>(
          vtkm::CELL_SHAPE_POLY_LINE, totalNumCells);
        vtkm::cont::ArrayCopy(polyLineShape, cellTypes);
        auto offsets = vtkm::cont::ConvertNumIndicesToOffsets(numPointsPerCellArray);

        vtkm::cont::CellSetExplicit<> polyLines;
        polyLines.Fill(totalNumPts, cellTypes, connectivity, offsets);
        ds.SetCellSet(polyLines);
      }

      output.AppendPartition(ds);
    }

    return output;
  }

protected:
  virtual void StoreResult(const vtkm::worklet::StreamlineResult<vtkm::Massless>& res,
                           vtkm::Id blockId) override
  {
    this->Results[blockId].push_back(res);
  }

  std::map<vtkm::Id, std::vector<vtkm::worklet::StreamlineResult<vtkm::Massless>>> Results;
};
}
} // namespace vtkm::filter


#ifndef vtk_m_filter_ParticleAdvector_hxx
#include <vtkm/filter/particleadvection/ParticleAdvector.hxx>
#endif

#endif
