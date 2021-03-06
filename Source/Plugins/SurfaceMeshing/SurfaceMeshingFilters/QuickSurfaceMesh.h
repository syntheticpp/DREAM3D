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
#ifndef _QuickSurfaceMesh_H_
#define _QuickSurfaceMesh_H_

#include <vector>
#include <QtCore/QString>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/DataArrays/IDataArray.h"
#include "SurfaceMeshing/SurfaceMeshingFilters/SurfaceMeshFilter.h"
#include "DREAM3DLib/DataContainers/VolumeDataContainer.h"
#include "DREAM3DLib/DataContainers/MeshStructs.h"


#include "SurfaceMeshing/SurfaceMeshingConstants.h"



/*
 *
 */
class QuickSurfaceMesh : public AbstractFilter
{
    Q_OBJECT /* Need this for Qt's signals and slots mechanism to work */
  public:
    DREAM3D_SHARED_POINTERS(QuickSurfaceMesh)
    DREAM3D_STATIC_NEW_MACRO(QuickSurfaceMesh)
    DREAM3D_TYPE_MACRO_SUPER(QuickSurfaceMesh, AbstractFilter)

    virtual ~QuickSurfaceMesh();
    DREAM3D_FILTER_PARAMETER(QString, SurfaceDataContainerName)
    Q_PROPERTY(QString SurfaceDataContainerName READ getSurfaceDataContainerName WRITE setSurfaceDataContainerName)
    DREAM3D_FILTER_PARAMETER(QString, VertexAttributeMatrixName)
    Q_PROPERTY(QString VertexAttributeMatrixName READ getVertexAttributeMatrixName WRITE setVertexAttributeMatrixName)
    DREAM3D_FILTER_PARAMETER(QString, FaceAttributeMatrixName)
    Q_PROPERTY(QString FaceAttributeMatrixName READ getFaceAttributeMatrixName WRITE setFaceAttributeMatrixName)

    DREAM3D_INSTANCE_PROPERTY(bool, TransferPhaseId)
    Q_PROPERTY(bool TransferPhaseId READ getTransferPhaseId WRITE setTransferPhaseId)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, FeatureIdsArrayPath)
    Q_PROPERTY(DataArrayPath FeatureIdsArrayPath READ getFeatureIdsArrayPath WRITE setFeatureIdsArrayPath)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, CellPhasesArrayPath)
    Q_PROPERTY(DataArrayPath CellPhasesArrayPath READ getCellPhasesArrayPath WRITE setCellPhasesArrayPath)

    DREAM3D_FILTER_PARAMETER(QString, FaceLabelsArrayName)
    Q_PROPERTY(QString FaceLabelsArrayName READ getFaceLabelsArrayName WRITE setFaceLabelsArrayName)

    DREAM3D_FILTER_PARAMETER(QString, NodeTypesArrayName)
    Q_PROPERTY(QString NodeTypesArrayName READ getNodeTypesArrayName WRITE setNodeTypesArrayName)

    DREAM3D_FILTER_PARAMETER(QString, FacePhasesArrayName)
    Q_PROPERTY(QString FacePhasesArrayName READ getFacePhasesArrayName WRITE setFacePhasesArrayName)

    virtual const QString getCompiledLibraryName();
    virtual AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters);
    virtual const QString getGroupName();
    virtual const QString getSubGroupName();
    virtual const QString getHumanLabel();

    virtual int writeFilterParameters(AbstractFilterParametersWriter* writer, int index);

    /**
    * @brief This method will read the options from a file
    * @param reader The reader that is used to read the options from a file
    */
    virtual void readFilterParameters(AbstractFilterParametersReader* reader, int index);

    virtual void setupFilterParameters();
    virtual void execute();
    virtual void preflight();

  signals:
    void updateFilterParameters(AbstractFilter* filter);
    void parametersChanged();
    void preflightAboutToExecute();
    void preflightExecuted();

  protected:
    QuickSurfaceMesh();

  private:
    DEFINE_REQUIRED_DATAARRAY_VARIABLE(int32_t, FeatureIds)
    DEFINE_REQUIRED_DATAARRAY_VARIABLE(int32_t, CellPhases)
    DEFINE_CREATED_DATAARRAY_VARIABLE(int32_t, FaceLabels)
    DEFINE_CREATED_DATAARRAY_VARIABLE(int8_t, NodeTypes)
    DEFINE_CREATED_DATAARRAY_VARIABLE(int32_t, FacePhases)

    void dataCheck();
    void updateFaceInstancePointers();
    void updateVertexInstancePointers();

    QuickSurfaceMesh(const QuickSurfaceMesh&); // Copy Constructor Not Implemented
    void operator=(const QuickSurfaceMesh&); // Operator '=' Not Implemented

};

#endif /* QuickSurfaceMesh_H_ */



