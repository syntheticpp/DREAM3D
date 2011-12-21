/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories)
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

#ifndef _RECONSTRUCTION_H_
#define _RECONSTRUCTION_H_

#if defined (_MSC_VER)
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <vector>

#include <MXA/Common/MXASetGetMacros.h>
#include "MXA/MXA.h"

#include "EbsdLib/EbsdConstants.h"
#include "EbsdLib/TSL/AngConstants.h"
#include "EbsdLib/HKL/CtfConstants.h"
#include "EbsdLib/QualityMetricFilter.h"

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/Common/AbstractPipeline.h"
#include "DREAM3DLib/Common/Observer.h"


#include "DREAM3DLib/Common/DataContainer.h"


/**
* @class Reconstruction Reconstruction Plugins/Reconstruction/Reconstruction.h
* @brief This class serves as the main entry point to execute the reconstruction codes
*
* @author Michael A. Jackson for BlueQuartz Software
* @author Dr. Michael Groeber, US AFRL
* @date Nov 3, 2009
* @version 1.0
*/
class Reconstruction : public AbstractPipeline, public Observer
{
  public:
    MXA_SHARED_POINTERS(Reconstruction);
    MXA_TYPE_MACRO(Reconstruction);
    MXA_STATIC_NEW_MACRO(Reconstruction);

    virtual ~Reconstruction();


    MXA_INSTANCE_STRING_PROPERTY(H5AngFile)
    MXA_INSTANCE_PROPERTY(int, ZStartIndex)
    MXA_INSTANCE_PROPERTY(int, ZEndIndex)
    MXA_INSTANCE_PROPERTY(std::vector<DREAM3D::Reconstruction::PhaseType>, PhaseTypes)
    MXA_INSTANCE_STRING_PROPERTY(OutputDirectory)
    MXA_INSTANCE_STRING_PROPERTY(OutputFilePrefix)
    MXA_INSTANCE_PROPERTY(bool, MergeTwins)
    MXA_INSTANCE_PROPERTY(bool, MergeColonies)
    MXA_INSTANCE_PROPERTY(int32_t, MinAllowedGrainSize)

    MXA_INSTANCE_PROPERTY(double, MisorientationTolerance)
    MXA_INSTANCE_PROPERTY(DREAM3D::Reconstruction::AlignmentMethod, AlignmentMethod)
    MXA_INSTANCE_PROPERTY(Ebsd::RefFrameZDir, RefFrameZDir)
    MXA_INSTANCE_PROPERTY(bool, ReorderArray)
    MXA_INSTANCE_PROPERTY(bool, RotateSlice)

    MXA_INSTANCE_PROPERTY(bool, WriteBinaryVTKFiles)
    MXA_INSTANCE_PROPERTY(bool, WriteVtkFile)
    MXA_INSTANCE_PROPERTY(bool, WriteGoodVoxels)
    MXA_INSTANCE_PROPERTY(bool, WritePhaseId)
    MXA_INSTANCE_PROPERTY(bool, WriteIPFColor)
    MXA_INSTANCE_PROPERTY(bool, WriteGoodVoxels)

    MXA_INSTANCE_PROPERTY(bool, WriteHDF5GrainFile)

    MXA_INSTANCE_PROPERTY(std::vector<QualityMetricFilter::Pointer>, QualityMetricFilters)

    /**
    * @brief Main method to run the operation
    */
    virtual void execute();

    /**
     * @brief This will output the settings that this class is using to the std::ostream object
     * @param stream The std::ostream object to print to
     */
    virtual void printSettings(std::ostream &stream);

  protected:
    Reconstruction();


  private:
    DataContainer::Pointer m;

    Reconstruction(const Reconstruction&);    // Copy Constructor Not Implemented
    void operator=(const Reconstruction&);  // Operator '=' Not Implemented

};

#endif /* RECONSTRUCTION_H_ */
