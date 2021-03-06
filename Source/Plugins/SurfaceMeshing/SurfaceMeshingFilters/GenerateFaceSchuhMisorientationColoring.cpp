/* ============================================================================
 * Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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
#include "GenerateFaceSchuhMisorientationColoring.h"

#ifdef DREAM3D_USE_PARALLEL_ALGORITHMS
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/partitioner.h>
#endif


#include "DREAM3DLib/Math/DREAM3DMath.h"
#include "DREAM3DLib/Math/MatrixMath.h"
#include "SurfaceMeshing/SurfaceMeshingFilters/util/Vector3.h"
#include "SurfaceMeshing/SurfaceMeshingFilters/util/TriangleOps.h"

#include "OrientationLib/OrientationOps/CubicOps.h"
#include "OrientationLib/OrientationOps/CubicLowOps.h"
#include "OrientationLib/OrientationOps/HexagonalOps.h"
#include "OrientationLib/OrientationOps/HexagonalLowOps.h"
#include "OrientationLib/OrientationOps/TrigonalOps.h"
#include "OrientationLib/OrientationOps/TrigonalLowOps.h"
#include "OrientationLib/OrientationOps/TetragonalOps.h"
#include "OrientationLib/OrientationOps/TetragonalLowOps.h"
#include "OrientationLib/OrientationOps/OrthoRhombicOps.h"
#include "OrientationLib/OrientationOps/MonoclinicOps.h"
#include "OrientationLib/OrientationOps/TriclinicOps.h"
#include "DREAM3DLib/Utilities/ColorTable.h"


class CalculateFaceSchuhMisorientationColorsImpl
{
    int32_t* m_Labels;
    int32_t* m_Phases;
    float* m_Quats;
    uint8_t* m_Colors;
    unsigned int* m_CrystalStructures;
    QVector<OrientationOps::Pointer> m_OrientationOps;

  public:
    CalculateFaceSchuhMisorientationColorsImpl(int32_t* labels, int32_t* phases, float* quats, uint8_t* colors, unsigned int* crystalStructures) :
      m_Labels(labels),
      m_Phases(phases),
      m_Quats(quats),
      m_Colors(colors),
      m_CrystalStructures(crystalStructures)
    {
      m_OrientationOps = OrientationOps::getOrientationOpsQVector();
    }
    virtual ~CalculateFaceSchuhMisorientationColorsImpl() {}

    /**
     * @brief generate Generates the colors for the triangles
     * @param start The starting DREAM3D::SurfaceMesh::Face_t Index
     * @param end The ending DREAM3D::SurfaceMesh::Face_t Index
     */
    void generate(size_t start, size_t end) const
    {
      QVector<OrientationOps::Pointer> ops = OrientationOps::getOrientationOpsQVector();
      DREAM3D::Rgb argb = 0x00000000;

      int grain1, grain2, phase1, phase2;
      QuatF q1;
      QuatF q2;
      QuatF* quats = reinterpret_cast<QuatF*>(m_Quats);

      for (size_t i = start; i < end; i++)
      {
        grain1 = m_Labels[2 * i];
        grain2 = m_Labels[2 * i + 1];
        if(grain1 > 0) { phase1 = m_Phases[grain1]; }
        else { phase1 = 0; }
        if(grain2 > 0) { phase2 = m_Phases[grain2]; }
        else { phase2 = 0; }

        m_Colors[3 * i + 0] = 0;
        m_Colors[3 * i + 1] = 0;
        m_Colors[3 * i + 2] = 0;

        if(phase1 > 0)
        {
          if(phase1 == phase2)
          {
            if(m_CrystalStructures[phase1] == Ebsd::CrystalStructure::Cubic_High)
            {
              QuaternionMathF::Copy(quats[grain1], q1);
              QuaternionMathF::Copy(quats[grain2], q2);
              argb = ops[m_CrystalStructures[phase1]]->generateMisorientationColor(q1, q2);
              m_Colors[3 * i] = RgbColor::dRed(argb);
              m_Colors[3 * i + 1] = RgbColor::dGreen(argb);
              m_Colors[3 * i + 2] = RgbColor::dBlue(argb);
            }
          }
        }
      }
    }

#ifdef DREAM3D_USE_PARALLEL_ALGORITHMS
    /**
     * @brief operator () This is called from the TBB stye of code
     * @param r The range to compute the values
     */
    void operator()(const tbb::blocked_range<size_t>& r) const
    {
      generate(r.begin(), r.end());
    }
#endif


};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
GenerateFaceSchuhMisorientationColoring::GenerateFaceSchuhMisorientationColoring() :
  SurfaceMeshFilter(),
  m_SurfaceMeshFaceLabelsArrayPath(DREAM3D::Defaults::SurfaceDataContainerName, DREAM3D::Defaults::FaceAttributeMatrixName, DREAM3D::FaceData::SurfaceMeshFaceLabels),
  m_AvgQuatsArrayPath(DREAM3D::Defaults::VolumeDataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, DREAM3D::FeatureData::AvgQuats),
  m_FeaturePhasesArrayPath(DREAM3D::Defaults::VolumeDataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, DREAM3D::FeatureData::Phases),
  m_CrystalStructuresArrayPath(DREAM3D::Defaults::VolumeDataContainerName, DREAM3D::Defaults::CellEnsembleAttributeMatrixName, DREAM3D::EnsembleData::CrystalStructures),
  m_SurfaceMeshFaceSchuhMisorientationColorsArrayName(DREAM3D::FaceData::SurfaceMeshFaceSchuhMisorientationColors),
  m_SurfaceMeshFaceLabelsArrayName(DREAM3D::FaceData::SurfaceMeshFaceLabels),
  m_SurfaceMeshFaceLabels(NULL),
  m_SurfaceMeshFaceSchuhMisorientationColors(NULL),
  m_AvgQuatsArrayName(DREAM3D::FeatureData::AvgQuats),
  m_AvgQuats(NULL),
  m_FeaturePhasesArrayName(DREAM3D::FeatureData::Phases),
  m_FeaturePhases(NULL),
  m_CrystalStructuresArrayName(DREAM3D::EnsembleData::CrystalStructures),
  m_CrystalStructures(NULL)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
GenerateFaceSchuhMisorientationColoring::~GenerateFaceSchuhMisorientationColoring()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GenerateFaceSchuhMisorientationColoring::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(FilterParameter::New("Required Information", "", FilterParameterWidgetType::SeparatorWidget, "", true));
  parameters.push_back(FilterParameter::New("SurfaceMeshFaceLabels", "SurfaceMeshFaceLabelsArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getSurfaceMeshFaceLabelsArrayPath(), true, ""));
  parameters.push_back(FilterParameter::New("AvgQuats", "AvgQuatsArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getAvgQuatsArrayPath(), true, ""));
  parameters.push_back(FilterParameter::New("FeaturePhases", "FeaturePhasesArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getFeaturePhasesArrayPath(), true, ""));
  parameters.push_back(FilterParameter::New("CrystalStructures", "CrystalStructuresArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getCrystalStructuresArrayPath(), true, ""));
  parameters.push_back(FilterParameter::New("Created Information", "", FilterParameterWidgetType::SeparatorWidget, "", true));
  parameters.push_back(FilterParameter::New("SurfaceMeshFaceSchuhMisorientationColors", "SurfaceMeshFaceSchuhMisorientationColorsArrayName", FilterParameterWidgetType::StringWidget, getSurfaceMeshFaceSchuhMisorientationColorsArrayName(), true, ""));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GenerateFaceSchuhMisorientationColoring::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSurfaceMeshFaceSchuhMisorientationColorsArrayName(reader->readString("SurfaceMeshFaceSchuhMisorientationColorsArrayName", getSurfaceMeshFaceSchuhMisorientationColorsArrayName() ) );
  setCrystalStructuresArrayPath(reader->readDataArrayPath("CrystalStructuresArrayPath", getCrystalStructuresArrayPath() ) );
  setFeaturePhasesArrayPath(reader->readDataArrayPath("FeaturePhasesArrayPath", getFeaturePhasesArrayPath() ) );
  setAvgQuatsArrayPath(reader->readDataArrayPath("AvgQuatsArrayPath", getAvgQuatsArrayPath() ) );
  setSurfaceMeshFaceLabelsArrayPath(reader->readDataArrayPath("SurfaceMeshFaceLabelsArrayPath", getSurfaceMeshFaceLabelsArrayPath() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int GenerateFaceSchuhMisorientationColoring::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  DREAM3D_FILTER_WRITE_PARAMETER(SurfaceMeshFaceSchuhMisorientationColorsArrayName)
  DREAM3D_FILTER_WRITE_PARAMETER(CrystalStructuresArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(FeaturePhasesArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(AvgQuatsArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(SurfaceMeshFaceLabelsArrayPath)
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GenerateFaceSchuhMisorientationColoring::dataCheckSurfaceMesh()
{
  DataArrayPath tempPath;
  setErrorCondition(0);

  SurfaceDataContainer* sm = getDataContainerArray()->getPrereqDataContainer<SurfaceDataContainer, AbstractFilter>(this, m_SurfaceMeshFaceLabelsArrayPath.getDataContainerName(), false);
  if(getErrorCondition() < 0) { return; }

  // We MUST have Nodes
  if(sm->getVertices().get() == NULL)
  {
    setErrorCondition(-384);
    notifyErrorMessage(getHumanLabel(), "SurfaceMesh DataContainer missing Nodes", getErrorCondition());
  }

  // We MUST have Triangles defined also.
  if(sm->getFaces().get() == NULL)
  {
    setErrorCondition(-385);
    notifyErrorMessage(getHumanLabel(), "SurfaceMesh DataContainer missing Triangles", getErrorCondition());
  }
  else
  {
    QVector<size_t> dims(1, 2);
    m_SurfaceMeshFaceLabelsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter>(this, getSurfaceMeshFaceLabelsArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
    if( NULL != m_SurfaceMeshFaceLabelsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
    { m_SurfaceMeshFaceLabels = m_SurfaceMeshFaceLabelsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
    dims[0] = 3;
    tempPath.update(m_SurfaceMeshFaceLabelsArrayPath.getDataContainerName(), m_SurfaceMeshFaceLabelsArrayPath.getAttributeMatrixName(), getSurfaceMeshFaceSchuhMisorientationColorsArrayName() );
    m_SurfaceMeshFaceSchuhMisorientationColorsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<uint8_t>, AbstractFilter, uint8_t>(this, tempPath, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
    if( NULL != m_SurfaceMeshFaceSchuhMisorientationColorsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
    { m_SurfaceMeshFaceSchuhMisorientationColors = m_SurfaceMeshFaceSchuhMisorientationColorsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GenerateFaceSchuhMisorientationColoring::dataCheckVoxel()
{
  DataArrayPath tempPath;
  setErrorCondition(0);

  QVector<size_t> dims(1, 4);
  m_AvgQuatsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>, AbstractFilter>(this, getAvgQuatsArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_AvgQuatsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_AvgQuats = m_AvgQuatsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  dims[0] = 1;
  m_FeaturePhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter>(this, getFeaturePhasesArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_FeaturePhasesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_FeaturePhases = m_FeaturePhasesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
//typedef DataArray<unsigned int> XTalStructArrayType;
  m_CrystalStructuresPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<unsigned int>, AbstractFilter>(this, getCrystalStructuresArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_CrystalStructuresPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_CrystalStructures = m_CrystalStructuresPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GenerateFaceSchuhMisorientationColoring::preflight()
{
  setInPreflight(true);
  emit preflightAboutToExecute();
  emit updateFilterParameters(this);
  dataCheckSurfaceMesh();
  dataCheckVoxel();
  emit preflightExecuted();
  setInPreflight(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GenerateFaceSchuhMisorientationColoring::execute()
{
  int err = 0;
  setErrorCondition(err);
  dataCheckSurfaceMesh();
  if(getErrorCondition() < 0) { return; }

  dataCheckVoxel();
  if(getErrorCondition() < 0) { return; }

  //SurfaceDataContainer* sm = getDataContainerArray()->getDataContainerAs<SurfaceDataContainer>(m_SurfaceMeshFaceLabelsArrayPath.getDataContainerName());
  notifyStatusMessage(getMessagePrefix(), getHumanLabel(), "Starting");

  // Run the data check to allocate the memory for the centroid array
  int64_t numTriangles = m_SurfaceMeshFaceLabelsPtr.lock()->getNumberOfTuples();

#ifdef DREAM3D_USE_PARALLEL_ALGORITHMS
  bool doParallel = true;
#endif

#ifdef DREAM3D_USE_PARALLEL_ALGORITHMS
  if (doParallel == true)
  {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, numTriangles),
                      CalculateFaceSchuhMisorientationColorsImpl(m_SurfaceMeshFaceLabels, m_FeaturePhases, m_AvgQuats, m_SurfaceMeshFaceSchuhMisorientationColors, m_CrystalStructures), tbb::auto_partitioner());
  }
  else
#endif
  {
    CalculateFaceSchuhMisorientationColorsImpl serial(m_SurfaceMeshFaceLabels, m_FeaturePhases, m_AvgQuats, m_SurfaceMeshFaceSchuhMisorientationColors, m_CrystalStructures);
    serial.generate(0, numTriangles);
  }

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer GenerateFaceSchuhMisorientationColoring::newFilterInstance(bool copyFilterParameters)
{
  GenerateFaceSchuhMisorientationColoring::Pointer filter = GenerateFaceSchuhMisorientationColoring::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString GenerateFaceSchuhMisorientationColoring::getCompiledLibraryName()
{ return SurfaceMeshing::SurfaceMeshingBaseName; }


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString GenerateFaceSchuhMisorientationColoring::getGroupName()
{ return DREAM3D::FilterGroups::SurfaceMeshingFilters; }


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString GenerateFaceSchuhMisorientationColoring::getSubGroupName()
{ return DREAM3D::FilterSubGroups::MiscFilters; }


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString GenerateFaceSchuhMisorientationColoring::getHumanLabel()
{ return "Generate Face Schuh Misorientation Colors"; }

