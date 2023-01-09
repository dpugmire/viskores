//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef vtk_m_cont_DataSet_h
#define vtk_m_cont_DataSet_h

#include <vtkm/cont/vtkm_cont_export.h>

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/CoordinateSystem.h>
#include <vtkm/cont/ErrorBadValue.h>
#include <vtkm/cont/Field.h>
#include <vtkm/cont/UnknownArrayHandle.h>
#include <vtkm/cont/UnknownCellSet.h>
#include <vtkm/cont/internal/FieldCollection.h>

namespace vtkm
{
namespace cont
{

VTKM_CONT_EXPORT VTKM_CONT std::string& GlobalGhostCellFieldName() noexcept;

VTKM_CONT_EXPORT VTKM_CONT const std::string& GetGlobalGhostCellFieldName() noexcept;

VTKM_CONT_EXPORT VTKM_CONT void SetGlobalGhostCellFieldName(const std::string& name) noexcept;

class VTKM_CONT_EXPORT DataSet
{
public:
  DataSet() = default;

  DataSet(vtkm::cont::DataSet&&) = default;

  DataSet(const vtkm::cont::DataSet&) = default;

  vtkm::cont::DataSet& operator=(vtkm::cont::DataSet&&) = default;

  vtkm::cont::DataSet& operator=(const vtkm::cont::DataSet&) = default;

  VTKM_CONT void Clear();

  /// \brief Get the number of cells contained in this DataSet
  VTKM_CONT vtkm::Id GetNumberOfCells() const;

  /// \brief Get the number of points contained in this DataSet
  ///
  /// Note: All coordinate systems for a DataSet are expected
  /// to have the same number of points.
  VTKM_CONT vtkm::Id GetNumberOfPoints() const;

  /// \brief Adds a field to the `DataSet`.
  ///
  /// Note that the indexing of fields is not the same as the order in which they are
  /// added, and that adding a field can arbitrarily reorder the integer indexing of
  /// all the fields. To retrieve a specific field, retrieve the field by name, not by
  /// integer index.
  VTKM_CONT void AddField(const Field& field);

  ///@{
  /// \brief Retrieves a field by index.
  ///
  /// Note that the indexing of fields is not the same as the order in which they are
  /// added, and that adding a field can arbitrarily reorder the integer indexing of
  /// all the fields. To retrieve a specific field, retrieve the field by name, not by
  /// integer index. This method is most useful for iterating over all the fields of
  /// a `DataSet` (indexed from `0` to `NumberOfFields() - 1`).
  VTKM_CONT
  const vtkm::cont::Field& GetField(vtkm::Id index) const { return this->Fields.GetField(index); }

  VTKM_CONT
  vtkm::cont::Field& GetField(vtkm::Id index) { return this->Fields.GetField(index); }
  ///@}

  VTKM_CONT
  bool HasField(const std::string& name,
                vtkm::cont::Field::Association assoc = vtkm::cont::Field::Association::Any) const
  {
    return this->Fields.HasField(name, assoc);
  }

  VTKM_CONT
  bool HasCellField(const std::string& name) const
  {
    return (this->Fields.GetFieldIndex(name, vtkm::cont::Field::Association::Cells) != -1);
  }

  VTKM_CONT
  bool HasGhostCellField() const;

  VTKM_CONT
  const std::string& GetGhostCellFieldName() const;

  VTKM_CONT
  bool HasPointField(const std::string& name) const
  {
    return (this->Fields.GetFieldIndex(name, vtkm::cont::Field::Association::Points) != -1);
  }


  /// \brief Returns the field that matches the provided name and association.
  ///
  /// This method will return -1 if no match for the field is found.
  ///
  /// Note that the indexing of fields is not the same as the order in which they are
  /// added, and that adding a field can arbitrarily reorder the integer indexing of
  /// all the fields. To retrieve a specific field, retrieve the field by name, not by
  /// integer index.
  VTKM_CONT
  vtkm::Id GetFieldIndex(
    const std::string& name,
    vtkm::cont::Field::Association assoc = vtkm::cont::Field::Association::Any) const
  {
    return this->Fields.GetFieldIndex(name, assoc);
  }

  /// \brief Returns the field that matches the provided name and association.
  ///
  /// This method will throw an exception if no match is found. Use `HasField()` to query
  /// whether a particular field exists.
  ///@{
  VTKM_CONT
  const vtkm::cont::Field& GetField(
    const std::string& name,
    vtkm::cont::Field::Association assoc = vtkm::cont::Field::Association::Any) const
  {
    return this->Fields.GetField(name, assoc);
  }


  VTKM_CONT
  vtkm::cont::Field& GetField(
    const std::string& name,
    vtkm::cont::Field::Association assoc = vtkm::cont::Field::Association::Any)
  {
    return this->Fields.GetField(name, assoc);
  }
  ///@}

  /// \brief Returns the first cell field that matches the provided name.
  ///
  /// This method will throw an exception if no match is found. Use `HasCellField()` to query
  /// whether a particular field exists.
  ///@{
  VTKM_CONT
  const vtkm::cont::Field& GetCellField(const std::string& name) const
  {
    return this->GetField(name, vtkm::cont::Field::Association::Cells);
  }

  VTKM_CONT
  vtkm::cont::Field& GetCellField(const std::string& name)
  {
    return this->GetField(name, vtkm::cont::Field::Association::Cells);
  }
  ///@}

  /// \brief Returns the cell field that matches the ghost cell field name.
  ///
  /// This method will throw an exception if no match is found. Use `HasGhostCellField()` to query
  /// whether a particular field exists.
  ///@{
  VTKM_CONT
  const vtkm::cont::Field& GetGhostCellField() const;
  ///@}

  /// \brief Returns the first point field that matches the provided name.
  ///
  /// This method will throw an exception if no match is found. Use `HasPointField()` to query
  /// whether a particular field exists.
  ///@{
  VTKM_CONT
  const vtkm::cont::Field& GetPointField(const std::string& name) const
  {
    return this->GetField(name, vtkm::cont::Field::Association::Points);
  }

  VTKM_CONT
  vtkm::cont::Field& GetPointField(const std::string& name)
  {
    return this->GetField(name, vtkm::cont::Field::Association::Points);
  }
  ///@}

  ///@{
  /// \brief Adds a point field of a given name to the `DataSet`.
  ///
  /// Note that the indexing of fields is not the same as the order in which they are
  /// added, and that adding a field can arbitrarily reorder the integer indexing of
  /// all the fields. To retrieve a specific field, retrieve the field by name, not by
  /// integer index.
  VTKM_CONT
  void AddPointField(const std::string& fieldName, const vtkm::cont::UnknownArrayHandle& field)
  {
    this->AddField(make_FieldPoint(fieldName, field));
  }

  template <typename T, typename Storage>
  VTKM_CONT void AddPointField(const std::string& fieldName,
                               const vtkm::cont::ArrayHandle<T, Storage>& field)
  {
    this->AddField(make_FieldPoint(fieldName, field));
  }

  template <typename T>
  VTKM_CONT void AddPointField(const std::string& fieldName, const std::vector<T>& field)
  {
    this->AddField(
      make_Field(fieldName, vtkm::cont::Field::Association::Points, field, vtkm::CopyFlag::On));
  }

  template <typename T>
  VTKM_CONT void AddPointField(const std::string& fieldName, const T* field, const vtkm::Id& n)
  {
    this->AddField(
      make_Field(fieldName, vtkm::cont::Field::Association::Points, field, n, vtkm::CopyFlag::On));
  }
  ///@}

  ///@{
  /// \brief Adds a cell field of a given name to the `DataSet`.
  ///
  /// Note that the indexing of fields is not the same as the order in which they are
  /// added, and that adding a field can arbitrarily reorder the integer indexing of
  /// all the fields. To retrieve a specific field, retrieve the field by name, not by
  /// integer index.
  VTKM_CONT
  void AddCellField(const std::string& fieldName, const vtkm::cont::UnknownArrayHandle& field)
  {
    this->AddField(make_FieldCell(fieldName, field));
  }

  template <typename T, typename Storage>
  VTKM_CONT void AddCellField(const std::string& fieldName,
                              const vtkm::cont::ArrayHandle<T, Storage>& field)
  {
    this->AddField(make_FieldCell(fieldName, field));
  }

  template <typename T>
  VTKM_CONT void AddCellField(const std::string& fieldName, const std::vector<T>& field)
  {
    this->AddField(
      make_Field(fieldName, vtkm::cont::Field::Association::Cells, field, vtkm::CopyFlag::On));
  }

  template <typename T>
  VTKM_CONT void AddCellField(const std::string& fieldName, const T* field, const vtkm::Id& n)
  {
    this->AddField(
      make_Field(fieldName, vtkm::cont::Field::Association::Cells, field, n, vtkm::CopyFlag::On));
  }
  ///@}

  /// \brief Sets the name of the field to use for cell ghost levels.
  ///
  /// This value can be set regardless of whether such a cell field actually exists.
  VTKM_CONT void SetGhostCellFieldName(const std::string& name);

  /// \brief Sets the cell field of the given name as the cell ghost levels.
  ///
  /// If a cell field of the given name does not exist, an exception is thrown.
  VTKM_CONT void SetGhostCellField(const std::string& name);

  ///@{
  /// \brief Sets the ghost cell levels.
  ///
  /// A field of the given name is added to the `DataSet`, and that field is set as the cell
  /// ghost levels.
  ///
  /// Note that the indexing of fields is not the same as the order in which they are
  /// added, and that adding a field can arbitrarily reorder the integer indexing of
  /// all the fields. To retrieve a specific field, retrieve the field by name, not by
  /// integer index.
  VTKM_CONT void SetGhostCellField(const vtkm::cont::Field& field);
  VTKM_CONT void SetGhostCellField(const std::string& fieldName,
                                   const vtkm::cont::UnknownArrayHandle& field);
  ///@}

  /// \brief Sets the ghost cell levels to the given array.
  ///
  /// A field with the global ghost cell field name (see `GlobalGhostCellFieldName`) is added
  /// to the `DataSet` and made to be the cell ghost levels.
  ///
  /// Note that the indexing of fields is not the same as the order in which they are
  /// added, and that adding a field can arbitrarily reorder the integer indexing of
  /// all the fields. To retrieve a specific field, retrieve the field by name, not by
  /// integer index.
  VTKM_CONT void SetGhostCellField(const vtkm::cont::UnknownArrayHandle& field);

  VTKM_DEPRECATED(2.0, "Use SetGhostCellField.")
  VTKM_CONT
  void AddGhostCellField(const std::string& fieldName, const vtkm::cont::UnknownArrayHandle& field)
  {
    this->SetGhostCellField(fieldName, field);
  }

  VTKM_DEPRECATED(2.0, "Use SetGhostCellField.")
  VTKM_CONT
  void AddGhostCellField(const vtkm::cont::UnknownArrayHandle& field)
  {
    this->SetGhostCellField(field);
  }

  VTKM_DEPRECATED(2.0, "Use SetGhostCellField.")
  VTKM_CONT
  void AddGhostCellField(const vtkm::cont::Field& field) { this->SetGhostCellField(field); }


  /// \brief Adds the given `CoordinateSystem` to the `DataSet`.
  ///
  /// The coordinate system will also be added as a point field of the same name.
  ///
  /// \returns the index assigned to the added coordinate system.
  VTKM_CONT
  vtkm::IdComponent AddCoordinateSystem(const vtkm::cont::CoordinateSystem& cs);

  /// \brief Marks the point field with the given name as a coordinate system.
  ///
  /// If no such point field exists or the point field is of the wrong format, an exception
  /// will be throw.
  ///
  /// \returns the index assigned to the added coordinate system.
  VTKM_CONT vtkm::IdComponent AddCoordinateSystem(const std::string& pointFieldName);

  VTKM_CONT
  bool HasCoordinateSystem(const std::string& name) const
  {
    return this->GetCoordinateSystemIndex(name) >= 0;
  }

  VTKM_CONT
  vtkm::cont::CoordinateSystem GetCoordinateSystem(vtkm::Id index = 0) const;

  /// Returns the index for the CoordinateSystem whose
  /// name matches the provided string.
  /// Will return -1 if no match is found
  VTKM_CONT
  vtkm::IdComponent GetCoordinateSystemIndex(const std::string& name) const;

  VTKM_CONT const std::string& GetCoordinateSystemName(vtkm::Id index = 0) const;

  /// Returns the CoordinateSystem that matches the provided name.
  /// Will throw an exception if no match is found
  VTKM_CONT
  vtkm::cont::CoordinateSystem GetCoordinateSystem(const std::string& name) const;

  template <typename CellSetType>
  VTKM_CONT void SetCellSet(const CellSetType& cellSet)
  {
    VTKM_IS_KNOWN_OR_UNKNOWN_CELL_SET(CellSetType);
    this->SetCellSetImpl(cellSet);
  }

  VTKM_CONT
  const vtkm::cont::UnknownCellSet& GetCellSet() const { return this->CellSet; }

  VTKM_CONT
  vtkm::cont::UnknownCellSet& GetCellSet() { return this->CellSet; }

  VTKM_CONT
  vtkm::IdComponent GetNumberOfFields() const { return this->Fields.GetNumberOfFields(); }

  VTKM_CONT
  vtkm::IdComponent GetNumberOfCoordinateSystems() const
  {
    return static_cast<vtkm::IdComponent>(this->CoordSystemNames.size());
  }

  /// Copies the structure from the source dataset. The structure includes the cellset,
  /// the coordinate systems, and any ghost layer information. The fields that are not
  /// part of a coordinate system or ghost layers are left unchanged.
  VTKM_CONT
  void CopyStructure(const vtkm::cont::DataSet& source);

  /// \brief Convert the structures in this data set to expected types.
  ///
  /// A `DataSet` object can contain data structures of unknown types. Using the data
  /// requires casting these data structures to concrete types. It is only possible to
  /// check a finite number of data structures.
  ///
  /// The types checked by default are listed in `vtkm/cont/DefaultTypes.h`, which can
  /// be configured at compile time. If a `DataSet` contains data not listed there, then
  /// it is likely going to cause problems pulling the data back out. To get around this
  /// problem, you can call this method to convert the data to a form that is likely to
  /// be recognized. This conversion is likely but not guaranteed because not all types
  /// are convertable to something recognizable.
  ///
  VTKM_CONT void ConvertToExpected();

  VTKM_CONT
  void PrintSummary(std::ostream& out) const;

private:
  std::vector<std::string> CoordSystemNames;
  vtkm::cont::internal::FieldCollection Fields{ vtkm::cont::Field::Association::WholeDataSet,
                                                vtkm::cont::Field::Association::Points,
                                                vtkm::cont::Field::Association::Cells };

  vtkm::cont::UnknownCellSet CellSet;
  std::shared_ptr<std::string> GhostCellName;

  VTKM_CONT void SetCellSetImpl(const vtkm::cont::UnknownCellSet& cellSet);
};

} // namespace cont
} // namespace vtkm

//=============================================================================
// Specializations of serialization related classes
/// @cond SERIALIZATION
namespace vtkm
{
namespace cont
{

template <typename FieldTypeList = VTKM_DEFAULT_TYPE_LIST,
          typename CellSetTypesList = VTKM_DEFAULT_CELL_SET_LIST>
struct SerializableDataSet
{
  SerializableDataSet() = default;

  explicit SerializableDataSet(const vtkm::cont::DataSet& dataset)
    : DataSet(dataset)
  {
  }

  vtkm::cont::DataSet DataSet;
};
}
} // vtkm::cont

namespace mangled_diy_namespace
{

template <typename FieldTypeList, typename CellSetTypesList>
struct Serialization<vtkm::cont::SerializableDataSet<FieldTypeList, CellSetTypesList>>
{
private:
  using Type = vtkm::cont::SerializableDataSet<FieldTypeList, CellSetTypesList>;

public:
  static VTKM_CONT void save(BinaryBuffer& bb, const Type& serializable)
  {
    const auto& dataset = serializable.DataSet;

    vtkmdiy::save(bb, dataset.GetCellSet().ResetCellSetList(CellSetTypesList{}));

    vtkm::IdComponent numberOfFields = dataset.GetNumberOfFields();
    vtkmdiy::save(bb, numberOfFields);
    for (vtkm::IdComponent i = 0; i < numberOfFields; ++i)
    {
      vtkmdiy::save(bb, dataset.GetField(i));
    }

    vtkm::IdComponent numberOfCoordinateSystems = dataset.GetNumberOfCoordinateSystems();
    vtkmdiy::save(bb, numberOfCoordinateSystems);
    for (vtkm::IdComponent i = 0; i < numberOfCoordinateSystems; ++i)
    {
      vtkmdiy::save(bb, dataset.GetCoordinateSystemName(i));
    }
  }

  static VTKM_CONT void load(BinaryBuffer& bb, Type& serializable)
  {
    auto& dataset = serializable.DataSet;
    dataset = {}; // clear

    vtkm::cont::UncertainCellSet<CellSetTypesList> cells;
    vtkmdiy::load(bb, cells);
    dataset.SetCellSet(cells);

    vtkm::IdComponent numberOfFields = 0;
    vtkmdiy::load(bb, numberOfFields);
    for (vtkm::IdComponent i = 0; i < numberOfFields; ++i)
    {
      vtkm::cont::Field field;
      vtkmdiy::load(bb, field);
      dataset.AddField(field);
    }

    vtkm::IdComponent numberOfCoordinateSystems = 0;
    vtkmdiy::load(bb, numberOfCoordinateSystems);
    for (vtkm::IdComponent i = 0; i < numberOfCoordinateSystems; ++i)
    {
      std::string coordName;
      vtkmdiy::load(bb, coordName);
      dataset.AddCoordinateSystem(coordName);
    }
  }
};

} // diy
/// @endcond SERIALIZATION

#endif //vtk_m_cont_DataSet_h
