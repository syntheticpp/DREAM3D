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

#ifndef _FindBoundaryCellFractions_H_
#define _FindBoundaryCellFractions_H_

#include <vector>
#include <QtCore/QString>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/DataArrays/IDataArray.h"

#include "DREAM3DLib/Common/AbstractFilter.h"
#include "DREAM3DLib/DataContainers/VolumeDataContainer.h"

#include "Statistics/StatisticsConstants.h"
/**
 * @class FindBoundaryCellFractions FindBoundaryCellFractions.h DREAM3DLib/GenericFilters/FindBoundaryCellFractions.h
 * @brief
 * @author
 * @date Nov 19, 2011
 * @version 1.0
 */
class FindBoundaryCellFractions : public AbstractFilter
{
    Q_OBJECT /* Need this for Qt's signals and slots mechanism to work */
  public:
    DREAM3D_SHARED_POINTERS(FindBoundaryCellFractions)
    DREAM3D_STATIC_NEW_MACRO(FindBoundaryCellFractions)
    DREAM3D_TYPE_MACRO_SUPER(FindBoundaryCellFractions, AbstractFilter)

    virtual ~FindBoundaryCellFractions();
    DREAM3D_FILTER_PARAMETER(DataArrayPath, CellFeatureAttributeMatrixName)
    Q_PROPERTY(DataArrayPath CellFeatureAttributeMatrixName READ getCellFeatureAttributeMatrixName WRITE setCellFeatureAttributeMatrixName)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, FeatureIdsArrayPath)
    Q_PROPERTY(DataArrayPath FeatureIdsArrayPath READ getFeatureIdsArrayPath WRITE setFeatureIdsArrayPath)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, BoundaryCellsArrayPath)
    Q_PROPERTY(DataArrayPath BoundaryCellsArrayPath READ getBoundaryCellsArrayPath WRITE setBoundaryCellsArrayPath)

    DREAM3D_FILTER_PARAMETER(QString, BoundaryCellFractionsArrayName)
    Q_PROPERTY(QString BoundaryCellFractionsArrayName READ getBoundaryCellFractionsArrayName WRITE setBoundaryCellFractionsArrayName)

    virtual const QString getCompiledLibraryName() { return Statistics::StatisticsBaseName; }
    virtual AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters);
    virtual const QString getGroupName() { return DREAM3D::FilterGroups::StatisticsFilters; }
    virtual const QString getSubGroupName() { return DREAM3D::FilterSubGroups::MorphologicalFilters; }
    virtual const QString getHumanLabel() { return "Find Feature Boundary Cell Fractions"; }

    /**
    * @brief This method will instantiate all the end user settable options/parameters
    * for this filter
    */
    virtual void setupFilterParameters();

    /**
     * @brief Reimplemented from @see AbstractFilter class
     */
    virtual int writeFilterParameters(AbstractFilterParametersWriter* writer, int index);

    /**
    * @brief This method will read the options from a file
    * @param reader The reader that is used to read the options from a file
    */
    virtual void readFilterParameters(AbstractFilterParametersReader* reader, int index);

    virtual void execute();
    virtual void preflight();


  signals:
    void updateFilterParameters(AbstractFilter* filter);
    void parametersChanged();
    void preflightAboutToExecute();
    void preflightExecuted();

  protected:
    FindBoundaryCellFractions();
    void find_surface_voxel_fractions();

  private:
    DEFINE_REQUIRED_DATAARRAY_VARIABLE(int32_t, FeatureIds)
    DEFINE_REQUIRED_DATAARRAY_VARIABLE(int8_t, BoundaryCells)
    DEFINE_CREATED_DATAARRAY_VARIABLE(float, BoundaryCellFractions)

    void dataCheck();

    FindBoundaryCellFractions(const FindBoundaryCellFractions&); // Copy Constructor Not Implemented
    void operator=(const FindBoundaryCellFractions&); // Operator '=' Not Implemented
};

#endif /* FindBoundaryCellFractions_H_ */



