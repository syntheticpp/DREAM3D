/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */




#include "EbsdLib/EbsdConstants.h"
#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/OrientationOps/CubicOps.h"
#include "DREAM3DLib/OrientationOps/CubicLowOps.h"
#include "DREAM3DLib/OrientationOps/HexagonalOps.h"
#include "DREAM3DLib/OrientationOps/HexagonalLowOps.h"
#include "DREAM3DLib/OrientationOps/MonoclinicOps.h"
#include "DREAM3DLib/OrientationOps/OrthoRhombicOps.h"
#include "DREAM3DLib/OrientationOps/TetragonalLowOps.h"
#include "DREAM3DLib/OrientationOps/TetragonalOps.h"
#include "DREAM3DLib/OrientationOps/TriclinicOps.h"
#include "DREAM3DLib/OrientationOps/TrigonalLowOps.h"
#include "DREAM3DLib/OrientationOps/TrigonalOps.h"
#include "DREAM3DLib/Utilities/ColorTable.h"



#include "UnitTestSupport.hpp"
#include "TestFileLocations.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TestCrystalStructureTest()
{

  uint32_t chigh = Ebsd::CrystalStructure::Cubic_High;
  uint32_t clow = Ebsd::CrystalStructure::Cubic_Low;

  uint32_t hhigh = Ebsd::CrystalStructure::Hexagonal_High;
  uint32_t hlow = Ebsd::CrystalStructure::Hexagonal_Low;

  bool b = Ebsd::CrystalStructure::Cubic_High == chigh;
  DREAM3D_REQUIRE_EQUAL(b, true)

  b = Ebsd::CrystalStructure::Cubic_High == clow;
  DREAM3D_REQUIRE_EQUAL(b, false)

  b = Ebsd::CrystalStructure::Cubic_High == hlow;
  DREAM3D_REQUIRE_EQUAL(b, false)

  b = Ebsd::CrystalStructure::Hexagonal_High == hhigh;
  DREAM3D_REQUIRE_EQUAL(b, true)

  b = Ebsd::CrystalStructure::Hexagonal_High == hlow;
  DREAM3D_REQUIRE_EQUAL(b, false)

  b = Ebsd::CrystalStructure::Hexagonal_High == clow;
  DREAM3D_REQUIRE_EQUAL(b, false)

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TestEbsdColoring()
{

  // Create 1 of every type of Ops class. This condenses the code below
  std::vector<OrientationOps::Pointer> ops;
  ops.push_back(HexagonalOps::New());
  ops.push_back(CubicOps::New());
  ops.push_back(HexagonalLowOps::New());
  ops.push_back(CubicLowOps::New());
  ops.push_back(TriclinicOps::New());
  ops.push_back(MonoclinicOps::New());
  ops.push_back(OrthoRhombicOps::New());
  ops.push_back(TetragonalLowOps::New());
  ops.push_back(TetragonalOps::New());
  ops.push_back(TrigonalLowOps::New());
  ops.push_back(TrigonalOps::New());

  double refDir[3] = {0.0, 0.0, 1.0};
  double dEuler[3] = {207.1653, 44.2854, 146.5178};
  DREAM3D::Rgb argb = 0x00000000;
  size_t idx = 0;
  argb = ops[idx++]->generateIPFColor(dEuler, refDir, false);
  argb = ops[idx++]->generateIPFColor(dEuler, refDir, false);
  argb = ops[idx++]->generateIPFColor(dEuler, refDir, false);
  argb = ops[idx++]->generateIPFColor(dEuler, refDir, false);
  argb = ops[idx++]->generateIPFColor(dEuler, refDir, false);
  argb = ops[idx++]->generateIPFColor(dEuler, refDir, false);
  argb = ops[idx++]->generateIPFColor(dEuler, refDir, false);
  argb = ops[idx++]->generateIPFColor(dEuler, refDir, false);
  argb = ops[idx++]->generateIPFColor(dEuler, refDir, false);
  argb = ops[idx++]->generateIPFColor(dEuler, refDir, false);
  argb = ops[idx++]->generateIPFColor(dEuler, refDir, false);

  DREAM3D_REQUIRE_EQUAL(idx, Ebsd::CrystalStructure::LaueGroupEnd)

  std::cout << RgbColor::dRed(argb) << " " << RgbColor::dGreen(argb) << " " << RgbColor::dBlue(argb) << " " << std::endl;
}

// -----------------------------------------------------------------------------
//  Use test framework
// -----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  int err = EXIT_SUCCESS;
  DREAM3D_REGISTER_TEST( TestEbsdColoring() )

  DREAM3D_REGISTER_TEST( TestCrystalStructureTest() )

  PRINT_TEST_SUMMARY();
  return err;
}




