//============================================================================
//  The contents of this file are covered by the Viskores license. See
//  LICENSE.txt for details.
//
//  By contributing to this file, all contributors agree to the Developer
//  Certificate of Origin Version 1.1 (DCO 1.1) as stated in DCO.txt.
//============================================================================

//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#include <viskores/cont/Initialize.h>

#include <viskores/io/VTKDataSetReader.h>
#include <viskores/io/VTKDataSetWriter.h>

#include <viskores/filter/geometry_refinement/Triangulate.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[])
{
  viskores::cont::Initialize(argc, argv);

  if ((argc < 2) || (argc > 3))
  {
    std::cerr << "Usage: " << argv[0] << " in_data.vtk [out_data.vtk]\n\n";
    std::cerr << "For example, you could use the vanc.vtk that comes with the Viskores source:\n\n";
    std::cerr << "  " << argv[0] << " <path-to-viskores-source>/data/data/rectilinear/vanc.vtk\n";
    return 1;
  }
  std::string infilename = argv[1];
  std::string outfilename = "out_tris.vtk";
  if (argc == 3)
  {
    outfilename = argv[2];
  }

  viskores::io::VTKDataSetReader reader(infilename);
  viskores::cont::DataSet input = reader.ReadDataSet();

  viskores::filter::geometry_refinement::Triangulate triangulateFilter;
  viskores::cont::DataSet output = triangulateFilter.Execute(input);

  viskores::io::VTKDataSetWriter writer(outfilename);
  writer.WriteDataSet(output);

  return 0;
}
