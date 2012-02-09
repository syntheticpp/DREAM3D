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

#include "FindMDF.h"

#include "DREAM3DLib/Common/DREAM3DMath.h"
#include "DREAM3DLib/Common/Constants.h"

#include "DREAM3DLib/PrivateFilters/FindNeighbors.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindMDF::FindMDF()  :
AbstractFilter(),
m_CreateNewStatsFile(true),
m_AvgQuats(NULL),
m_SurfaceFields(NULL),
m_PhasesF(NULL),
m_TotalSurfaceAreas(NULL),
m_NeighborList(NULL),
m_SharedSurfaceAreaList(NULL)
{
  m_HexOps = HexagonalOps::New();
  m_OrientationOps.push_back(dynamic_cast<OrientationMath*> (m_HexOps.get()));

  m_CubicOps = CubicOps::New();
  m_OrientationOps.push_back(dynamic_cast<OrientationMath*> (m_CubicOps.get()));

  m_OrthoOps = OrthoRhombicOps::New();
  m_OrientationOps.push_back(dynamic_cast<OrientationMath*> (m_OrthoOps.get()));
  setupFilterOptions();

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindMDF::~FindMDF()
{
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMDF::setupFilterOptions()
{
  std::vector<FilterOption::Pointer> options;
  {
    FilterOption::Pointer option = FilterOption::New();
    option->setHumanLabel("Output Statistics File");
    option->setPropertyName("H5StatsFile");
    option->setWidgetType(FilterOption::OutputFileWidget);
    option->setValueType("string");
    options.push_back(option);
  }
  setFilterOptions(options);

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMDF::dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles)
{
  setErrorCondition(0);
  std::stringstream ss;
  DataContainer* m = getDataContainer();

  GET_PREREQ_DATA(m, DREAM3D, FieldData, AvgQuats, ss, -301, float, FloatArrayType, fields, 5);
  GET_PREREQ_DATA(m, DREAM3D, FieldData, SurfaceFields, ss, -303, bool, BoolArrayType, fields, 1);
  GET_PREREQ_DATA_SUFFIX(m, DREAM3D, FieldData, Phases, F, ss, -303,  int32_t, Int32ArrayType, fields, 1);

  // Now we are going to get a "Pointer" to the NeighborList object out of the DataContainer
   m_NeighborList = NeighborList<int>::SafeObjectDownCast<IDataArray*, NeighborList<int>* >
                                           (m->getFieldData(DREAM3D::FieldData::NeighborList).get());
   if(m_NeighborList == NULL)
   {
     ss << "NeighborLists Array Not Initialized At Beginning of MatchCrystallography Filter" << std::endl;
     setErrorCondition(-308);
   }

   // And we do the same for the SharedSurfaceArea list
   m_SharedSurfaceAreaList = NeighborList<float>::SafeObjectDownCast<IDataArray*, NeighborList<float>*>
                                  (m->getFieldData(DREAM3D::FieldData::SharedSurfaceAreaList).get());
   if(m_SharedSurfaceAreaList == NULL)
   {
     ss << "SurfaceAreaLists Array Not Initialized At Beginning of MatchCrystallography Filter" << std::endl;
     setErrorCondition(-309);
   }

  GET_PREREQ_DATA(m, DREAM3D, EnsembleData, TotalSurfaceAreas, ss, -303,  float, FloatArrayType, ensembles, 1);


  setErrorMessage(ss.str());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMDF::preflight()
{
  dataCheck(true, 1, 1, 1);
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMDF::execute()
{
  DataContainer* m = getDataContainer();
  if (NULL == m)
  {
    setErrorCondition(-1);
    std::stringstream ss;
    ss << getNameOfClass() << " DataContainer was NULL";
    setErrorMessage(ss.str());
    return;
  }
  setErrorCondition(0);


  H5StatsWriter::Pointer h5io = H5StatsWriter::New(getH5StatsFile(), m_CreateNewStatsFile);

  dataCheck(false, m->totalPoints(), m->getTotalFields(), m->getNumEnsembleTuples());
  if (getErrorCondition() < 0)
  {
    return;
  }


  // But since a pointer is difficult to use operators with we will now create a
  // reference variable to the pointer with the correct variable name that allows
  // us to use the same syntax as the "vector of vectors"
  NeighborList<int>& neighborlist = *m_NeighborList;
  // And we do the same for the SharedSurfaceArea list
  NeighborList<float>& neighborsurfacearealist = *m_SharedSurfaceAreaList;

  float n1, n2, n3;
  float r1, r2, r3;
  int mbin;
  float w;
  float q1[5];
  float q2[5];
  size_t numgrains = m->getTotalFields();
  unsigned int phase1, phase2;
  float **misobin;
  int numbins = 0;

  typedef DataArray<unsigned int> XTalType;
  XTalType* crystructPtr
      = XTalType::SafeObjectDownCast<IDataArray*, XTalType*>(m->getEnsembleData(DREAM3D::EnsembleData::CrystalStructures).get());
  unsigned int* crystruct = crystructPtr->GetPointer(0);
  size_t numXTals = crystructPtr->GetNumberOfTuples();

  misobin = new float *[numXTals];
  for(size_t i=1;i<numXTals;i++)
  {
    if (crystruct[i] == Ebsd::CrystalStructure::Hexagonal)
    {
      numbins = 36 * 36 * 12;
      misobin[i] = new float[numbins];
    }
    else if (crystruct[i] == Ebsd::CrystalStructure::Cubic)
    {
      numbins = 18 * 18 * 18;
      misobin[i] = new float[numbins];
    }
    // Now initialize all bins to 0.0
    for (int j = 0; j < numbins; j++)
    {
      misobin[i][j] = 0.0;
    }
  }
  size_t nname;
  float nsa;
  misorientationlists.resize(numgrains);
  for (size_t i = 1; i < numgrains; i++)
  {
		q1[0] = m_AvgQuats[3*i] / m_AvgQuats[3*i];
		q1[1] = m_AvgQuats[3*i+1] / m_AvgQuats[3*i+1];
		q1[2] = m_AvgQuats[3*i+2] / m_AvgQuats[3*i+2];
		q1[3] = m_AvgQuats[3*i+3] / m_AvgQuats[3*i+3];
		q1[4] = m_AvgQuats[3*i+4] / m_AvgQuats[3*i+4];
		phase1 = crystruct[m_PhasesF[i]];
		misorientationlists[i].resize(neighborlist[i].size() * 3, -1.0);
		for (size_t j = 0; j < neighborlist[i].size(); j++)
		{
		  w = 10000.0;
		  nname = neighborlist[i][j];
		  q2[0] = m_AvgQuats[3*nname] / m_AvgQuats[3*nname];
		  q2[1] = m_AvgQuats[3*nname+1] / m_AvgQuats[3*nname+1];
		  q2[2] = m_AvgQuats[3*nname+2] / m_AvgQuats[3*nname+2];
		  q2[3] = m_AvgQuats[3*nname+3] / m_AvgQuats[3*nname+3];
		  q2[4] = m_AvgQuats[3*nname+4] / m_AvgQuats[3*nname+4];
		  phase2 = crystruct[m_PhasesF[nname]];
		  if (phase1 == phase2) w = m_OrientationOps[phase1]->getMisoQuat( q1, q2, n1, n2, n3);
		  if (phase1 == phase2)
		  {
			OrientationMath::axisAngletoHomochoric(w, n1, n2, n3, r1, r2, r3);
			misorientationlists[i][3 * j] = r1;
			misorientationlists[i][3 * j + 1] = r2;
			misorientationlists[i][3 * j + 2] = r3;
		  }
		  if (phase1 != phase2)
		  {
			misorientationlists[i][3 * j] = -100;
			misorientationlists[i][3 * j + 1] = -100;
			misorientationlists[i][3 * j + 2] = -100;
		  }
		  if (phase1 == phase2) mbin = m_OrientationOps[phase1]->getMisoBin(
																   misorientationlists[i][3*j],
																   misorientationlists[i][3 * j + 1],
																   misorientationlists[i][3 * j + 2]);
		  if ((nname > i || m_SurfaceFields[nname] == true) && phase1 == phase2)
		  {
			nsa = neighborsurfacearealist[i][j];
			misobin[m_PhasesF[i]][mbin] = misobin[m_PhasesF[i]][mbin] + (nsa / m_TotalSurfaceAreas[m_PhasesF[i]]);
		  }
		}
  }
  unsigned long long int dims = static_cast<unsigned long long int>(numbins);
  for (size_t i = 1; i < numXTals; i++)
  {
	  h5io->writeMisorientationBinsData(i, &dims, misobin[i]);
	  delete[] misobin[i];
  }
  delete[] misobin;

  notify("FindMDF Completed", 0, Observable::UpdateProgressMessage);
}
