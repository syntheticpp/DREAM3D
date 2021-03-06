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
#ifndef _CRYSTALSTRUCTURE_H_
#define _CRYSTALSTRUCTURE_H_


#error
#include "EbsdLib/EbsdConstants.h"


#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/Constants.h"


/**
 * @class CrystalStructure CrystalStructure.h DREAM3DLib/Common/CrystalStructure.h
 * @brief  This class has some utility methods relating to CrystalStructure and
 * how they are used in the DREAM3D package.
 * @author Michael A. Jackson for BlueQuartz Software
 * @date Apr 21, 2011
 * @version 1.0
 */
class  CrystalStructure
{
  public:
    virtual ~CrystalStructure()
    {
    }

    /**
     * @brief Converts an enumeration value for CrystalStructure into a String
     * @param xtal
     * @return
     */
    static QString getCrystalStructureString(unsigned int xtal)
    {
      switch(xtal)
      {
        case Ebsd::CrystalStructure::Hexagonal:
          return QString("Hexagonal");
        case Ebsd::CrystalStructure::Cubic:
          return QString("Cubic");
        case Ebsd::CrystalStructure::AxisOrthoRhombic:
          return QString("OrthoRhombic");
        default:
          break;
      }
      return QString("Unknown");
    }



  protected:
    CrystalStructure()
    {
    }

  private:
    CrystalStructure(const CrystalStructure&); // Copy Constructor Not Implemented
    void operator=(const CrystalStructure&); // Operator '=' Not Implemented

};

#endif /* CRYSTALSTRUCTURE_H_ */

