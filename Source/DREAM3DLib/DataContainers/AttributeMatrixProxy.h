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
 *                           FA8650-10-D-5210
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#ifndef _AttributeMatrixProxy_H_
#define _AttributeMatrixProxy_H_

#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtCore/QMap>


#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/DataContainers/DataArrayProxy.h"


class AttributeMatrixProxy
{
  public:
    /**
     * @brief AttributeMatrixProxy
     */
    AttributeMatrixProxy() :
      flag(0),
      name(""),
      amType(DREAM3D::AttributeMatrixType::Unknown)
    {}

    /**
     * @brief AttributeMatrixProxy
     * @param am_name
     * @param read_am
     * @param am_type
     */
    AttributeMatrixProxy(QString am_name, uint8_t read_am = Qt::Checked, unsigned int am_type = DREAM3D::AttributeMatrixType::Unknown) :
      flag(read_am),
      name(am_name),
      amType(am_type)
    {}

    /**
    * @brief Copy Constructor
    */
    AttributeMatrixProxy(const AttributeMatrixProxy& amp)
    {
      flag = amp.flag;
      name = amp.name;
      amType = amp.amType;
      dataArrays = amp.dataArrays;
    }

    /**
    * @brief operator = method
    */
    void operator=(const AttributeMatrixProxy& amp)
    {
      flag = amp.flag;
      name = amp.name;
      amType = amp.amType;
      dataArrays = amp.dataArrays;
    }

    //----- Our variables, publicly available
    uint8_t flag;
    QString name;
    unsigned int amType;
    QMap<QString, DataArrayProxy> dataArrays;
  private:

};
Q_DECLARE_METATYPE(AttributeMatrixProxy)

#endif /* _AttributeMatrixProxy_H_ */

