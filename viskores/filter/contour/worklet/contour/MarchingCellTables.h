//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

// **** DO NOT EDIT THIS FILE!!! ****
// This file is automatically generated by VariantDetail.h.in
#ifndef viskores_MarchingCellTables_h
#define viskores_MarchingCellTables_h

#include <viskores/CellShape.h>
#include <viskores/StaticAssert.h>
#include <viskores/Types.h>

#include <viskores/cont/ArrayHandle.h>
#include <viskores/cont/ExecutionObjectBase.h>

namespace viskores
{
namespace worklet
{
namespace marching_cells
{

// Make sure the table indices are the same as Viskores's shape IDs.
VISKORES_STATIC_ASSERT(viskores::CELL_SHAPE_EMPTY == 0);
VISKORES_STATIC_ASSERT(viskores::CELL_SHAPE_VERTEX == 1);
VISKORES_STATIC_ASSERT(viskores::CELL_SHAPE_LINE == 3);
VISKORES_STATIC_ASSERT(viskores::CELL_SHAPE_POLY_LINE == 4);
VISKORES_STATIC_ASSERT(viskores::CELL_SHAPE_TRIANGLE == 5);
VISKORES_STATIC_ASSERT(viskores::CELL_SHAPE_POLYGON == 7);
VISKORES_STATIC_ASSERT(viskores::CELL_SHAPE_QUAD == 9);
VISKORES_STATIC_ASSERT(viskores::CELL_SHAPE_TETRA == 10);
VISKORES_STATIC_ASSERT(viskores::CELL_SHAPE_HEXAHEDRON == 12);
VISKORES_STATIC_ASSERT(viskores::CELL_SHAPE_WEDGE == 13);
VISKORES_STATIC_ASSERT(viskores::CELL_SHAPE_PYRAMID == 14);

VISKORES_EXEC inline viskores::IdComponent GetNumTrianglesOffset(viskores::UInt8 cellType)
{
  // clang-format off
  VISKORES_STATIC_CONSTEXPR_ARRAY viskores::IdComponent offsets[] = {
    0, // CELL_SHAPE_EMPTY
    0, // CELL_SHAPE_VERTEX
    0, // CELL_SHAPE_POLY_VERTEX
    0, // CELL_SHAPE_LINE
    0, // CELL_SHAPE_POLY_LINE
    0, // CELL_SHAPE_TRIANGLE
    0, // CELL_SHAPE_TRIANGLE_STRIP
    0, // CELL_SHAPE_POLYGON
    0, // CELL_SHAPE_PIXEL
    0, // CELL_SHAPE_QUAD
    0, // CELL_SHAPE_TETRA
    16, // CELL_SHAPE_VOXEL
    16, // CELL_SHAPE_HEXAHEDRON
    272, // CELL_SHAPE_WEDGE
    336, // CELL_SHAPE_PYRAMID
    0 // dummy
  };
  // clang-format on

  return offsets[cellType];
}

VISKORES_EXEC inline viskores::UInt8 GetNumTriangles(viskores::UInt8 cellType, viskores::UInt8 caseNumber)
{
  // clang-format off
  VISKORES_STATIC_CONSTEXPR_ARRAY viskores::UInt8 numTriangles[] = {
    // CELL_SHAPE_EMPTY
    // CELL_SHAPE_VERTEX
    // CELL_SHAPE_POLY_VERTEX
    // CELL_SHAPE_LINE
    // CELL_SHAPE_POLY_LINE
    // CELL_SHAPE_TRIANGLE
    // CELL_SHAPE_TRIANGLE_STRIP
    // CELL_SHAPE_POLYGON
    // CELL_SHAPE_PIXEL
    // CELL_SHAPE_QUAD
    // CELL_SHAPE_TETRA
    0, 1, 1, 2, 1, 2, 2, 1, 1, 2, 2, 1, 2, 1, 1, 0,   // cases 0 - 15
    // CELL_SHAPE_VOXEL
    // CELL_SHAPE_HEXAHEDRON
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2,   // cases 0 - 15
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3,   // cases 16 - 31
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3,   // cases 32 - 47
    2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 4, 5, 5, 2,   // cases 48 - 63
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3,   // cases 64 - 79
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4,   // cases 80 - 95
    2, 3, 3, 4, 3, 4, 2, 3, 3, 4, 4, 5, 4, 5, 3, 2,   // cases 96 - 111
    3, 4, 4, 3, 4, 5, 3, 2, 4, 5, 5, 4, 5, 2, 4, 1,   // cases 112 - 127
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3,   // cases 128 - 143
    2, 3, 3, 4, 3, 4, 4, 5, 3, 2, 4, 3, 4, 3, 5, 2,   // cases 144 - 159
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4,   // cases 160 - 175
    3, 4, 4, 3, 4, 5, 5, 4, 4, 3, 5, 2, 5, 4, 2, 1,   // cases 176 - 191
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 2, 3, 3, 2,   // cases 192 - 207
    3, 4, 4, 5, 4, 5, 5, 2, 4, 3, 5, 4, 3, 2, 4, 1,   // cases 208 - 223
    3, 4, 4, 5, 4, 5, 3, 4, 4, 5, 5, 2, 3, 4, 2, 1,   // cases 224 - 239
    2, 3, 3, 2, 3, 4, 2, 1, 3, 2, 4, 1, 2, 1, 1, 0,   // cases 240 - 255
    // CELL_SHAPE_WEDGE
    0, 1, 1, 2, 1, 2, 2, 1, 1, 2, 2, 3, 2, 3, 3, 2,   // cases 0 - 15
    1, 2, 2, 3, 2, 3, 3, 2, 2, 3, 3, 2, 3, 4, 4, 1,   // cases 16 - 31
    1, 2, 2, 3, 2, 3, 3, 2, 2, 3, 3, 4, 3, 2, 4, 1,   // cases 32 - 47
    2, 3, 3, 4, 3, 4, 2, 1, 1, 2, 2, 1, 2, 1, 1, 0,   // cases 48 - 63
    // CELL_SHAPE_PYRAMID
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2,   // cases 0 - 15
    2, 3, 3, 2, 3, 4, 2, 1, 3, 2, 4, 1, 2, 1, 1, 0,   // cases 16 - 31
    0 // dummy
  };
  // clang-format on

  viskores::IdComponent offset = GetNumTrianglesOffset(cellType);
  return numTriangles[offset + caseNumber];
}

VISKORES_EXEC inline viskores::IdComponent GetTriTableOffset(viskores::UInt8 cellType, viskores::UInt8 caseNumber)
{
  // clang-format off
  VISKORES_STATIC_CONSTEXPR_ARRAY viskores::IdComponent triTableOffset[] = {
    // CELL_SHAPE_EMPTY
    // CELL_SHAPE_VERTEX
    // CELL_SHAPE_POLY_VERTEX
    // CELL_SHAPE_LINE
    // CELL_SHAPE_POLY_LINE
    // CELL_SHAPE_TRIANGLE
    // CELL_SHAPE_TRIANGLE_STRIP
    // CELL_SHAPE_POLYGON
    // CELL_SHAPE_PIXEL
    // CELL_SHAPE_QUAD
    // CELL_SHAPE_TETRA
    0, 0, 1, 2, 4, 5, 7, 9,   // cases 0 - 7
    10, 11, 13, 15, 16, 18, 19, 20,   // cases 8 - 15
    // CELL_SHAPE_VOXEL
    // CELL_SHAPE_HEXAHEDRON
    20, 20, 21, 22, 24, 25, 27, 29,   // cases 0 - 7
    32, 33, 35, 37, 40, 42, 45, 48,   // cases 8 - 15
    50, 51, 53, 55, 58, 60, 63, 66,   // cases 16 - 23
    70, 72, 75, 78, 82, 85, 89, 93,   // cases 24 - 31
    96, 97, 99, 101, 104, 106, 109, 112,   // cases 32 - 39
    116, 118, 121, 124, 128, 131, 135, 139,   // cases 40 - 47
    142, 144, 147, 150, 152, 155, 159, 163,   // cases 48 - 55
    166, 169, 173, 177, 180, 184, 189, 194,   // cases 56 - 63
    196, 197, 199, 201, 204, 206, 209, 212,   // cases 64 - 71
    216, 218, 221, 224, 228, 231, 235, 239,   // cases 72 - 79
    242, 244, 247, 250, 254, 257, 261, 265,   // cases 80 - 87
    270, 273, 277, 281, 286, 290, 295, 300,   // cases 88 - 95
    304, 306, 309, 312, 316, 319, 323, 325,   // cases 96 - 103
    328, 331, 335, 339, 344, 348, 353, 356,   // cases 104 - 111
    358, 361, 365, 369, 372, 376, 381, 384,   // cases 112 - 119
    386, 390, 395, 400, 404, 409, 411, 415,   // cases 120 - 127
    416, 417, 419, 421, 424, 426, 429, 432,   // cases 128 - 135
    436, 438, 441, 444, 448, 451, 455, 459,   // cases 136 - 143
    462, 464, 467, 470, 474, 477, 481, 485,   // cases 144 - 151
    490, 493, 495, 499, 502, 506, 509, 514,   // cases 152 - 159
    516, 518, 521, 524, 528, 531, 535, 539,   // cases 160 - 167
    544, 547, 551, 555, 560, 564, 569, 574,   // cases 168 - 175
    578, 581, 585, 589, 592, 596, 601, 606,   // cases 176 - 183
    610, 614, 617, 622, 624, 629, 633, 635,   // cases 184 - 191
    636, 638, 641, 644, 648, 651, 655, 659,   // cases 192 - 199
    664, 667, 671, 675, 680, 682, 685, 688,   // cases 200 - 207
    690, 693, 697, 701, 706, 710, 715, 720,   // cases 208 - 215
    722, 726, 729, 734, 738, 741, 743, 747,   // cases 216 - 223
    748, 751, 755, 759, 764, 768, 773, 776,   // cases 224 - 231
    780, 784, 789, 794, 796, 799, 803, 805,   // cases 232 - 239
    806, 808, 811, 814, 816, 819, 823, 825,   // cases 240 - 247
    826, 829, 831, 835, 836, 838, 839, 840,   // cases 248 - 255
    // CELL_SHAPE_WEDGE
    840, 840, 841, 842, 844, 845, 847, 849,   // cases 0 - 7
    850, 851, 853, 855, 858, 860, 863, 866,   // cases 8 - 15
    868, 869, 871, 873, 876, 878, 881, 884,   // cases 16 - 23
    886, 888, 891, 894, 896, 899, 903, 907,   // cases 24 - 31
    908, 909, 911, 913, 916, 918, 921, 924,   // cases 32 - 39
    926, 928, 931, 934, 938, 941, 943, 947,   // cases 40 - 47
    948, 950, 953, 956, 960, 963, 967, 969,   // cases 48 - 55
    970, 971, 973, 975, 976, 978, 979, 980,   // cases 56 - 63
    // CELL_SHAPE_PYRAMID
    980, 980, 981, 982, 984, 985, 987, 989,   // cases 0 - 7
    992, 993, 995, 997, 1000, 1002, 1005, 1008,   // cases 8 - 15
    1010, 1012, 1015, 1018, 1020, 1023, 1027, 1029,   // cases 16 - 23
    1030, 1033, 1035, 1039, 1040, 1042, 1043, 1044,   // cases 24 - 31
    0 // dummy
  };
  // clang-format on

  viskores::IdComponent offset = GetNumTrianglesOffset(cellType);
  return triTableOffset[offset + caseNumber];
}

VISKORES_EXEC inline const viskores::UInt8* GetTriangleEdges(viskores::UInt8 cellType,
                                                     viskores::UInt8 caseNumber,
                                                     viskores::UInt8 triangleNumber)
{
  // clang-format off
  VISKORES_STATIC_CONSTEXPR_ARRAY viskores::UInt8 triTable[][3] = {
    // CELL_SHAPE_EMPTY
    // CELL_SHAPE_VERTEX
    // CELL_SHAPE_POLY_VERTEX
    // CELL_SHAPE_LINE
    // CELL_SHAPE_POLY_LINE
    // CELL_SHAPE_TRIANGLE
    // CELL_SHAPE_TRIANGLE_STRIP
    // CELL_SHAPE_POLYGON
    // CELL_SHAPE_PIXEL
    // CELL_SHAPE_QUAD
    // CELL_SHAPE_TETRA
    // case 0
    { 0, 3, 2 }, // case 1
    { 0, 1, 4 }, // case 2
    { 1, 4, 2 }, { 2, 4, 3 }, // case 3
    { 1, 2, 5 }, // case 4
    { 0, 3, 5 }, { 0, 5, 1 }, // case 5
    { 0, 2, 5 }, { 0, 5, 4 }, // case 6
    { 5, 4, 3 }, // case 7
    { 3, 4, 5 }, // case 8
    { 4, 5, 0 }, { 5, 2, 0 }, // case 9
    { 1, 5, 0 }, { 5, 3, 0 }, // case 10
    { 5, 2, 1 }, // case 11
    { 3, 4, 2 }, { 2, 4, 1 }, // case 12
    { 4, 1, 0 }, // case 13
    { 2, 3, 0 }, // case 14
    // case 15
    // CELL_SHAPE_VOXEL
    // CELL_SHAPE_HEXAHEDRON
    // case 0
    { 0, 8, 3 }, // case 1
    { 0, 1, 9 }, // case 2
    { 1, 8, 3 }, { 9, 8, 1 }, // case 3
    { 1, 2, 11 }, // case 4
    { 0, 8, 3 }, { 1, 2, 11 }, // case 5
    { 9, 2, 11 }, { 0, 2, 9 }, // case 6
    { 2, 8, 3 }, { 2, 11, 8 }, { 11, 9, 8 }, // case 7
    { 3, 10, 2 }, // case 8
    { 0, 10, 2 }, { 8, 10, 0 }, // case 9
    { 1, 9, 0 }, { 2, 3, 10 }, // case 10
    { 1, 10, 2 }, { 1, 9, 10 }, { 9, 8, 10 }, // case 11
    { 3, 11, 1 }, { 10, 11, 3 }, // case 12
    { 0, 11, 1 }, { 0, 8, 11 }, { 8, 10, 11 }, // case 13
    { 3, 9, 0 }, { 3, 10, 9 }, { 10, 11, 9 }, // case 14
    { 9, 8, 11 }, { 11, 8, 10 }, // case 15
    { 4, 7, 8 }, // case 16
    { 4, 3, 0 }, { 7, 3, 4 }, // case 17
    { 0, 1, 9 }, { 8, 4, 7 }, // case 18
    { 4, 1, 9 }, { 4, 7, 1 }, { 7, 3, 1 }, // case 19
    { 1, 2, 11 }, { 8, 4, 7 }, // case 20
    { 3, 4, 7 }, { 3, 0, 4 }, { 1, 2, 11 }, // case 21
    { 9, 2, 11 }, { 9, 0, 2 }, { 8, 4, 7 }, // case 22
    { 2, 11, 9 }, { 2, 9, 7 }, { 2, 7, 3 }, { 7, 9, 4 }, // case 23
    { 8, 4, 7 }, { 3, 10, 2 }, // case 24
    { 10, 4, 7 }, { 10, 2, 4 }, { 2, 0, 4 }, // case 25
    { 9, 0, 1 }, { 8, 4, 7 }, { 2, 3, 10 }, // case 26
    { 4, 7, 10 }, { 9, 4, 10 }, { 9, 10, 2 }, { 9, 2, 1 }, // case 27
    { 3, 11, 1 }, { 3, 10, 11 }, { 7, 8, 4 }, // case 28
    { 1, 10, 11 }, { 1, 4, 10 }, { 1, 0, 4 }, { 7, 10, 4 }, // case 29
    { 4, 7, 8 }, { 9, 0, 10 }, { 9, 10, 11 }, { 10, 0, 3 }, // case 30
    { 4, 7, 10 }, { 4, 10, 9 }, { 9, 10, 11 }, // case 31
    { 9, 5, 4 }, // case 32
    { 9, 5, 4 }, { 0, 8, 3 }, // case 33
    { 0, 5, 4 }, { 1, 5, 0 }, // case 34
    { 8, 5, 4 }, { 8, 3, 5 }, { 3, 1, 5 }, // case 35
    { 1, 2, 11 }, { 9, 5, 4 }, // case 36
    { 3, 0, 8 }, { 1, 2, 11 }, { 4, 9, 5 }, // case 37
    { 5, 2, 11 }, { 5, 4, 2 }, { 4, 0, 2 }, // case 38
    { 2, 11, 5 }, { 3, 2, 5 }, { 3, 5, 4 }, { 3, 4, 8 }, // case 39
    { 9, 5, 4 }, { 2, 3, 10 }, // case 40
    { 0, 10, 2 }, { 0, 8, 10 }, { 4, 9, 5 }, // case 41
    { 0, 5, 4 }, { 0, 1, 5 }, { 2, 3, 10 }, // case 42
    { 2, 1, 5 }, { 2, 5, 8 }, { 2, 8, 10 }, { 4, 8, 5 }, // case 43
    { 11, 3, 10 }, { 11, 1, 3 }, { 9, 5, 4 }, // case 44
    { 4, 9, 5 }, { 0, 8, 1 }, { 8, 11, 1 }, { 8, 10, 11 }, // case 45
    { 5, 4, 0 }, { 5, 0, 10 }, { 5, 10, 11 }, { 10, 0, 3 }, // case 46
    { 5, 4, 8 }, { 5, 8, 11 }, { 11, 8, 10 }, // case 47
    { 9, 7, 8 }, { 5, 7, 9 }, // case 48
    { 9, 3, 0 }, { 9, 5, 3 }, { 5, 7, 3 }, // case 49
    { 0, 7, 8 }, { 0, 1, 7 }, { 1, 5, 7 }, // case 50
    { 1, 5, 3 }, { 3, 5, 7 }, // case 51
    { 9, 7, 8 }, { 9, 5, 7 }, { 11, 1, 2 }, // case 52
    { 11, 1, 2 }, { 9, 5, 0 }, { 5, 3, 0 }, { 5, 7, 3 }, // case 53
    { 8, 0, 2 }, { 8, 2, 5 }, { 8, 5, 7 }, { 11, 5, 2 }, // case 54
    { 2, 11, 5 }, { 2, 5, 3 }, { 3, 5, 7 }, // case 55
    { 7, 9, 5 }, { 7, 8, 9 }, { 3, 10, 2 }, // case 56
    { 9, 5, 7 }, { 9, 7, 2 }, { 9, 2, 0 }, { 2, 7, 10 }, // case 57
    { 2, 3, 10 }, { 0, 1, 8 }, { 1, 7, 8 }, { 1, 5, 7 }, // case 58
    { 10, 2, 1 }, { 10, 1, 7 }, { 7, 1, 5 }, // case 59
    { 9, 5, 8 }, { 8, 5, 7 }, { 11, 1, 3 }, { 11, 3, 10 }, // case 60
    { 5, 7, 0 }, { 5, 0, 9 }, { 7, 10, 0 }, { 1, 0, 11 }, { 10, 11, 0 }, // case 61
    { 10, 11, 0 }, { 10, 0, 3 }, { 11, 5, 0 }, { 8, 0, 7 }, { 5, 7, 0 }, // case 62
    { 10, 11, 5 }, { 7, 10, 5 }, // case 63
    { 11, 6, 5 }, // case 64
    { 0, 8, 3 }, { 5, 11, 6 }, // case 65
    { 9, 0, 1 }, { 5, 11, 6 }, // case 66
    { 1, 8, 3 }, { 1, 9, 8 }, { 5, 11, 6 }, // case 67
    { 1, 6, 5 }, { 2, 6, 1 }, // case 68
    { 1, 6, 5 }, { 1, 2, 6 }, { 3, 0, 8 }, // case 69
    { 9, 6, 5 }, { 9, 0, 6 }, { 0, 2, 6 }, // case 70
    { 5, 9, 8 }, { 5, 8, 2 }, { 5, 2, 6 }, { 3, 2, 8 }, // case 71
    { 2, 3, 10 }, { 11, 6, 5 }, // case 72
    { 10, 0, 8 }, { 10, 2, 0 }, { 11, 6, 5 }, // case 73
    { 0, 1, 9 }, { 2, 3, 10 }, { 5, 11, 6 }, // case 74
    { 5, 11, 6 }, { 1, 9, 2 }, { 9, 10, 2 }, { 9, 8, 10 }, // case 75
    { 6, 3, 10 }, { 6, 5, 3 }, { 5, 1, 3 }, // case 76
    { 0, 8, 10 }, { 0, 10, 5 }, { 0, 5, 1 }, { 5, 10, 6 }, // case 77
    { 3, 10, 6 }, { 0, 3, 6 }, { 0, 6, 5 }, { 0, 5, 9 }, // case 78
    { 6, 5, 9 }, { 6, 9, 10 }, { 10, 9, 8 }, // case 79
    { 5, 11, 6 }, { 4, 7, 8 }, // case 80
    { 4, 3, 0 }, { 4, 7, 3 }, { 6, 5, 11 }, // case 81
    { 1, 9, 0 }, { 5, 11, 6 }, { 8, 4, 7 }, // case 82
    { 11, 6, 5 }, { 1, 9, 7 }, { 1, 7, 3 }, { 7, 9, 4 }, // case 83
    { 6, 1, 2 }, { 6, 5, 1 }, { 4, 7, 8 }, // case 84
    { 1, 2, 5 }, { 5, 2, 6 }, { 3, 0, 4 }, { 3, 4, 7 }, // case 85
    { 8, 4, 7 }, { 9, 0, 5 }, { 0, 6, 5 }, { 0, 2, 6 }, // case 86
    { 7, 3, 9 }, { 7, 9, 4 }, { 3, 2, 9 }, { 5, 9, 6 }, { 2, 6, 9 }, // case 87
    { 3, 10, 2 }, { 7, 8, 4 }, { 11, 6, 5 }, // case 88
    { 5, 11, 6 }, { 4, 7, 2 }, { 4, 2, 0 }, { 2, 7, 10 }, // case 89
    { 0, 1, 9 }, { 4, 7, 8 }, { 2, 3, 10 }, { 5, 11, 6 }, // case 90
    { 9, 2, 1 }, { 9, 10, 2 }, { 9, 4, 10 }, { 7, 10, 4 }, { 5, 11, 6 }, // case 91
    { 8, 4, 7 }, { 3, 10, 5 }, { 3, 5, 1 }, { 5, 10, 6 }, // case 92
    { 5, 1, 10 }, { 5, 10, 6 }, { 1, 0, 10 }, { 7, 10, 4 }, { 0, 4, 10 }, // case 93
    { 0, 5, 9 }, { 0, 6, 5 }, { 0, 3, 6 }, { 10, 6, 3 }, { 8, 4, 7 }, // case 94
    { 6, 5, 9 }, { 6, 9, 10 }, { 4, 7, 9 }, { 7, 10, 9 }, // case 95
    { 11, 4, 9 }, { 6, 4, 11 }, // case 96
    { 4, 11, 6 }, { 4, 9, 11 }, { 0, 8, 3 }, // case 97
    { 11, 0, 1 }, { 11, 6, 0 }, { 6, 4, 0 }, // case 98
    { 8, 3, 1 }, { 8, 1, 6 }, { 8, 6, 4 }, { 6, 1, 11 }, // case 99
    { 1, 4, 9 }, { 1, 2, 4 }, { 2, 6, 4 }, // case 100
    { 3, 0, 8 }, { 1, 2, 9 }, { 2, 4, 9 }, { 2, 6, 4 }, // case 101
    { 0, 2, 4 }, { 4, 2, 6 }, // case 102
    { 8, 3, 2 }, { 8, 2, 4 }, { 4, 2, 6 }, // case 103
    { 11, 4, 9 }, { 11, 6, 4 }, { 10, 2, 3 }, // case 104
    { 0, 8, 2 }, { 2, 8, 10 }, { 4, 9, 11 }, { 4, 11, 6 }, // case 105
    { 3, 10, 2 }, { 0, 1, 6 }, { 0, 6, 4 }, { 6, 1, 11 }, // case 106
    { 6, 4, 1 }, { 6, 1, 11 }, { 4, 8, 1 }, { 2, 1, 10 }, { 8, 10, 1 }, // case 107
    { 9, 6, 4 }, { 9, 3, 6 }, { 9, 1, 3 }, { 10, 6, 3 }, // case 108
    { 8, 10, 1 }, { 8, 1, 0 }, { 10, 6, 1 }, { 9, 1, 4 }, { 6, 4, 1 }, // case 109
    { 3, 10, 6 }, { 3, 6, 0 }, { 0, 6, 4 }, // case 110
    { 6, 4, 8 }, { 10, 6, 8 }, // case 111
    { 7, 11, 6 }, { 7, 8, 11 }, { 8, 9, 11 }, // case 112
    { 0, 7, 3 }, { 0, 11, 7 }, { 0, 9, 11 }, { 6, 7, 11 }, // case 113
    { 11, 6, 7 }, { 1, 11, 7 }, { 1, 7, 8 }, { 1, 8, 0 }, // case 114
    { 11, 6, 7 }, { 11, 7, 1 }, { 1, 7, 3 }, // case 115
    { 1, 2, 6 }, { 1, 6, 8 }, { 1, 8, 9 }, { 8, 6, 7 }, // case 116
    { 2, 6, 9 }, { 2, 9, 1 }, { 6, 7, 9 }, { 0, 9, 3 }, { 7, 3, 9 }, // case 117
    { 7, 8, 0 }, { 7, 0, 6 }, { 6, 0, 2 }, // case 118
    { 7, 3, 2 }, { 6, 7, 2 }, // case 119
    { 2, 3, 10 }, { 11, 6, 8 }, { 11, 8, 9 }, { 8, 6, 7 }, // case 120
    { 2, 0, 7 }, { 2, 7, 10 }, { 0, 9, 7 }, { 6, 7, 11 }, { 9, 11, 7 }, // case 121
    { 1, 8, 0 }, { 1, 7, 8 }, { 1, 11, 7 }, { 6, 7, 11 }, { 2, 3, 10 }, // case 122
    { 10, 2, 1 }, { 10, 1, 7 }, { 11, 6, 1 }, { 6, 7, 1 }, // case 123
    { 8, 9, 6 }, { 8, 6, 7 }, { 9, 1, 6 }, { 10, 6, 3 }, { 1, 3, 6 }, // case 124
    { 0, 9, 1 }, { 10, 6, 7 }, // case 125
    { 7, 8, 0 }, { 7, 0, 6 }, { 3, 10, 0 }, { 10, 6, 0 }, // case 126
    { 7, 10, 6 }, // case 127
    { 7, 6, 10 }, // case 128
    { 3, 0, 8 }, { 10, 7, 6 }, // case 129
    { 0, 1, 9 }, { 10, 7, 6 }, // case 130
    { 8, 1, 9 }, { 8, 3, 1 }, { 10, 7, 6 }, // case 131
    { 11, 1, 2 }, { 6, 10, 7 }, // case 132
    { 1, 2, 11 }, { 3, 0, 8 }, { 6, 10, 7 }, // case 133
    { 2, 9, 0 }, { 2, 11, 9 }, { 6, 10, 7 }, // case 134
    { 6, 10, 7 }, { 2, 11, 3 }, { 11, 8, 3 }, { 11, 9, 8 }, // case 135
    { 7, 2, 3 }, { 6, 2, 7 }, // case 136
    { 7, 0, 8 }, { 7, 6, 0 }, { 6, 2, 0 }, // case 137
    { 2, 7, 6 }, { 2, 3, 7 }, { 0, 1, 9 }, // case 138
    { 1, 6, 2 }, { 1, 8, 6 }, { 1, 9, 8 }, { 8, 7, 6 }, // case 139
    { 11, 7, 6 }, { 11, 1, 7 }, { 1, 3, 7 }, // case 140
    { 11, 7, 6 }, { 1, 7, 11 }, { 1, 8, 7 }, { 1, 0, 8 }, // case 141
    { 0, 3, 7 }, { 0, 7, 11 }, { 0, 11, 9 }, { 6, 11, 7 }, // case 142
    { 7, 6, 11 }, { 7, 11, 8 }, { 8, 11, 9 }, // case 143
    { 6, 8, 4 }, { 10, 8, 6 }, // case 144
    { 3, 6, 10 }, { 3, 0, 6 }, { 0, 4, 6 }, // case 145
    { 8, 6, 10 }, { 8, 4, 6 }, { 9, 0, 1 }, // case 146
    { 9, 4, 6 }, { 9, 6, 3 }, { 9, 3, 1 }, { 10, 3, 6 }, // case 147
    { 6, 8, 4 }, { 6, 10, 8 }, { 2, 11, 1 }, // case 148
    { 1, 2, 11 }, { 3, 0, 10 }, { 0, 6, 10 }, { 0, 4, 6 }, // case 149
    { 4, 10, 8 }, { 4, 6, 10 }, { 0, 2, 9 }, { 2, 11, 9 }, // case 150
    { 11, 9, 3 }, { 11, 3, 2 }, { 9, 4, 3 }, { 10, 3, 6 }, { 4, 6, 3 }, // case 151
    { 8, 2, 3 }, { 8, 4, 2 }, { 4, 6, 2 }, // case 152
    { 0, 4, 2 }, { 4, 6, 2 }, // case 153
    { 1, 9, 0 }, { 2, 3, 4 }, { 2, 4, 6 }, { 4, 3, 8 }, // case 154
    { 1, 9, 4 }, { 1, 4, 2 }, { 2, 4, 6 }, // case 155
    { 8, 1, 3 }, { 8, 6, 1 }, { 8, 4, 6 }, { 6, 11, 1 }, // case 156
    { 11, 1, 0 }, { 11, 0, 6 }, { 6, 0, 4 }, // case 157
    { 4, 6, 3 }, { 4, 3, 8 }, { 6, 11, 3 }, { 0, 3, 9 }, { 11, 9, 3 }, // case 158
    { 11, 9, 4 }, { 6, 11, 4 }, // case 159
    { 4, 9, 5 }, { 7, 6, 10 }, // case 160
    { 0, 8, 3 }, { 4, 9, 5 }, { 10, 7, 6 }, // case 161
    { 5, 0, 1 }, { 5, 4, 0 }, { 7, 6, 10 }, // case 162
    { 10, 7, 6 }, { 8, 3, 4 }, { 3, 5, 4 }, { 3, 1, 5 }, // case 163
    { 9, 5, 4 }, { 11, 1, 2 }, { 7, 6, 10 }, // case 164
    { 6, 10, 7 }, { 1, 2, 11 }, { 0, 8, 3 }, { 4, 9, 5 }, // case 165
    { 7, 6, 10 }, { 5, 4, 11 }, { 4, 2, 11 }, { 4, 0, 2 }, // case 166
    { 3, 4, 8 }, { 3, 5, 4 }, { 3, 2, 5 }, { 11, 5, 2 }, { 10, 7, 6 }, // case 167
    { 7, 2, 3 }, { 7, 6, 2 }, { 5, 4, 9 }, // case 168
    { 9, 5, 4 }, { 0, 8, 6 }, { 0, 6, 2 }, { 6, 8, 7 }, // case 169
    { 3, 6, 2 }, { 3, 7, 6 }, { 1, 5, 0 }, { 5, 4, 0 }, // case 170
    { 6, 2, 8 }, { 6, 8, 7 }, { 2, 1, 8 }, { 4, 8, 5 }, { 1, 5, 8 }, // case 171
    { 9, 5, 4 }, { 11, 1, 6 }, { 1, 7, 6 }, { 1, 3, 7 }, // case 172
    { 1, 6, 11 }, { 1, 7, 6 }, { 1, 0, 7 }, { 8, 7, 0 }, { 9, 5, 4 }, // case 173
    { 4, 0, 11 }, { 4, 11, 5 }, { 0, 3, 11 }, { 6, 11, 7 }, { 3, 7, 11 }, // case 174
    { 7, 6, 11 }, { 7, 11, 8 }, { 5, 4, 11 }, { 4, 8, 11 }, // case 175
    { 6, 9, 5 }, { 6, 10, 9 }, { 10, 8, 9 }, // case 176
    { 3, 6, 10 }, { 0, 6, 3 }, { 0, 5, 6 }, { 0, 9, 5 }, // case 177
    { 0, 10, 8 }, { 0, 5, 10 }, { 0, 1, 5 }, { 5, 6, 10 }, // case 178
    { 6, 10, 3 }, { 6, 3, 5 }, { 5, 3, 1 }, // case 179
    { 1, 2, 11 }, { 9, 5, 10 }, { 9, 10, 8 }, { 10, 5, 6 }, // case 180
    { 0, 10, 3 }, { 0, 6, 10 }, { 0, 9, 6 }, { 5, 6, 9 }, { 1, 2, 11 }, // case 181
    { 10, 8, 5 }, { 10, 5, 6 }, { 8, 0, 5 }, { 11, 5, 2 }, { 0, 2, 5 }, // case 182
    { 6, 10, 3 }, { 6, 3, 5 }, { 2, 11, 3 }, { 11, 5, 3 }, // case 183
    { 5, 8, 9 }, { 5, 2, 8 }, { 5, 6, 2 }, { 3, 8, 2 }, // case 184
    { 9, 5, 6 }, { 9, 6, 0 }, { 0, 6, 2 }, // case 185
    { 1, 5, 8 }, { 1, 8, 0 }, { 5, 6, 8 }, { 3, 8, 2 }, { 6, 2, 8 }, // case 186
    { 1, 5, 6 }, { 2, 1, 6 }, // case 187
    { 1, 3, 6 }, { 1, 6, 11 }, { 3, 8, 6 }, { 5, 6, 9 }, { 8, 9, 6 }, // case 188
    { 11, 1, 0 }, { 11, 0, 6 }, { 9, 5, 0 }, { 5, 6, 0 }, // case 189
    { 0, 3, 8 }, { 5, 6, 11 }, // case 190
    { 11, 5, 6 }, // case 191
    { 10, 5, 11 }, { 7, 5, 10 }, // case 192
    { 10, 5, 11 }, { 10, 7, 5 }, { 8, 3, 0 }, // case 193
    { 5, 10, 7 }, { 5, 11, 10 }, { 1, 9, 0 }, // case 194
    { 11, 7, 5 }, { 11, 10, 7 }, { 9, 8, 1 }, { 8, 3, 1 }, // case 195
    { 10, 1, 2 }, { 10, 7, 1 }, { 7, 5, 1 }, // case 196
    { 0, 8, 3 }, { 1, 2, 7 }, { 1, 7, 5 }, { 7, 2, 10 }, // case 197
    { 9, 7, 5 }, { 9, 2, 7 }, { 9, 0, 2 }, { 2, 10, 7 }, // case 198
    { 7, 5, 2 }, { 7, 2, 10 }, { 5, 9, 2 }, { 3, 2, 8 }, { 9, 8, 2 }, // case 199
    { 2, 5, 11 }, { 2, 3, 5 }, { 3, 7, 5 }, // case 200
    { 8, 2, 0 }, { 8, 5, 2 }, { 8, 7, 5 }, { 11, 2, 5 }, // case 201
    { 9, 0, 1 }, { 5, 11, 3 }, { 5, 3, 7 }, { 3, 11, 2 }, // case 202
    { 9, 8, 2 }, { 9, 2, 1 }, { 8, 7, 2 }, { 11, 2, 5 }, { 7, 5, 2 }, // case 203
    { 1, 3, 5 }, { 3, 7, 5 }, // case 204
    { 0, 8, 7 }, { 0, 7, 1 }, { 1, 7, 5 }, // case 205
    { 9, 0, 3 }, { 9, 3, 5 }, { 5, 3, 7 }, // case 206
    { 9, 8, 7 }, { 5, 9, 7 }, // case 207
    { 5, 8, 4 }, { 5, 11, 8 }, { 11, 10, 8 }, // case 208
    { 5, 0, 4 }, { 5, 10, 0 }, { 5, 11, 10 }, { 10, 3, 0 }, // case 209
    { 0, 1, 9 }, { 8, 4, 11 }, { 8, 11, 10 }, { 11, 4, 5 }, // case 210
    { 11, 10, 4 }, { 11, 4, 5 }, { 10, 3, 4 }, { 9, 4, 1 }, { 3, 1, 4 }, // case 211
    { 2, 5, 1 }, { 2, 8, 5 }, { 2, 10, 8 }, { 4, 5, 8 }, // case 212
    { 0, 4, 10 }, { 0, 10, 3 }, { 4, 5, 10 }, { 2, 10, 1 }, { 5, 1, 10 }, // case 213
    { 0, 2, 5 }, { 0, 5, 9 }, { 2, 10, 5 }, { 4, 5, 8 }, { 10, 8, 5 }, // case 214
    { 9, 4, 5 }, { 2, 10, 3 }, // case 215
    { 2, 5, 11 }, { 3, 5, 2 }, { 3, 4, 5 }, { 3, 8, 4 }, // case 216
    { 5, 11, 2 }, { 5, 2, 4 }, { 4, 2, 0 }, // case 217
    { 3, 11, 2 }, { 3, 5, 11 }, { 3, 8, 5 }, { 4, 5, 8 }, { 0, 1, 9 }, // case 218
    { 5, 11, 2 }, { 5, 2, 4 }, { 1, 9, 2 }, { 9, 4, 2 }, // case 219
    { 8, 4, 5 }, { 8, 5, 3 }, { 3, 5, 1 }, // case 220
    { 0, 4, 5 }, { 1, 0, 5 }, // case 221
    { 8, 4, 5 }, { 8, 5, 3 }, { 9, 0, 5 }, { 0, 3, 5 }, // case 222
    { 9, 4, 5 }, // case 223
    { 4, 10, 7 }, { 4, 9, 10 }, { 9, 11, 10 }, // case 224
    { 0, 8, 3 }, { 4, 9, 7 }, { 9, 10, 7 }, { 9, 11, 10 }, // case 225
    { 1, 11, 10 }, { 1, 10, 4 }, { 1, 4, 0 }, { 7, 4, 10 }, // case 226
    { 3, 1, 4 }, { 3, 4, 8 }, { 1, 11, 4 }, { 7, 4, 10 }, { 11, 10, 4 }, // case 227
    { 4, 10, 7 }, { 9, 10, 4 }, { 9, 2, 10 }, { 9, 1, 2 }, // case 228
    { 9, 7, 4 }, { 9, 10, 7 }, { 9, 1, 10 }, { 2, 10, 1 }, { 0, 8, 3 }, // case 229
    { 10, 7, 4 }, { 10, 4, 2 }, { 2, 4, 0 }, // case 230
    { 10, 7, 4 }, { 10, 4, 2 }, { 8, 3, 4 }, { 3, 2, 4 }, // case 231
    { 2, 9, 11 }, { 2, 7, 9 }, { 2, 3, 7 }, { 7, 4, 9 }, // case 232
    { 9, 11, 7 }, { 9, 7, 4 }, { 11, 2, 7 }, { 8, 7, 0 }, { 2, 0, 7 }, // case 233
    { 3, 7, 11 }, { 3, 11, 2 }, { 7, 4, 11 }, { 1, 11, 0 }, { 4, 0, 11 }, // case 234
    { 1, 11, 2 }, { 8, 7, 4 }, // case 235
    { 4, 9, 1 }, { 4, 1, 7 }, { 7, 1, 3 }, // case 236
    { 4, 9, 1 }, { 4, 1, 7 }, { 0, 8, 1 }, { 8, 7, 1 }, // case 237
    { 4, 0, 3 }, { 7, 4, 3 }, // case 238
    { 4, 8, 7 }, // case 239
    { 9, 11, 8 }, { 11, 10, 8 }, // case 240
    { 3, 0, 9 }, { 3, 9, 10 }, { 10, 9, 11 }, // case 241
    { 0, 1, 11 }, { 0, 11, 8 }, { 8, 11, 10 }, // case 242
    { 3, 1, 11 }, { 10, 3, 11 }, // case 243
    { 1, 2, 10 }, { 1, 10, 9 }, { 9, 10, 8 }, // case 244
    { 3, 0, 9 }, { 3, 9, 10 }, { 1, 2, 9 }, { 2, 10, 9 }, // case 245
    { 0, 2, 10 }, { 8, 0, 10 }, // case 246
    { 3, 2, 10 }, // case 247
    { 2, 3, 8 }, { 2, 8, 11 }, { 11, 8, 9 }, // case 248
    { 9, 11, 2 }, { 0, 9, 2 }, // case 249
    { 2, 3, 8 }, { 2, 8, 11 }, { 0, 1, 8 }, { 1, 11, 8 }, // case 250
    { 1, 11, 2 }, // case 251
    { 1, 3, 8 }, { 9, 1, 8 }, // case 252
    { 0, 9, 1 }, // case 253
    { 0, 3, 8 }, // case 254
    // case 255
    // CELL_SHAPE_WEDGE
    // case 0
    { 0, 6, 2 }, // case 1
    { 0, 1, 7 }, // case 2
    { 6, 1, 7 }, { 6, 2, 1 }, // case 3
    { 1, 2, 8 }, // case 4
    { 6, 1, 0 }, { 6, 8, 1 }, // case 5
    { 0, 2, 8 }, { 7, 0, 8 }, // case 6
    { 7, 6, 8 }, // case 7
    { 3, 5, 6 }, // case 8
    { 3, 5, 0 }, { 5, 2, 0 }, // case 9
    { 0, 1, 7 }, { 6, 3, 5 }, // case 10
    { 1, 7, 3 }, { 1, 3, 5 }, { 1, 5, 2 }, // case 11
    { 2, 8, 1 }, { 6, 3, 5 }, // case 12
    { 0, 3, 1 }, { 1, 3, 5 }, { 1, 5, 8 }, // case 13
    { 6, 3, 5 }, { 0, 8, 7 }, { 0, 2, 8 }, // case 14
    { 7, 3, 5 }, { 7, 5, 8 }, // case 15
    { 7, 4, 3 }, // case 16
    { 7, 4, 3 }, { 0, 6, 2 }, // case 17
    { 0, 1, 3 }, { 1, 4, 3 }, // case 18
    { 1, 4, 3 }, { 1, 3, 6 }, { 1, 6, 2 }, // case 19
    { 7, 4, 3 }, { 2, 8, 1 }, // case 20
    { 7, 4, 3 }, { 6, 1, 0 }, { 6, 8, 1 }, // case 21
    { 0, 4, 3 }, { 0, 8, 4 }, { 0, 2, 8 }, // case 22
    { 6, 8, 3 }, { 3, 8, 4 }, // case 23
    { 6, 7, 4 }, { 6, 4, 5 }, // case 24
    { 0, 7, 5 }, { 7, 4, 5 }, { 2, 0, 5 }, // case 25
    { 1, 6, 0 }, { 1, 5, 6 }, { 1, 4, 5 }, // case 26
    { 2, 1, 5 }, { 5, 1, 4 }, // case 27
    { 2, 8, 1 }, { 6, 7, 5 }, { 7, 4, 5 }, // case 28
    { 0, 7, 5 }, { 7, 4, 5 }, { 0, 5, 1 }, { 1, 5, 8 }, // case 29
    { 0, 2, 8 }, { 0, 8, 4 }, { 0, 4, 5 }, { 0, 5, 6 }, // case 30
    { 8, 4, 5 }, // case 31
    { 4, 8, 5 }, // case 32
    { 4, 8, 5 }, { 0, 6, 2 }, // case 33
    { 4, 8, 5 }, { 0, 1, 7 }, // case 34
    { 4, 8, 5 }, { 6, 1, 7 }, { 6, 2, 1 }, // case 35
    { 1, 5, 4 }, { 2, 5, 1 }, // case 36
    { 1, 5, 4 }, { 1, 6, 5 }, { 1, 0, 6 }, // case 37
    { 5, 4, 7 }, { 5, 7, 0 }, { 5, 0, 2 }, // case 38
    { 6, 4, 7 }, { 6, 5, 4 }, // case 39
    { 6, 3, 8 }, { 3, 4, 8 }, // case 40
    { 0, 3, 4 }, { 0, 4, 8 }, { 0, 8, 2 }, // case 41
    { 7, 0, 1 }, { 6, 3, 4 }, { 6, 4, 8 }, // case 42
    { 1, 7, 3 }, { 1, 3, 2 }, { 2, 3, 8 }, { 8, 3, 4 }, // case 43
    { 2, 6, 1 }, { 6, 3, 1 }, { 3, 4, 1 }, // case 44
    { 0, 3, 1 }, { 1, 3, 4 }, // case 45
    { 7, 0, 4 }, { 4, 0, 2 }, { 4, 2, 3 }, { 3, 2, 6 }, // case 46
    { 7, 3, 4 }, // case 47
    { 7, 8, 5 }, { 7, 5, 3 }, // case 48
    { 0, 6, 2 }, { 7, 8, 5 }, { 7, 5, 3 }, // case 49
    { 0, 1, 3 }, { 1, 5, 3 }, { 1, 8, 5 }, // case 50
    { 2, 1, 6 }, { 6, 1, 3 }, { 5, 1, 8 }, { 3, 1, 5 }, // case 51
    { 1, 3, 7 }, { 1, 5, 3 }, { 1, 2, 5 }, // case 52
    { 1, 0, 6 }, { 1, 6, 5 }, { 1, 5, 7 }, { 7, 5, 3 }, // case 53
    { 0, 2, 5 }, { 0, 5, 3 }, // case 54
    { 3, 6, 5 }, // case 55
    { 7, 8, 6 }, // case 56
    { 0, 7, 8 }, { 0, 8, 2 }, // case 57
    { 0, 1, 6 }, { 1, 8, 6 }, // case 58
    { 2, 1, 8 }, // case 59
    { 6, 7, 1 }, { 6, 1, 2 }, // case 60
    { 0, 7, 1 }, // case 61
    { 0, 2, 6 }, // case 62
    // case 63
    // CELL_SHAPE_PYRAMID
    // case 0
    { 3, 4, 0 }, // case 1
    { 5, 1, 0 }, // case 2
    { 5, 1, 4 }, { 1, 3, 4 }, // case 3
    { 6, 2, 1 }, // case 4
    { 3, 4, 0 }, { 6, 2, 1 }, // case 5
    { 5, 2, 0 }, { 6, 2, 5 }, // case 6
    { 2, 3, 4 }, { 2, 4, 6 }, { 4, 5, 6 }, // case 7
    { 2, 7, 3 }, // case 8
    { 2, 7, 4 }, { 4, 0, 2 }, // case 9
    { 5, 1, 0 }, { 2, 7, 3 }, // case 10
    { 5, 7, 4 }, { 1, 7, 5 }, { 2, 7, 1 }, // case 11
    { 6, 3, 1 }, { 7, 3, 6 }, // case 12
    { 4, 6, 7 }, { 0, 6, 4 }, { 1, 6, 0 }, // case 13
    { 7, 5, 6 }, { 3, 5, 7 }, { 0, 5, 3 }, // case 14
    { 7, 4, 5 }, { 7, 5, 6 }, // case 15
    { 7, 5, 4 }, { 7, 6, 5 }, // case 16
    { 5, 0, 3 }, { 6, 5, 3 }, { 7, 6, 3 }, // case 17
    { 1, 0, 4 }, { 7, 1, 4 }, { 6, 1, 7 }, // case 18
    { 6, 1, 3 }, { 7, 6, 3 }, // case 19
    { 7, 5, 4 }, { 7, 1, 5 }, { 7, 2, 1 }, // case 20
    { 3, 7, 0 }, { 7, 5, 0 }, { 7, 2, 5 }, { 2, 1, 5 }, // case 21
    { 4, 2, 0 }, { 7, 2, 4 }, // case 22
    { 7, 2, 3 }, // case 23
    { 2, 4, 3 }, { 5, 4, 2 }, { 6, 5, 2 }, // case 24
    { 2, 5, 0 }, { 2, 6, 5 }, // case 25
    { 6, 1, 0 }, { 4, 6, 0 }, { 3, 6, 4 }, { 3, 2, 6 }, // case 26
    { 2, 6, 1 }, // case 27
    { 1, 4, 3 }, { 1, 5, 4 }, // case 28
    { 1, 5, 0 }, // case 29
    { 4, 3, 0 }, // case 30
    // case 31
    { 0, 0, 0 } // dummy
  };
  // clang-format on

  viskores::IdComponent offset = GetTriTableOffset(cellType, caseNumber);
  return triTable[offset + triangleNumber];
}
}
}
} // namespace viskores::worklet::marching_cells

#endif //viskores_MarchingCellTables_h
