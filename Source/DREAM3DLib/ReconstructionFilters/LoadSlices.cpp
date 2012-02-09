/* ============================================================================
 * Copyright (c) 2011 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2011 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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

#include <limits>
#include <vector>
#include "LoadSlices.h"


#include "EbsdLib/H5EbsdVolumeInfo.h"
#include "EbsdLib/H5EbsdVolumeReader.h"
#include "EbsdLib/TSL/AngDirectoryPatterns.h"
#include "EbsdLib/TSL/AngReader.h"
#include "EbsdLib/TSL/AngPhase.h"
#include "EbsdLib/TSL/H5AngVolumeReader.h"
#include "EbsdLib/HKL/H5CtfVolumeReader.h"

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/Common/DREAM3DMath.h"
#include "DREAM3DLib/Common/OrientationMath.h"
#include "DREAM3DLib/Common/DREAM3DRandom.h"

#include "DREAM3DLib/OrientationOps/CubicOps.h"
#include "DREAM3DLib/OrientationOps/HexagonalOps.h"
#include "DREAM3DLib/OrientationOps/OrthoRhombicOps.h"

#include "DREAM3DLib/PrivateFilters/DetermineGoodVoxels.h"


#define ERROR_TXT_OUT 1
#define ERROR_TXT_OUT1 1

const static float m_pi = M_PI;



#define NEW_SHARED_ARRAY(var, type, size)\
  boost::shared_array<type> var##Array(new type[size]);\
  type* var = var##Array.get();

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
LoadSlices::LoadSlices() :
AbstractFilter(),
m_H5EbsdFile(""),
//m_MisorientationTolerance(0.0f),
m_RefFrameZDir(Ebsd::UnknownRefFrameZDirection),
m_ZStartIndex(0),
m_ZEndIndex(0),
m_PhasesC(NULL),
//m_GoodVoxels(NULL),
m_Quats(NULL),
m_EulerAnglesC(NULL)
{
  Seed = MXA::getMilliSeconds();

  m_HexOps = HexagonalOps::New();
  m_OrientationOps.push_back(m_HexOps.get());
  m_CubicOps = CubicOps::New();
  m_OrientationOps.push_back(m_CubicOps.get());
  m_OrthoOps = OrthoRhombicOps::New();
  m_OrientationOps.push_back(m_OrthoOps.get());
  setupFilterOptions();

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
LoadSlices::~LoadSlices()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LoadSlices::setupFilterOptions()
{

  std::vector<FilterOption::Pointer> options;
  {
    FilterOption::Pointer option = FilterOption::New();
    option->setHumanLabel("EBSD Input File (HDF5)");
    option->setPropertyName("H5EbsdFile");
    option->setWidgetType(FilterOption::InputFileWidget);
    option->setValueType("string");
    options.push_back(option);
  }
#if 0
   This should be read from the file so we are NOT going to expose it to the user
  {
    ChoiceFilterOption::Pointer option = ChoiceFilterOption::New();
    option->setHumanLabel("Reference Frame");
    option->setPropertyName("RefFrameZDir");
    option->setWidgetType(FilterOption::ChoiceWidget);
    option->setValueType("Ebsd::RefFrameZDir");
    option->setCastableValueType("unsigned int");
    std::vector<std::string> choices;
    choices.push_back("Low To High");
    choices.push_back("High To Low");
    option->setChoices(choices);
    options.push_back(option);
  }
#endif
  {
    ConstrainedFilterOption<int>::Pointer option = ConstrainedFilterOption<int>::New();
    option->setHumanLabel("Z Start Index");
    option->setPropertyName("ZStartIndex");
    option->setWidgetType(FilterOption::IntConstrainedWidget);
    option->setValueType("int");
    option->setMinimum(0);
    option->setMaximum(0);
    options.push_back(option);
  }
  {
    ConstrainedFilterOption<int>::Pointer option = ConstrainedFilterOption<int>::New();
    option->setHumanLabel("Z End Index");
    option->setPropertyName("ZEndIndex");
    option->setWidgetType(FilterOption::IntConstrainedWidget);
    option->setValueType("int");
    option->setMinimum(0);
    option->setMaximum(0);
    options.push_back(option);
  }
  {
    FilterOption::Pointer option = FilterOption::New();
    option->setHumanLabel("Misorientation Tolerance");
    option->setPropertyName("MisorientationTolerance");
    option->setWidgetType(FilterOption::DoubleWidget);
    option->setValueType("float");
    options.push_back(option);
  }
  // Some how need to get the custom GUIs for the Phase Types and QualityMetric Filters
  // into a Filter Option also.



  setFilterOptions(options);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LoadSlices::dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles)
{
  setErrorCondition(0);
  std::stringstream ss;
  DataContainer* m = getDataContainer();

  CREATE_NON_PREREQ_DATA_SUFFIX(m, DREAM3D, CellData, Phases, C, ss, int32_t, Int32ArrayType, voxels, 1);
  CREATE_NON_PREREQ_DATA_SUFFIX(m, DREAM3D, CellData, EulerAngles, C, ss, float, FloatArrayType, voxels, 3);
  CREATE_NON_PREREQ_DATA(m, DREAM3D, CellData, Quats, ss, float, FloatArrayType, voxels, 5);
//  CREATE_NON_PREREQ_DATA(m, DREAM3D, CellData, GoodVoxels, ss, bool, BoolArrayType, voxels, 1);

  setErrorMessage(ss.str());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LoadSlices::preflight()
{
  dataCheck(true, 1, 1, 1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LoadSlices::execute()
{
  std::stringstream ss;
  DataContainer* m = getDataContainer();
  if(NULL == m)
  {
    setErrorCondition(-1);
    ss << getNameOfClass() << " DataContainer was NULL";
    setErrorMessage(ss.str());
    return;
  }
  int err = 0;
  setErrorCondition(err);
  std::string manufacturer;
  // Get the Size and Resolution of the Volume
  {
    H5EbsdVolumeInfo::Pointer volumeInfoReader = H5EbsdVolumeInfo::New();
    volumeInfoReader->setFileName(m_H5EbsdFile);
    err = volumeInfoReader->readVolumeInfo();
    setErrorCondition(err);
    int64_t dims[3];
    float res[3];
    volumeInfoReader->getDimsAndResolution(dims[0], dims[1], dims[2], res[0], res[1], res[2]);
    /* Sanity check what we are trying to load to make sure it can fit in our address space.
     * Note that this does not guarantee the user has enough left, just that the
     * size of the volume can fit in the address space of the program
     */
#if   (CMP_SIZEOF_SSIZE_T==4)
    int64_t max = std::numeric_limits<size_t>::max();
#else
    int64_t max = std::numeric_limits<int64_t>::max();
#endif
    if(dims[0] * dims[1] * dims[2] > max)
    {
      err = -1;
      std::stringstream s;
      s << "The total number of elements '" << (dims[0] * dims[1] * dims[2]) << "' is greater than this program can hold. Try the 64 bit version.";
      setErrorCondition(err);
      setErrorMessage(s.str());
      return;
    }

    if(dims[0] > max || dims[1] > max || dims[2] > max)
    {
      err = -1;
      std::stringstream s;
      s << "One of the dimensions is greater than the max index for this sysem. Try the 64 bit version.";
      s << " dim[0]=" << dims[0] << "  dim[1]=" << dims[1] << "  dim[2]=" << dims[2];
      setErrorCondition(err);
      setErrorMessage(s.str());
      return;
    }
    /* ************ End Sanity Check *************************** */
    size_t dcDims[3] =
    { dims[0], dims[1], dims[2] };
    m->setDimensions(dcDims);
    m->setResolution(res);
    //Now Calculate our "subvolume" of slices, ie, those start and end values that the user selected from the GUI
    // The GUI code has already added 1 to the end index so nothing special needs to be done
    // for this calculation
    dcDims[2] = m_ZEndIndex - m_ZStartIndex + 1;
    m->setDimensions(dcDims);
    manufacturer = volumeInfoReader->getManufacturer();
    volumeInfoReader = H5EbsdVolumeInfo::NullPointer();
  }
  H5EbsdVolumeReader::Pointer ebsdReader;
  std::vector<unsigned int> crystalStructures;
  if(manufacturer.compare(Ebsd::Ang::Manufacturer) == 0)
  {
    ebsdReader = H5AngVolumeReader::New();
    if(NULL == ebsdReader)
    {
      setErrorCondition(-1);
      setErrorMessage("Could not Create H5AngVolumeReader object.");
      return;
    }
    H5AngVolumeReader* angReader = dynamic_cast<H5AngVolumeReader*>(ebsdReader.get());
    err = loadInfo<H5AngVolumeReader, AngPhase>(angReader);
    if(err < 0)
    {
      setErrorCondition(-1);
      setErrorMessage("Could not read information about the Ebsd Volume.");
      return;
    }
  }
  else if(manufacturer.compare(Ebsd::Ctf::Manufacturer) == 0)
  {
    ebsdReader = H5CtfVolumeReader::New();
    if(NULL == ebsdReader)
    {
      setErrorCondition(-1);
      setErrorMessage("Could not Create H5CtfVolumeReader object.");
      return;
    }
    H5CtfVolumeReader* ctfReader = dynamic_cast<H5CtfVolumeReader*>(ebsdReader.get());
    err = loadInfo<H5CtfVolumeReader, CtfPhase>(ctfReader);
    if(err < 0)
    {
      setErrorCondition(-1);
      setErrorMessage("Could not read information about the Ebsd Volume.");
      return;
    }
  }
  else
  {
    setErrorCondition(-1);
    std::string msg("Could not determine or match a supported manufacturer from the data file.");
    msg = msg.append("Supported manufacturer codes are: ").append(Ebsd::Ctf::Manufacturer);
    msg = msg.append(" and ").append(Ebsd::Ang::Manufacturer);
    setErrorMessage(msg);
    return;
  }

  // This will create the arrays with the correct sizes
  dataCheck(false, m->totalPoints(), m->getTotalFields(), m->getNumEnsembleTuples());
  if(getErrorCondition() < 0)
  {
    return;
  }

  // Initialize all the arrays with some default values
  int64_t totalPoints = m->totalPoints();
  ss.str("");
  ss << getHumanLabel() << " - Initializing " << totalPoints << " voxels";
  notify(ss.str(), 0, Observable::UpdateProgressMessage);
  initializeArrays(totalPoints);

  ss.str("");
  ss << getHumanLabel() << " - Reading Ebsd Data from file";
  notify(ss.str(), 0, Observable::UpdateProgressMessage);
  ebsdReader->setSliceStart(m_ZStartIndex);
  ebsdReader->setSliceEnd(m_ZEndIndex);
  err = ebsdReader->loadData(m->getXPoints(), m->getYPoints(), m->getZPoints(), m_RefFrameZDir);
  if(err < 0)
  {
    setErrorCondition(err);
    setErrorMessage("Error Loading Data from Ebsd Data file.");
    return;
  }

  // The GoodVoxels array was just created so rerun the dataCheck to update the pointers
  dataCheck(false, m->totalPoints(), m->getTotalFields(), m->getNumEnsembleTuples());
  if(getErrorCondition() < 0)
  {
    return;
  }

  float* euler1Ptr = NULL;
  float* euler2Ptr = NULL;
  float* euler3Ptr = NULL;
  int* phasePtr = NULL;

  float radianconversion = 1.0f;
  if(manufacturer.compare(Ebsd::Ang::Manufacturer) == 0)
  {
    euler1Ptr = reinterpret_cast<float*>(ebsdReader->getPointerByName(Ebsd::Ang::Phi1));
    euler2Ptr = reinterpret_cast<float*>(ebsdReader->getPointerByName(Ebsd::Ang::Phi));
    euler3Ptr = reinterpret_cast<float*>(ebsdReader->getPointerByName(Ebsd::Ang::Phi2));
    phasePtr = reinterpret_cast<int*>(ebsdReader->getPointerByName(Ebsd::Ang::PhaseData));
  }
  else if(manufacturer.compare(Ebsd::Ctf::Manufacturer) == 0)
  {
    radianconversion = M_PI / 180.0;
    euler1Ptr = reinterpret_cast<float*>(ebsdReader->getPointerByName(Ebsd::Ctf::Euler1));
    euler2Ptr = reinterpret_cast<float*>(ebsdReader->getPointerByName(Ebsd::Ctf::Euler2));
    euler3Ptr = reinterpret_cast<float*>(ebsdReader->getPointerByName(Ebsd::Ctf::Euler3));
    phasePtr = reinterpret_cast<int*>(ebsdReader->getPointerByName(Ebsd::Ctf::Phase));
  }
  else
  {
    std::string msg("Could not determine or match a supported manufacturer from the data file.");
    msg = msg.append("Supported manufacturer codes are: ").append(Ebsd::Ctf::Manufacturer);
    msg = msg.append(" and ").append(Ebsd::Ang::Manufacturer);
    setErrorMessage(msg);
    return;
  }
  // Copy Euler Angles and Phases and possibly convert from degrees to radians
  for (int64_t i = 0; i < totalPoints; i++)
  {
    m_EulerAnglesC[3 * i] = euler1Ptr[i] * radianconversion;
    m_EulerAnglesC[3 * i + 1] = euler2Ptr[i] * radianconversion;
    m_EulerAnglesC[3 * i + 2] = euler3Ptr[i] * radianconversion;
    m_PhasesC[i] = phasePtr[i];
  }

  initializeQuats();

  // Run the filter to determine the good Voxels
  DetermineGoodVoxels::Pointer filter = DetermineGoodVoxels::New();
  filter->setQualityMetricFilters(m_QualityMetricFilters);
  filter->setObservers(getObservers());
  filter->setEbsdVolumeReader(ebsdReader);
  filter->setDataContainer(m);
  filter->execute();
  err = filter->getErrorCondition();
  if(err < 0)
  {
    setErrorCondition(err);
    setErrorMessage("Error Filtering Ebsd Data.");
    return;
  }
  filter = DetermineGoodVoxels::NullPointer(); // Clean up some memory

  // If there is an error set this to something negative and also set a message
  ss.str("");
  ss << getHumanLabel() << " Completed";
  notify(ss.str(), 0, Observable::UpdateProgressMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LoadSlices::initializeArrays(int64_t totalPoints)
{

  for(int i = 0;i < totalPoints;i++)
  {
    m_PhasesC[i] = 0;
    m_EulerAnglesC[3*i] = 0.0f;
    m_EulerAnglesC[3*i + 1] = 0.0f;
    m_EulerAnglesC[3*i + 2] = 0.0f;
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LoadSlices::initializeQuats()
{
  DataContainer* m = getDataContainer();
  int64_t totalPoints = m->totalPoints();

  float qr[5];
  unsigned int xtal = Ebsd::CrystalStructure::UnknownCrystalStructure;
  DataArray<unsigned int>* crystruct
      = DataArray<unsigned int>::SafeObjectDownCast<IDataArray*, DataArray<unsigned int>*>(m->getEnsembleData(DREAM3D::EnsembleData::CrystalStructures).get());
  int phase = -1;
  for (int i = 0; i < totalPoints; i++)
  {
    OrientationMath::eulertoQuat(qr, m_EulerAnglesC[3*i], m_EulerAnglesC[3*i + 1], m_EulerAnglesC[3*i + 2]);
    phase = m_PhasesC[i];
    xtal = crystruct->GetValue(phase);
    if (xtal == Ebsd::CrystalStructure::UnknownCrystalStructure)
    {
      qr[1] = 0.0;
      qr[2] = 0.0;
      qr[3] = 0.0;
      qr[4] = 1.0;
    }
    else
    {
      m_OrientationOps[xtal]->getFZQuat(qr);
    }

    m_Quats[i*5 + 0] = 1.0f;
    m_Quats[i*5 + 1] = qr[1];
    m_Quats[i*5 + 2] = qr[2];
    m_Quats[i*5 + 3] = qr[3];
    m_Quats[i*5 + 4] = qr[4];
  }
}

#if 0
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void LoadSlices::threshold_points()
{
  DataContainer* m = getDataContainer();
  int64_t totalPoints = m->totalPoints();

  size_t udims[3] = {0,0,0};
  m->getDimensions(udims);
#if (CMP_SIZEOF_SIZE_T == 4)
  typedef int32_t DimType;
#else
  typedef int64_t DimType;
#endif
  DimType dims[3] = {
    static_cast<DimType>(udims[0]),
    static_cast<DimType>(udims[1]),
    static_cast<DimType>(udims[2]),
  };

  DimType neighpoints[6];
  neighpoints[0] = -dims[0]*dims[1];
  neighpoints[1] = -dims[0];
  neighpoints[2] = -1;
  neighpoints[3] = 1;
  neighpoints[4] = dims[0];
  neighpoints[5] = dims[0]*dims[1];

  std::vector<bool> AlreadyChecked;
  float w, n1, n2, n3;
  float q1[5];
  float q2[5];
//  int index;
  int good = 0;
  size_t count = 0;
  int currentpoint = 0;
  int neighbor = 0;
  DimType col, row, plane;

//  int noborder = 0;

  DataArray<unsigned int>* crystruct
      = DataArray<unsigned int>::SafeObjectDownCast<IDataArray*, DataArray<unsigned int>*>(m->getEnsembleData(DREAM3D::EnsembleData::CrystalStructures).get());

  unsigned int phase1, phase2;
  int initialVoxelsListSize = 10000;
  std::vector<int> voxelslist(initialVoxelsListSize, -1);

  AlreadyChecked.resize(totalPoints);
  for (int iter = 0; iter < totalPoints; iter++)
  {
    AlreadyChecked[iter] = false;
    if(m_GoodVoxels[iter] == true && m_PhasesC[iter] > 0)
    {
      voxelslist[count] = iter;
      AlreadyChecked[iter] = true;
      count++;
      if(count >= voxelslist.size()) voxelslist.resize(count + initialVoxelsListSize, -1);
    }
  }
  for (size_t j = 0; j < count; j++)
  {
    currentpoint = voxelslist[j];
    col = currentpoint % m->getXPoints();
    row = (currentpoint / m->getXPoints()) % m->getYPoints();
    plane = currentpoint / (m->getXPoints() * m->getYPoints());
    q1[0] = 0;
    q1[1] = m_Quats[currentpoint * 5 + 1];
    q1[2] = m_Quats[currentpoint * 5 + 2];
    q1[3] = m_Quats[currentpoint * 5 + 3];
    q1[4] = m_Quats[currentpoint * 5 + 4];
    phase1 = crystruct->GetValue(m_PhasesC[currentpoint]);
    for (DimType i = 0; i < 6; i++)
    {
      good = 1;
      neighbor = currentpoint + neighpoints[i];
      if(i == 0 && plane == 0) good = 0;
      if(i == 5 && plane == (dims[2] - 1)) good = 0;
      if(i == 1 && row == 0) good = 0;
      if(i == 4 && row == (dims[1] - 1)) good = 0;
      if(i == 2 && col == 0) good = 0;
      if(i == 3 && col == (dims[0] - 1)) good = 0;
      if(good == 1 && m_GoodVoxels[neighbor] == false && m_PhasesC[neighbor] > 0)
      {
        w = 10000.0;
        q2[0] = 0;
        q2[1] = m_Quats[neighbor * 5 + 1];
        q2[2] = m_Quats[neighbor * 5 + 2];
        q2[3] = m_Quats[neighbor * 5 + 3];
        q2[4] = m_Quats[neighbor * 5 + 4];
        phase2 = crystruct->GetValue(m_PhasesC[neighbor]);
        if(phase1 == phase2)
        {
          w = m_OrientationOps[phase1]->getMisoQuat(q1, q2, n1, n2, n3);
        }
        if(w < m_MisorientationTolerance)
        {
          m_GoodVoxels[neighbor] = true;
          AlreadyChecked[neighbor] = true;
          voxelslist[count] = neighbor;
          count++;
          if(count >= voxelslist.size()) voxelslist.resize(count + initialVoxelsListSize, -1);
        }
      }
    }
  }
  voxelslist.clear();
}
#endif
