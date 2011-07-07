/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Grober (US Air Force Research Laboratories
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
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
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
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "H5AngDataLoader.h"

#include <cmath>


#include "MXA/HDF5/H5Lite.h"
#include "MXA/HDF5/H5Utilities.h"
#include "MXA/Utilities/StringUtils.h"

#include "TSLLib/H5AngReader.h"

#include "DREAM3D/Common/Constants.h"
#include "DREAM3D/Common/OrientationMath.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5AngDataLoader::H5AngDataLoader() :
m_Orientation(Ang::NoOrientation)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5AngDataLoader::~H5AngDataLoader()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5AngDataLoader::readZHeader(int &zStart, int &zEnd, float &zRes)
{
  int err = -1;
  int retErr = 0;
  hid_t fileId = H5Utilities::openFile(m_Filename, true);
  if (fileId < 0)
  {
    std::cout << "Error Opening file '" << m_Filename << "'" << std::endl;
    return -1;
  }
  err = H5Lite::readScalarDataset(fileId, Ang::ZResolution, zRes);
  if (err < 0)
  {
    std::cout << "H5AngDataLoader::readZHeader Error: Could not load ZResolution data" << std::endl;
    retErr = err;
  }
  err = H5Lite::readScalarDataset(fileId, Ang::ZStartIndex, zStart);
  if (err < 0)
  {
    std::cout << "H5AngDataLoader::readZHeader Error: Could not load ZStartIndex data" << std::endl;
    retErr = err;
  }
  err = H5Lite::readScalarDataset(fileId, Ang::ZEndIndex, zEnd);
  if (err < 0)
  {
    std::cout << "H5AngDataLoader::readZHeader Error: Could not load ZEndIndex data" << std::endl;
    retErr = err;
  }

  err = H5Fclose(fileId);
  return retErr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5AngDataLoader::getSizeAndResolution(int &xpoints, int &ypoints, int &zpoints,
                                    float &xres, float &yres, float &zres)
{
  int err = -1;
  zpoints = (getZEndIndex() - getZStartIndex() );
  int xpointstemp = 0;
  int ypointstemp = 0;
  xpoints = 0;
  ypoints = 0;

  hid_t fileId = H5Utilities::openFile(m_Filename, true);
  if (fileId < 0)
  {
    std::cout << "Error - Could not open HDF5 based OIM file" << std::endl;
    return -1;
  }


  zres = 0.0f;
  err = H5Lite::readScalarDataset(fileId, Ang::ZResolution, zres);

  for(int i=0;i<zpoints;i++)
  {
    std::string index = StringUtils::numToString(i + getZStartIndex());
    hid_t gid = H5Gopen(fileId, index.c_str());
    H5AngReader::Pointer reader = H5AngReader::New();
    reader->setHDF5Path(index);
    err = reader->readHeader(gid);
    if (err < 0)
    {
      std::cout << "Error reading the .HDF5 Ang Header data" << std::endl;
      err = H5Gclose(gid);
      err = H5Fclose(fileId);
      return -1;
    }
    xres = reader->getXStep();
    yres = reader->getYStep();
    xpointstemp = reader->getNumEvenCols();
    ypointstemp = reader->getNumRows();
    if(xpointstemp > xpoints) xpoints = xpointstemp;
    if(ypointstemp > ypoints) ypoints = ypointstemp;
    err = H5Gclose(gid);
  }
  err = H5Fclose(fileId);

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<AngPhase::Pointer> H5AngDataLoader::getPhases()
{
  m_Phases.clear();
  hid_t fileId = H5Utilities::openFile(m_Filename, true);
  if (fileId < 0)
  {
    std::cout << "Error" << std::endl;
    return m_Phases;
  }
  herr_t err = 0;

  std::string index = StringUtils::numToString(getZStartIndex());
  hid_t gid = H5Gopen(fileId, index.c_str());
  H5AngReader::Pointer reader = H5AngReader::New();
  reader->setHDF5Path(index);
  err = reader->readHeader(gid);
  m_Phases = reader->getPhases();
  if (err < 0)
  {
    std::cout << "Error reading the .HDF5 Ang Header data" << std::endl;
    err = H5Gclose(gid);
    err = H5Fclose(fileId);
    return m_Phases;
  }
  return m_Phases;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5AngDataLoader::loadData(ReconstructionFunc* m)
{
  int index = 0;
  int err = -1;

  int readerIndex;
  int xpointstemp;
  int ypointstemp;
  float* euler1Ptr;
  float* euler2Ptr;
  float* euler3Ptr;
  float* confPtr;
  int* phasePtr;
  float* imqualPtr;
  float* imqual2Ptr;
  int xstartspot;
  int ystartspot;

  for (int slice = 0; slice < zpoints; ++slice)
  {
    H5AngReader::Pointer reader = H5AngReader::New();
    reader->setFileName(getFilename());
    reader->setHDF5Path(StringUtils::numToString(slice + m_ZStartIndex));
    reader->setUserOrigin(m_Orientation);

    err = reader->readFile();
    if (err < 0)
    {
      std::cout << "H5AngDataLoader Error: There was an issue loading the data from the hdf5 file." << std::endl;
      return -1;
    }
    readerIndex = 0;
    xpointstemp = reader->getNumEvenCols();
    ypointstemp = reader->getNumRows();
    euler1Ptr = reader->getPhi1Pointer();
    euler2Ptr = reader->getPhiPointer();
    euler3Ptr = reader->getPhi2Pointer();
    phasePtr = reader->getPhasePointer();
    confPtr = reader->getConfidenceIndexPointer();
    imqualPtr = reader->getImageQualityPointer();
    imqual2Ptr = reader->getImageQuality2Pointer();
    xstartspot = (xpoints - xpointstemp) / 2;
    ystartspot = (ypoints - ypointstemp) / 2;
    for (int j = 0; j < ypointstemp; j++)
    {
      for (int i = 0; i < xpointstemp; i++)
      {
        index = ((zpoints - 1 - slice) * xpoints * ypoints) + ((j + ystartspot) * xpoints) + (i + xstartspot);
        m->euler1s[index] = euler1Ptr[readerIndex]; // Phi1
        m->euler2s[index] = euler2Ptr[readerIndex]; // Phi
        m->euler3s[index] = euler3Ptr[readerIndex]; // Phi2
        m->imagequalities[index] = imqualPtr[readerIndex]; // Image Quality
        m->confidences[index] = confPtr[readerIndex]; // Confidence
        m->phases[index] = phasePtr[readerIndex]; // Phase
        /* For TSL OIM Files if there is a single phase then the value of the phase
         * data is zero (0). If there are 2 or more phases then the lowest value
         * of phase is one (1). In the rest of the reconstruction code we follow the
         * convention that the lowest value is One (1) even if there is only a single
         * phase. The next if statement converts all zeros to ones if there is a single
         * phase in the OIM data.
         */
        if (m->phases[index] < 1) {
          phases[index] = 1;
        }
        ++readerIndex;
      }
    }
  }
  return err;
}


