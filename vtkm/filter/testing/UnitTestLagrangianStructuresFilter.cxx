//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#include <iostream>
#include <vtkm/cont/DataSetBuilderUniform.h>
#include <vtkm/cont/DataSetFieldAdd.h>
#include <vtkm/cont/testing/Testing.h>

#include <vtkm/filter/LagrangianStructures.h>

namespace auxiliary
{
enum class Option
{
  _2D = 0,
  _3D
};

void ValidateLCSFilterResult(const vtkm::cont::ArrayHandle<vtkm::FloatDefault>& vtkmOutput,
                             const vtkm::cont::ArrayHandle<vtkm::FloatDefault>& visitOutput,
                             const vtkm::cont::ArrayHandle<vtkm::FloatDefault>& expDiff)
{
  VTKM_TEST_ASSERT(vtkmOutput.GetNumberOfValues() == visitOutput.GetNumberOfValues(),
                   "Wrong number of values");

  const vtkm::FloatDefault tolerance = 100 * vtkm::Epsilon<vtkm::FloatDefault>();

  auto vtkmPortal = vtkmOutput.GetPortalConstControl();
  auto visitPortal = visitOutput.GetPortalConstControl();
  auto diffPortal = expDiff.GetPortalConstControl();

  for (vtkm::Id index = 0; index < vtkmOutput.GetNumberOfValues(); index++)
  {
    auto vtkm = vtkmPortal.Get(index);
    auto visit = visitPortal.Get(index);
    auto exp = diffPortal.Get(index);
    auto diff = vtkm::Abs(vtkm - visit);
    std::stringstream message;
    message << "Calculated o/p is not correct, \n"
            << "VTKM : " << vtkm << ", VISIT : " << visit << ", RES : " << vtkm::Abs(diff - exp);
    VTKM_TEST_ASSERT(vtkm::Abs(diff - exp) <= tolerance || diff < tolerance, message.str());
  }
}

vtkm::FloatDefault vecData2D[200 * 3] = {
  0.000000f,  0.000000f,  0.000000f,  -0.032369f, 0.000000f,  0.000000f,  -0.061107f, 0.000000f,
  0.000000f,  -0.083145f, 0.000000f,  0.000000f,  -0.096136f, 0.000000f,  0.000000f,  -0.098712f,
  0.000000f,  0.000000f,  -0.090620f, 0.000000f,  0.000000f,  -0.072751f, -0.000000f, 0.000000f,
  -0.047041f, 0.000000f,  0.000000f,  -0.016264f, 0.000000f,  0.000000f,  0.016263f,  0.000000f,
  0.000000f,  0.047040f,  0.000000f,  0.000000f,  0.072750f,  0.000000f,  0.000000f,  0.090620f,
  0.000000f,  0.000000f,  0.098712f,  -0.000000f, 0.000000f,  0.096136f,  0.000000f,  0.000000f,
  0.083145f,  -0.000000f, 0.000000f,  0.061108f,  0.000000f,  0.000000f,  0.032369f,  0.000000f,
  0.000000f,  0.000000f,  0.000000f,  0.000000f,  0.000000f,  0.106811f,  0.000000f,  -0.030274f,
  0.100785f,  0.000000f,  -0.057152f, 0.083925f,  0.000000f,  -0.077763f, 0.058081f,  0.000000f,
  -0.089914f, 0.026058f,  0.000000f,  -0.092323f, -0.008686f, 0.000000f,  -0.084755f, -0.042409f,
  0.000000f,  -0.068042f, -0.071488f, 0.000000f,  -0.043996f, -0.092800f, 0.000000f,  -0.015211f,
  -0.104060f, 0.000000f,  0.015210f,  -0.104060f, 0.000000f,  0.043995f,  -0.092801f, 0.000000f,
  0.068042f,  -0.071489f, 0.000000f,  0.084754f,  -0.042410f, 0.000000f,  0.092323f,  -0.008687f,
  0.000000f,  0.089914f,  0.026057f,  0.000000f,  0.077763f,  0.058080f,  0.000000f,  0.057152f,
  0.083925f,  0.000000f,  0.030274f,  0.100785f,  0.000000f,  0.000000f,  0.106811f,  0.000000f,
  0.000000f,  0.200103f,  0.000000f,  -0.024596f, 0.188813f,  0.000000f,  -0.046433f, 0.157227f,
  0.000000f,  -0.063178f, 0.108809f,  0.000000f,  -0.073050f, 0.048817f,  0.000000f,  -0.075007f,
  -0.016272f, 0.000000f,  -0.068858f, -0.079450f, 0.000000f,  -0.055280f, -0.133927f, 0.000000f,
  -0.035744f, -0.173854f, 0.000000f,  -0.012358f, -0.194948f, 0.000000f,  0.012357f,  -0.194949f,
  0.000000f,  0.035743f,  -0.173855f, 0.000000f,  0.055280f,  -0.133929f, 0.000000f,  0.068858f,
  -0.079452f, 0.000000f,  0.075007f,  -0.016274f, 0.000000f,  0.073050f,  0.048816f,  0.000000f,
  0.063178f,  0.108809f,  0.000000f,  0.046433f,  0.157227f,  0.000000f,  0.024596f,  0.188813f,
  0.000000f,  0.000000f,  0.200103f,  0.000000f,  0.000000f,  0.269034f,  0.000000f,  -0.016018f,
  0.253856f,  0.000000f,  -0.030240f, 0.211388f,  0.000000f,  -0.041145f, 0.146292f,  0.000000f,
  -0.047574f, 0.065633f,  0.000000f,  -0.048849f, -0.021878f, 0.000000f,  -0.044845f, -0.106820f,
  0.000000f,  -0.036002f, -0.180062f, 0.000000f,  -0.023279f, -0.233744f, 0.000000f,  -0.008049f,
  -0.262104f, 0.000000f,  0.008048f,  -0.262105f, 0.000000f,  0.023278f,  -0.233745f, 0.000000f,
  0.036001f,  -0.180065f, 0.000000f,  0.044844f,  -0.106822f, 0.000000f,  0.048849f,  -0.021880f,
  0.000000f,  0.047574f,  0.065632f,  0.000000f,  0.041145f,  0.146291f,  0.000000f,  0.030240f,
  0.211388f,  0.000000f,  0.016018f,  0.253856f,  0.000000f,  0.000000f,  0.269034f,  0.000000f,
  0.000000f,  0.305617f,  0.000000f,  -0.005557f, 0.288374f,  0.000000f,  -0.010491f, 0.240132f,
  0.000000f,  -0.014274f, 0.166185f,  0.000000f,  -0.016504f, 0.074558f,  0.000000f,  -0.016947f,
  -0.024852f, 0.000000f,  -0.015557f, -0.121345f, 0.000000f,  -0.012490f, -0.204547f, 0.000000f,
  -0.008076f, -0.265527f, 0.000000f,  -0.002792f, -0.297744f, 0.000000f,  0.002792f,  -0.297745f,
  0.000000f,  0.008076f,  -0.265529f, 0.000000f,  0.012490f,  -0.204549f, 0.000000f,  0.015557f,
  -0.121348f, 0.000000f,  0.016947f,  -0.024855f, 0.000000f,  0.016504f,  0.074557f,  0.000000f,
  0.014274f,  0.166184f,  0.000000f,  0.010491f,  0.240132f,  0.000000f,  0.005557f,  0.288374f,
  0.000000f,  0.000000f,  0.305617f,  0.000000f,  0.000000f,  0.305617f,  0.000000f,  0.005557f,
  0.288374f,  0.000000f,  0.010491f,  0.240132f,  0.000000f,  0.014274f,  0.166185f,  0.000000f,
  0.016504f,  0.074558f,  0.000000f,  0.016947f,  -0.024852f, 0.000000f,  0.015557f,  -0.121345f,
  0.000000f,  0.012490f,  -0.204547f, 0.000000f,  0.008076f,  -0.265527f, 0.000000f,  0.002792f,
  -0.297744f, 0.000000f,  -0.002792f, -0.297745f, 0.000000f,  -0.008076f, -0.265529f, 0.000000f,
  -0.012490f, -0.204549f, 0.000000f,  -0.015557f, -0.121348f, 0.000000f,  -0.016947f, -0.024855f,
  0.000000f,  -0.016504f, 0.074557f,  0.000000f,  -0.014274f, 0.166184f,  0.000000f,  -0.010491f,
  0.240132f,  0.000000f,  -0.005557f, 0.288374f,  0.000000f,  -0.000000f, 0.305617f,  0.000000f,
  0.000000f,  0.269034f,  0.000000f,  0.016018f,  0.253856f,  0.000000f,  0.030240f,  0.211388f,
  0.000000f,  0.041145f,  0.146292f,  0.000000f,  0.047574f,  0.065633f,  0.000000f,  0.048849f,
  -0.021878f, 0.000000f,  0.044845f,  -0.106820f, 0.000000f,  0.036002f,  -0.180062f, 0.000000f,
  0.023279f,  -0.233744f, 0.000000f,  0.008049f,  -0.262104f, 0.000000f,  -0.008048f, -0.262105f,
  0.000000f,  -0.023278f, -0.233745f, 0.000000f,  -0.036001f, -0.180065f, 0.000000f,  -0.044844f,
  -0.106822f, 0.000000f,  -0.048849f, -0.021880f, 0.000000f,  -0.047574f, 0.065632f,  0.000000f,
  -0.041145f, 0.146291f,  0.000000f,  -0.030240f, 0.211388f,  0.000000f,  -0.016018f, 0.253856f,
  0.000000f,  -0.000000f, 0.269034f,  0.000000f,  0.000000f,  0.200103f,  0.000000f,  0.024596f,
  0.188813f,  0.000000f,  0.046433f,  0.157227f,  0.000000f,  0.063178f,  0.108809f,  0.000000f,
  0.073050f,  0.048817f,  0.000000f,  0.075007f,  -0.016272f, 0.000000f,  0.068858f,  -0.079450f,
  0.000000f,  0.055280f,  -0.133927f, 0.000000f,  0.035744f,  -0.173854f, 0.000000f,  0.012358f,
  -0.194948f, 0.000000f,  -0.012357f, -0.194949f, 0.000000f,  -0.035743f, -0.173855f, 0.000000f,
  -0.055280f, -0.133929f, 0.000000f,  -0.068858f, -0.079452f, 0.000000f,  -0.075007f, -0.016274f,
  0.000000f,  -0.073050f, 0.048816f,  0.000000f,  -0.063178f, 0.108809f,  0.000000f,  -0.046433f,
  0.157227f,  0.000000f,  -0.024596f, 0.188813f,  0.000000f,  -0.000000f, 0.200103f,  0.000000f,
  0.000000f,  0.106811f,  0.000000f,  0.030274f,  0.100785f,  0.000000f,  0.057152f,  0.083925f,
  0.000000f,  0.077763f,  0.058081f,  0.000000f,  0.089914f,  0.026058f,  0.000000f,  0.092323f,
  -0.008686f, 0.000000f,  0.084755f,  -0.042409f, 0.000000f,  0.068042f,  -0.071488f, 0.000000f,
  0.043996f,  -0.092800f, 0.000000f,  0.015211f,  -0.104060f, 0.000000f,  -0.015210f, -0.104060f,
  0.000000f,  -0.043995f, -0.092801f, 0.000000f,  -0.068042f, -0.071489f, 0.000000f,  -0.084754f,
  -0.042410f, 0.000000f,  -0.092323f, -0.008687f, 0.000000f,  -0.089914f, 0.026057f,  0.000000f,
  -0.077763f, 0.058080f,  0.000000f,  -0.057152f, 0.083925f,  0.000000f,  -0.030274f, 0.100785f,
  0.000000f,  -0.000000f, 0.106811f,  0.000000f,  0.000000f,  0.000000f,  0.000000f,  0.032369f,
  0.000000f,  0.000000f,  0.061107f,  0.000000f,  0.000000f,  0.083145f,  0.000000f,  0.000000f,
  0.096136f,  0.000000f,  0.000000f,  0.098712f,  -0.000000f, 0.000000f,  0.090620f,  -0.000000f,
  0.000000f,  0.072751f,  -0.000000f, 0.000000f,  0.047041f,  -0.000000f, 0.000000f,  0.016264f,
  -0.000000f, 0.000000f,  -0.016263f, -0.000000f, 0.000000f,  -0.047040f, -0.000000f, 0.000000f,
  -0.072750f, -0.000000f, 0.000000f,  -0.090620f, -0.000000f, 0.000000f,  -0.098712f, -0.000000f,
  0.000000f,  -0.096136f, 0.000000f,  0.000000f,  -0.083145f, 0.000000f,  0.000000f,  -0.061108f,
  0.000000f,  0.000000f,  -0.032369f, 0.000000f,  0.000000f,  -0.000000f, 0.000000f,  0.000000f
};

vtkm::FloatDefault vecData3D[125 * 3] = {
  -1.359000f, -1.359000f, -1.359000f, -1.359000f, -0.136603f, -0.056133f, -1.359000f, -0.300565f,
  -1.704760f, -1.359000f, -1.143790f, -0.278636f, -1.359000f, 0.513701f,  -0.945288f, -0.056133f,
  -1.359000f, -0.136603f, -0.056133f, -0.136603f, 1.166260f,  -0.056133f, -0.300565f, -0.482370f,
  -0.056133f, -1.143790f, 0.943757f,  -0.056133f, 0.513701f,  0.277104f,  -1.704760f, -1.359000f,
  -0.300565f, -1.704760f, -0.136603f, 1.002290f,  -1.704760f, -0.300565f, -0.646332f, -1.704760f,
  -1.143790f, 0.779793f,  -1.704760f, 0.513701f,  0.113142f,  -0.278636f, -1.359000f, -1.143790f,
  -0.278636f, -0.136603f, 0.159074f,  -0.278636f, -0.300565f, -1.489550f, -0.278636f, -1.143790f,
  -0.063429f, -0.278636f, 0.513701f,  -0.730080f, -0.945288f, -1.359000f, 0.513701f,  -0.945288f,
  -0.136603f, 1.816560f,  -0.945288f, -0.300565f, 0.167933f,  -0.945288f, -1.143790f, 1.594060f,
  -0.945288f, 0.513701f,  0.927407f,  -0.136603f, -0.056133f, -1.359000f, -0.136603f, 1.166260f,
  -0.056133f, -0.136603f, 1.002290f,  -1.704760f, -0.136603f, 0.159074f,  -0.278636f, -0.136603f,
  1.816560f,  -0.945288f, 1.166260f,  -0.056133f, -0.136603f, 1.166260f,  1.166260f,  1.166260f,
  1.166260f,  1.002290f,  -0.482370f, 1.166260f,  0.159074f,  0.943757f,  1.166260f,  1.816560f,
  0.277104f,  -0.482370f, -0.056133f, -0.300565f, -0.482370f, 1.166260f,  1.002290f,  -0.482370f,
  1.002290f,  -0.646332f, -0.482370f, 0.159074f,  0.779793f,  -0.482370f, 1.816560f,  0.113142f,
  0.943757f,  -0.056133f, -1.143790f, 0.943757f,  1.166260f,  0.159074f,  0.943757f,  1.002290f,
  -1.489550f, 0.943757f,  0.159074f,  -0.063429f, 0.943757f,  1.816560f,  -0.730080f, 0.277104f,
  -0.056133f, 0.513701f,  0.277104f,  1.166260f,  1.816560f,  0.277104f,  1.002290f,  0.167933f,
  0.277104f,  0.159074f,  1.594060f,  0.277104f,  1.816560f,  0.927407f,  -0.300565f, -1.704760f,
  -1.359000f, -0.300565f, -0.482370f, -0.056133f, -0.300565f, -0.646332f, -1.704760f, -0.300565f,
  -1.489550f, -0.278636f, -0.300565f, 0.167933f,  -0.945288f, 1.002290f,  -1.704760f, -0.136603f,
  1.002290f,  -0.482370f, 1.166260f,  1.002290f,  -0.646332f, -0.482370f, 1.002290f,  -1.489550f,
  0.943757f,  1.002290f,  0.167933f,  0.277104f,  -0.646332f, -1.704760f, -0.300565f, -0.646332f,
  -0.482370f, 1.002290f,  -0.646332f, -0.646332f, -0.646332f, -0.646332f, -1.489550f, 0.779793f,
  -0.646332f, 0.167933f,  0.113142f,  0.779792f,  -1.704760f, -1.143790f, 0.779792f,  -0.482370f,
  0.159074f,  0.779792f,  -0.646332f, -1.489550f, 0.779792f,  -1.489550f, -0.063429f, 0.779792f,
  0.167933f,  -0.730080f, 0.113142f,  -1.704760f, 0.513701f,  0.113142f,  -0.482370f, 1.816560f,
  0.113142f,  -0.646332f, 0.167933f,  0.113142f,  -1.489550f, 1.594060f,  0.113142f,  0.167933f,
  0.927407f,  -1.143790f, -0.278636f, -1.359000f, -1.143790f, 0.943757f,  -0.056133f, -1.143790f,
  0.779793f,  -1.704760f, -1.143790f, -0.063429f, -0.278636f, -1.143790f, 1.594060f,  -0.945288f,
  0.159074f,  -0.278636f, -0.136603f, 0.159074f,  0.943757f,  1.166260f,  0.159074f,  0.779793f,
  -0.482370f, 0.159074f,  -0.063429f, 0.943757f,  0.159074f,  1.594060f,  0.277104f,  -1.489550f,
  -0.278636f, -0.300565f, -1.489550f, 0.943757f,  1.002290f,  -1.489550f, 0.779793f,  -0.646332f,
  -1.489550f, -0.063429f, 0.779793f,  -1.489550f, 1.594060f,  0.113142f,  -0.063428f, -0.278636f,
  -1.143790f, -0.063428f, 0.943757f,  0.159074f,  -0.063428f, 0.779793f,  -1.489550f, -0.063428f,
  -0.063429f, -0.063429f, -0.063428f, 1.594060f,  -0.730080f, -0.730080f, -0.278636f, 0.513701f,
  -0.730080f, 0.943757f,  1.816560f,  -0.730080f, 0.779793f,  0.167933f,  -0.730080f, -0.063429f,
  1.594060f,  -0.730080f, 1.594060f,  0.927407f,  0.513701f,  -0.945288f, -1.359000f, 0.513701f,
  0.277104f,  -0.056133f, 0.513701f,  0.113142f,  -1.704760f, 0.513701f,  -0.730080f, -0.278636f,
  0.513701f,  0.927407f,  -0.945288f, 1.816560f,  -0.945288f, -0.136603f, 1.816560f,  0.277104f,
  1.166260f,  1.816560f,  0.113142f,  -0.482370f, 1.816560f,  -0.730080f, 0.943757f,  1.816560f,
  0.927407f,  0.277104f,  0.167933f,  -0.945288f, -0.300565f, 0.167933f,  0.277104f,  1.002290f,
  0.167933f,  0.113142f,  -0.646332f, 0.167933f,  -0.730080f, 0.779793f,  0.167933f,  0.927407f,
  0.113142f,  1.594060f,  -0.945288f, -1.143790f, 1.594060f,  0.277104f,  0.159074f,  1.594060f,
  0.113142f,  -1.489550f, 1.594060f,  -0.730080f, -0.063429f, 1.594060f,  0.927407f,  -0.730080f,
  0.927407f,  -0.945288f, 0.513701f,  0.927407f,  0.277104f,  1.816560f,  0.927407f,  0.113142f,
  0.167933f,  0.927407f,  -0.730080f, 1.594060f,  0.927407f,  0.927407f,  0.927407f
};

vtkm::FloatDefault visitData2D[200] = {
  0.175776f, 0.193068f, 0.184354f, 0.162709f, 0.156567f, 0.165079f, 0.183523f, 0.176101f, 0.182635f,
  0.150985f, 0.127186f, 0.173021f, 0.173493f, 0.160812f, 0.124465f, 0.110838f, 0.135879f, 0.184853f,
  0.193068f, 0.000007f, 0.157161f, 0.144306f, 0.108385f, 0.086895f, 0.073086f, 0.092054f, 0.109178f,
  0.115175f, 0.144648f, 0.179178f, 0.177300f, 0.143649f, 0.113350f, 0.097418f, 0.081223f, 0.011488f,
  0.067968f, 0.108383f, 0.161194f, 0.191896f, 0.165629f, 0.133326f, 0.127635f, 0.058606f, 0.025823f,
  0.059042f, 0.100954f, 0.089389f, 0.124448f, 0.174170f, 0.174172f, 0.124446f, 0.089387f, 0.100955f,
  0.059041f, 0.025821f, 0.058604f, 0.127634f, 0.106871f, 0.188216f, 0.169607f, 0.135035f, 0.109767f,
  0.052501f, 0.029647f, 0.016691f, 0.076807f, 0.086811f, 0.121697f, 0.172919f, 0.172921f, 0.121695f,
  0.086809f, 0.076809f, 0.016692f, 0.029643f, 0.052500f, 0.109766f, 0.100262f, 0.181406f, 0.172943f,
  0.133487f, 0.098476f, 0.063748f, 0.031703f, 0.007183f, 0.062060f, 0.078550f, 0.119979f, 0.172034f,
  0.172036f, 0.119977f, 0.078548f, 0.062062f, 0.007186f, 0.031700f, 0.063747f, 0.098476f, 0.099140f,
  0.175363f, 0.175794f, 0.132061f, 0.090747f, 0.073996f, 0.033685f, 0.007585f, 0.053561f, 0.068431f,
  0.120009f, 0.171078f, 0.171080f, 0.120008f, 0.068429f, 0.053560f, 0.007588f, 0.033684f, 0.073997f,
  0.090747f, 0.132058f, 0.176183f, 0.177344f, 0.132153f, 0.091405f, 0.085764f, 0.052348f, 0.010314f,
  0.042262f, 0.064794f, 0.124248f, 0.168788f, 0.168789f, 0.124248f, 0.064795f, 0.042261f, 0.010313f,
  0.052345f, 0.085766f, 0.091405f, 0.132151f, 0.177345f, 0.177002f, 0.134660f, 0.101316f, 0.096766f,
  0.089878f, 0.029814f, 0.039441f, 0.096341f, 0.128119f, 0.162818f, 0.162820f, 0.128117f, 0.096345f,
  0.039441f, 0.029812f, 0.089876f, 0.096768f, 0.101316f, 0.134658f, 0.177003f, 0.174898f, 0.151468f,
  0.135607f, 0.108288f, 0.090863f, 0.080745f, 0.075302f, 0.100106f, 0.124623f, 0.161293f, 0.161294f,
  0.124626f, 0.100106f, 0.075302f, 0.080745f, 0.090864f, 0.108288f, 0.135607f, 0.151465f, 0.174900f,
  0.000001f, 0.161986f, 0.181619f, 0.169865f, 0.145339f, 0.163338f, 0.157883f, 0.171822f, 0.192594f,
  0.186741f, 0.186741f, 0.192594f, 0.171821f, 0.157882f, 0.163337f, 0.145338f, 0.169865f, 0.181618f,
  0.161986f, 0.000001f
};

vtkm::FloatDefault visitData3D[125] = {
  0.000000f, 0.246171f, 0.026549f, 0.051611f, 0.165443f, 0.246171f, 0.071890f, 0.157521f, 0.224824f,
  0.165498f, 0.026549f, 0.177905f, 0.082937f, 0.107466f, 0.149497f, 0.051611f, 0.209922f, 0.057065f,
  0.176006f, 0.095284f, 0.165443f, 0.093980f, 0.106193f, 0.132160f, 0.155382f, 0.246171f, 0.071890f,
  0.177905f, 0.209922f, 0.093980f, 0.071890f, 0.035283f, 0.059712f, 0.068458f, 0.091744f, 0.157521f,
  0.059712f, 0.031083f, 0.161508f, 0.145593f, 0.224824f, 0.068458f, 0.076021f, 0.112219f, 0.054363f,
  0.165498f, 0.091744f, 0.074956f, 0.052716f, 0.000000f, 0.026549f, 0.157521f, 0.082937f, 0.057065f,
  0.106193f, 0.177905f, 0.059712f, 0.031083f, 0.076021f, 0.074956f, 0.082937f, 0.031083f, 0.144075f,
  0.013539f, 0.036500f, 0.107466f, 0.161508f, 0.013539f, 0.074017f, 0.199354f, 0.149497f, 0.145593f,
  0.036500f, 0.130521f, 0.126838f, 0.051611f, 0.224824f, 0.107466f, 0.176006f, 0.132160f, 0.209922f,
  0.068458f, 0.161508f, 0.112219f, 0.052716f, 0.057065f, 0.076021f, 0.013539f, 0.074018f, 0.130521f,
  0.176006f, 0.112219f, 0.074017f, 0.156035f, 0.116269f, 0.095284f, 0.054363f, 0.199354f, 0.116269f,
  0.092932f, 0.165443f, 0.165498f, 0.149497f, 0.095284f, 0.155382f, 0.093980f, 0.091744f, 0.145593f,
  0.054363f, 0.000000f, 0.106193f, 0.074956f, 0.036500f, 0.199354f, 0.126838f, 0.132161f, 0.052716f,
  0.130521f, 0.116269f, 0.092932f, 0.155382f, 0.000000f, 0.126838f, 0.092932f, 0.000000f
};

vtkm::FloatDefault diffData2D[200] = {
  0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.002152f, 0.003656f, 0.021269f,
  0.020149f, 0.043948f, 0.011656f, 0.006264f, 0.020559f, 0.040614f, 0.045728f, 0.026829f, 0.000501f,
  0.000000f, 0.175769f, 0.000001f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
  0.014023f, 0.001336f, 0.003623f, 0.001744f, 0.002334f, 0.015848f, 0.011760f, 0.010831f, 0.061597f,
  0.018926f, 0.000001f, 0.016888f, 0.034735f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
  0.000001f, 0.000000f, 0.000000f, 0.000000f, 0.000001f, 0.000002f, 0.000002f, 0.000001f, 0.000000f,
  0.000000f, 0.000001f, 0.000000f, 0.000000f, 0.026455f, 0.022587f, 0.000000f, 0.000000f, 0.000000f,
  0.000001f, 0.000000f, 0.000001f, 0.000001f, 0.000000f, 0.000001f, 0.000001f, 0.000002f, 0.000001f,
  0.000002f, 0.000002f, 0.000001f, 0.000004f, 0.000001f, 0.000001f, 0.034773f, 0.011799f, 0.000000f,
  0.000000f, 0.000000f, 0.000000f, 0.000001f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000001f,
  0.000002f, 0.000002f, 0.000002f, 0.000000f, 0.000003f, 0.000001f, 0.000001f, 0.000002f, 0.034346f,
  0.002419f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000001f, 0.000001f, 0.000000f, 0.000000f,
  0.000000f, 0.000000f, 0.000002f, 0.000001f, 0.000002f, 0.000002f, 0.000002f, 0.000002f, 0.000000f,
  0.000002f, 0.000002f, 0.000389f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000001f,
  0.000000f, 0.000000f, 0.000000f, 0.000001f, 0.000001f, 0.000001f, 0.000000f, 0.000003f, 0.000002f,
  0.000000f, 0.000001f, 0.000002f, 0.000001f, 0.000001f, 0.000000f, 0.000000f, 0.000000f, 0.000001f,
  0.000000f, 0.000001f, 0.000000f, 0.000001f, 0.000000f, 0.000000f, 0.000002f, 0.000002f, 0.000001f,
  0.000001f, 0.000000f, 0.000001f, 0.000001f, 0.000002f, 0.000001f, 0.000001f, 0.000000f, 0.011404f,
  0.009266f, 0.013854f, 0.008200f, 0.000001f, 0.000002f, 0.000005f, 0.000006f, 0.000000f, 0.000000f,
  0.000004f, 0.000006f, 0.000002f, 0.000001f, 0.008199f, 0.013853f, 0.009266f, 0.011401f, 0.000002f,
  0.173409f, 0.040531f, 0.007484f, 0.012952f, 0.028796f, 0.004284f, 0.000001f, 0.000003f, 0.000006f,
  0.000009f, 0.000008f, 0.000006f, 0.000004f, 0.000002f, 0.004284f, 0.028796f, 0.012952f, 0.007483f,
  0.040531f, 0.173409f
};

vtkm::FloatDefault diffData3D[125] = {
  0.000018f, 0.000000f, 0.000000f, 0.000000f, 0.000063f, 0.000000f, 0.000000f, 0.000001f, 0.000000f,
  0.000074f, 0.000000f, 0.000001f, 0.000001f, 0.000010f, 0.000204f, 0.000000f, 0.000006f, 0.002958f,
  0.019681f, 0.000105f, 0.000067f, 0.001084f, 0.079343f, 0.132158f, 0.155378f, 0.000000f, 0.000000f,
  0.000001f, 0.000006f, 0.001084f, 0.000000f, 0.027930f, 0.000000f, 0.000000f, 0.000006f, 0.000000f,
  0.000000f, 0.000000f, 0.000004f, 0.000027f, 0.000000f, 0.000000f, 0.000001f, 0.000000f, 0.000025f,
  0.000078f, 0.000014f, 0.016555f, 0.040407f, 0.000007f, 0.000000f, 0.000001f, 0.000001f, 0.002958f,
  0.015710f, 0.000001f, 0.000000f, 0.000000f, 0.000001f, 0.016554f, 0.000001f, 0.000000f, 0.000000f,
  0.000015f, 0.000057f, 0.000010f, 0.000003f, 0.000027f, 0.000039f, 0.000006f, 0.000228f, 0.000023f,
  0.000098f, 0.000084f, 0.000015f, 0.000000f, 0.000000f, 0.000008f, 0.000015f, 0.001319f, 0.000006f,
  0.000000f, 0.000004f, 0.000000f, 0.000029f, 0.002958f, 0.000001f, 0.000027f, 0.000090f, 0.000207f,
  0.019681f, 0.000000f, 0.000169f, 0.000002f, 0.000043f, 0.000112f, 0.000020f, 0.000011f, 0.000015f,
  0.000001f, 0.000067f, 0.000069f, 0.000160f, 0.000088f, 0.000227f, 0.001084f, 0.000003f, 0.000030f,
  0.000028f, 0.000001f, 0.079343f, 0.016555f, 0.000104f, 0.000011f, 0.000030f, 0.132159f, 0.040438f,
  0.000389f, 0.000078f, 0.000003f, 0.155380f, 0.000001f, 0.000060f, 0.000001f, 0.000001f
};

void PopulateData(std::vector<vtkm::Vec3f>& fieldVec,
                  std::vector<vtkm::FloatDefault>& visitVec,
                  std::vector<vtkm::FloatDefault>& diffVec,
                  const Option option)
{
  switch (option)
  {
    case Option::_2D:
    {
      vtkm::Id elements = 200;
      for (vtkm::Id index = 0; index < elements; index++)
      {
        vtkm::Id flatidx = index * 3;
        vtkm::Vec3f field;
        field[0] = auxiliary::vecData2D[flatidx];
        field[1] = auxiliary::vecData2D[++flatidx];
        field[2] = auxiliary::vecData2D[++flatidx];
        fieldVec.push_back(field);
        vtkm::FloatDefault diff = auxiliary::diffData2D[index];
        diffVec.push_back(diff);
        vtkm::FloatDefault visit = auxiliary::visitData2D[index];
        visitVec.push_back(visit);
      }
    }
    break;
    case Option::_3D:
    {
      vtkm::Id elements = 125;
      for (vtkm::Id index = 0; index < elements; index++)
      {
        vtkm::Id flatidx = index * 3;
        vtkm::Vec3f field;
        field[0] = auxiliary::vecData3D[flatidx];
        field[1] = auxiliary::vecData3D[++flatidx];
        field[2] = auxiliary::vecData3D[++flatidx];
        fieldVec.push_back(field);
        vtkm::FloatDefault diff = auxiliary::diffData3D[index];
        diffVec.push_back(diff);
        vtkm::FloatDefault visit = auxiliary::visitData3D[index];
        visitVec.push_back(visit);
      }
      break;
    }
  }
}
}

void Test2DLCS()
{
  /*Construct dataset and vector field*/
  vtkm::cont::DataSet inputData;
  vtkm::Id2 dims(20, 10);
  vtkm::Vec2f origin(0.0f, 0.0f);
  vtkm::Vec2f spacing;
  spacing[0] = 2.0f / static_cast<vtkm::FloatDefault>(dims[0] - 1);
  spacing[1] = 1.0f / static_cast<vtkm::FloatDefault>(dims[1] - 1);
  vtkm::cont::DataSetBuilderUniform dataBuilder;
  inputData = dataBuilder.Create(dims, origin, spacing);

  std::vector<vtkm::FloatDefault> diffVec;
  std::vector<vtkm::FloatDefault> visitVec;
  std::vector<vtkm::Vec3f> fieldVec;

  auxiliary::PopulateData(fieldVec, visitVec, diffVec, auxiliary::Option::_2D);

  vtkm::cont::ArrayHandle<vtkm::FloatDefault> diffHandle = vtkm::cont::make_ArrayHandle(diffVec);
  vtkm::cont::ArrayHandle<vtkm::FloatDefault> visitHandle = vtkm::cont::make_ArrayHandle(visitVec);
  vtkm::cont::ArrayHandle<vtkm::Vec3f> fieldHandle = vtkm::cont::make_ArrayHandle(fieldVec);
  vtkm::cont::DataSetFieldAdd fieldAdder;
  fieldAdder.AddPointField(inputData, "velocity", fieldHandle);

  vtkm::filter::LagrangianStructures lagrangianStructures;
  lagrangianStructures.SetStepSize(0.025f);
  lagrangianStructures.SetNumberOfSteps(500);
  lagrangianStructures.SetAdvectionTime(0.025f * 500);
  lagrangianStructures.SetActiveField("velocity");
  vtkm::cont::DataSet outputData = lagrangianStructures.Execute(inputData);

  vtkm::cont::ArrayHandle<vtkm::FloatDefault> FTLEField;
  outputData.GetField("FTLE").GetData().CopyTo(FTLEField);
  auxiliary::ValidateLCSFilterResult(FTLEField, visitHandle, diffHandle);
}

void Test3DLCS()
{
  /*Construct dataset and vector field*/
  vtkm::cont::DataSet inputData;
  vtkm::Id3 dims(5, 5, 5);
  vtkm::Vec3f origin(0.0f, 0.0f, 0.0f);
  vtkm::Vec3f spacing;
  spacing[0] = 10.0f / static_cast<vtkm::FloatDefault>(dims[0] - 1);
  spacing[1] = 10.0f / static_cast<vtkm::FloatDefault>(dims[1] - 1);
  spacing[2] = 10.0f / static_cast<vtkm::FloatDefault>(dims[2] - 1);
  vtkm::cont::DataSetBuilderUniform dataBuilder;
  inputData = dataBuilder.Create(dims, origin, spacing);

  std::vector<vtkm::FloatDefault> diffVec;
  std::vector<vtkm::FloatDefault> visitVec;
  std::vector<vtkm::Vec3f> fieldVec;

  auxiliary::PopulateData(fieldVec, visitVec, diffVec, auxiliary::Option::_3D);

  vtkm::cont::ArrayHandle<vtkm::FloatDefault> diffHandle = vtkm::cont::make_ArrayHandle(diffVec);
  vtkm::cont::ArrayHandle<vtkm::FloatDefault> visitHandle = vtkm::cont::make_ArrayHandle(visitVec);
  vtkm::cont::ArrayHandle<vtkm::Vec3f> fieldHandle = vtkm::cont::make_ArrayHandle(fieldVec);
  vtkm::cont::DataSetFieldAdd fieldAdder;
  fieldAdder.AddPointField(inputData, "velocity", fieldHandle);

  vtkm::filter::LagrangianStructures lagrangianStructures;
  lagrangianStructures.SetStepSize(0.01f);
  lagrangianStructures.SetNumberOfSteps(500);
  lagrangianStructures.SetAdvectionTime(0.01f * 500);
  lagrangianStructures.SetActiveField("velocity");
  vtkm::cont::DataSet outputData = lagrangianStructures.Execute(inputData);

  vtkm::cont::ArrayHandle<vtkm::FloatDefault> FTLEField;
  outputData.GetField("FTLE").GetData().CopyTo(FTLEField);
  auxiliary::ValidateLCSFilterResult(FTLEField, visitHandle, diffHandle);
}

void TestLagrangianStructures()
{
  Test2DLCS();
  Test3DLCS();
}

int UnitTestLagrangianStructuresFilter(int argc, char* argv[])
{
  return vtkm::cont::testing::Testing::Run(TestLagrangianStructures, argc, argv);
}
