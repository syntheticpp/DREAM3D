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
#include "QFilterParametersWriter.h"

#include <QtCore/QFileInfo>


#include "DREAM3DLib/DREAM3DVersion.h"

#include <QtCore/QStringListIterator>
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QFilterParametersWriter::QFilterParametersWriter() :
  AbstractFilterParametersWriter(),
  m_Prefs(NULL)
{
  //  qRegisterMetaType<IntVec3_t>("IntVec3_t");
  //  qRegisterMetaTypeStreamOperators<IntVec3_t>("IntVec3_t");

  //  qRegisterMetaType<FloatVec3_t>("FloatVec3_t");
  //  qRegisterMetaTypeStreamOperators<FloatVec3_t>("FloatVec3_t");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QFilterParametersWriter::~QFilterParametersWriter()
{
  if(NULL != m_Prefs)
  {
    closeFile();
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::WritePipelineToFile(FilterPipeline::Pointer pipeline, const QString& filePath, const QString& name, QSettings::Format format, IObserver* obs)
{

  if(NULL == pipeline.get())
  {
    if(NULL != obs)
    {
      PipelineMessage pm(QFilterParametersWriter::ClassName(), "FilterPipeline Object was NULL for writing", -1, PipelineMessage::Error);
      obs->processPipelineMessage(pm);
    }
    return -1;
  }

  FilterPipeline::FilterContainerType& filters = pipeline->getFilterContainer();

  QFilterParametersWriter::Pointer writer = QFilterParametersWriter::New();
  // This will open the file, and write/update the initial group of settings like the name and DREAM3D Version
  writer->openFile(filePath, format);
  // Loop over each filter and write it's input parameters to the file
  int count = filters.size();
  int index = 0;
  for(qint32 i = 0; i < count; ++i)
  {
    AbstractFilter::Pointer filter = filters.at(i);
    if(NULL != filter.get())
    {
      index = filter->writeFilterParameters(writer.get(), index);
    }
    else
    {
      AbstractFilter::Pointer badFilter = AbstractFilter::New();
      writer->openFilterGroup(badFilter.get(), i);
      writer->writeValue("Unkown Filter", "ERROR: Filter instance was NULL within the PipelineFilterWidget instance. Report this error to the DREAM3D Developers");
      writer->closeFilterGroup();
    }
  }

  writer->setNumberOfFilters(index);
  writer->setPipelineName(name);
  writer->closeFile(); // Close the file
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSettings* QFilterParametersWriter::getPrefs()
{
  return m_Prefs;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QFilterParametersWriter::openFile(QString filename, QSettings::Format format)
{
  if(NULL != m_Prefs)
  {
    closeFile();
    delete m_Prefs;
    m_Prefs = NULL;
  }
  m_Prefs = new QSettings(filename, format);
  m_Prefs->beginGroup(DREAM3D::Settings::PipelineBuilderGroup);
  m_Prefs->setValue("Name", "Unnamed Pipeline");
  m_Prefs->setValue(DREAM3D::Settings::NumFilters, 0);
  m_Prefs->setValue(DREAM3D::Settings::Version, DREAM3DLib::Version::Package() );
  m_Prefs->endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QFilterParametersWriter::closeFile()
{
  delete m_Prefs;
  m_Prefs = NULL;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QFilterParametersWriter::setNumberOfFilters(int numFilters)
{
  m_Prefs->beginGroup(DREAM3D::Settings::PipelineBuilderGroup);
  m_Prefs->setValue(DREAM3D::Settings::NumFilters, numFilters);
  m_Prefs->endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QFilterParametersWriter::setPipelineName(const QString& pipelineName)
{
  m_Prefs->beginGroup(DREAM3D::Settings::PipelineBuilderGroup);
  m_Prefs->setValue("Name", pipelineName);
  m_Prefs->endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::openFilterGroup(AbstractFilter* unused, int index)
{
  BOOST_ASSERT(m_Prefs != NULL);
  int err = 0;
  QString gName = QString::number(index);
  m_Prefs->beginGroup(gName);
  m_Prefs->setValue(DREAM3D::Settings::FilterName, unused->getNameOfClass());
  m_Prefs->setValue(DREAM3D::Settings::HumanLabel, unused->getHumanLabel());
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::closeFilterGroup()
{
  BOOST_ASSERT(m_Prefs != NULL);
  m_Prefs->endGroup();
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, const QString value)
{
  int err = 0;
  m_Prefs->setValue(name, value );
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, const QVector<QString> value)
{
  int err = 0;
  QString buf;
  QTextStream out(&buf);
  int size = value.size();
  for(qint32 i = 0; i < size; ++i)
  {
    out << value.at(i);
    if (i < size - 1)
    {
      out << " | ";
    }
  }
  m_Prefs->setValue(name, buf);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, int8_t value)
{
  int err = 0;
  m_Prefs->setValue(name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, int16_t value)
{
  int err = 0;
  m_Prefs->setValue(name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, int32_t value)
{
  int err = 0;
  m_Prefs->setValue(name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, int64_t value)
{
  int err = 0;
  qint64 _val = static_cast<qint64>(value);
  m_Prefs->setValue(name, _val);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, uint8_t value)
{
  int err = 0;
  m_Prefs->setValue(name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, uint16_t value)
{
  int err = 0;
  m_Prefs->setValue(name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, uint32_t value)
{
  int err = 0;
  m_Prefs->setValue(name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, uint64_t value)
{
  int err = 0;
  quint64 _val = static_cast<quint64>(value);
  m_Prefs->setValue(name, _val);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, float value)
{
  int err = 0;
  QByteArray buf;
  buf = buf.setNum(value, 'g', 8);
  m_Prefs->setValue(name, QString(buf));
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, double value)
{
  int err = 0;
  QByteArray buf;
  buf = buf.setNum(value, 'g', 8);
  m_Prefs->setValue(name, QString(buf));
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template<typename T>
void writeArray(QSettings* prefs, const QString name, QVector<T>& value)
{
  QString buf;
  QTextStream out(&buf);
  int size = value.size();
  for(qint32 i = 0; i < size; ++i)
  {
    out << value.at(i);
    if (i < size - 1)
    {
      out << ' ';
    }
  }
  prefs->setValue(name, buf);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, QVector<int8_t> value)
{
  writeArray(m_Prefs, name, value);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, QVector<int16_t> value)
{
  writeArray(m_Prefs, name, value);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, QVector<int32_t> value)
{
  writeArray(m_Prefs, name, value);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, QVector<int64_t> value)
{
  writeArray(m_Prefs, name, value);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, QVector<uint8_t> value)
{
  writeArray(m_Prefs, name, value);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, QVector<uint16_t> value)
{
  writeArray(m_Prefs, name, value);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, QVector<uint32_t> value)
{
  writeArray(m_Prefs, name, value);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, QVector<uint64_t> value)
{
  writeArray(m_Prefs, name, value);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, QVector<float> value)
{
  writeArray(m_Prefs, name, value);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, QVector<double> value)
{
  writeArray(m_Prefs, name, value);
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, IntVec3_t v)
{
  BOOST_ASSERT(m_Prefs != NULL);
  int err = 0;
  m_Prefs->beginWriteArray(name, 3);
  m_Prefs->setArrayIndex(0);
  m_Prefs->setValue("x", v.x);

  m_Prefs->setArrayIndex(1);
  m_Prefs->setValue("y", v.y);

  m_Prefs->setArrayIndex(2);
  m_Prefs->setValue("z", v.z);

  m_Prefs->endArray();
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, FloatVec3_t v)
{
  BOOST_ASSERT(m_Prefs != NULL);
  int err = 0;
  m_Prefs->beginWriteArray(name, 3);
  m_Prefs->setArrayIndex(0);
  m_Prefs->setValue("x", static_cast<double>(v.x));

  m_Prefs->setArrayIndex(1);
  m_Prefs->setValue("y", static_cast<double>(v.y));

  m_Prefs->setArrayIndex(2);
  m_Prefs->setValue("z", static_cast<double>(v.z));

  m_Prefs->endArray();
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, FloatVec4_t v)
{
  BOOST_ASSERT(m_Prefs != NULL);
  int err = 0;
  m_Prefs->beginWriteArray(name, 3);
  m_Prefs->setArrayIndex(0);
  m_Prefs->setValue("a", static_cast<double>(v.a));

  m_Prefs->setArrayIndex(1);
  m_Prefs->setValue("b", static_cast<double>(v.b));

  m_Prefs->setArrayIndex(2);
  m_Prefs->setValue("c", static_cast<double>(v.c));

  m_Prefs->setArrayIndex(3);
  m_Prefs->setValue("d", static_cast<double>(v.d));

  m_Prefs->endArray();
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, FloatVec21_t v)
{
  BOOST_ASSERT(m_Prefs != NULL);
  int err = 0;
  m_Prefs->beginWriteArray(name, 3);

  m_Prefs->setArrayIndex(0);
  m_Prefs->setValue("v11", static_cast<double>(v.v11));
  m_Prefs->setArrayIndex(1);
  m_Prefs->setValue("v12", static_cast<double>(v.v12));
  m_Prefs->setArrayIndex(2);
  m_Prefs->setValue("v13", static_cast<double>(v.v13));
  m_Prefs->setArrayIndex(3);
  m_Prefs->setValue("v14", static_cast<double>(v.v14));
  m_Prefs->setArrayIndex(4);
  m_Prefs->setValue("v15", static_cast<double>(v.v15));
  m_Prefs->setArrayIndex(5);
  m_Prefs->setValue("v16", static_cast<double>(v.v16));

  m_Prefs->setArrayIndex(6);
  m_Prefs->setValue("v22", static_cast<double>(v.v22));
  m_Prefs->setArrayIndex(7);
  m_Prefs->setValue("v23", static_cast<double>(v.v23));
  m_Prefs->setArrayIndex(8);
  m_Prefs->setValue("v24", static_cast<double>(v.v24));
  m_Prefs->setArrayIndex(9);
  m_Prefs->setValue("v25", static_cast<double>(v.v25));
  m_Prefs->setArrayIndex(10);
  m_Prefs->setValue("v26", static_cast<double>(v.v26));

  m_Prefs->setArrayIndex(11);
  m_Prefs->setValue("v33", static_cast<double>(v.v33));
  m_Prefs->setArrayIndex(12);
  m_Prefs->setValue("v34", static_cast<double>(v.v34));
  m_Prefs->setArrayIndex(13);
  m_Prefs->setValue("v35", static_cast<double>(v.v35));
  m_Prefs->setArrayIndex(14);
  m_Prefs->setValue("v36", static_cast<double>(v.v36));

  m_Prefs->setArrayIndex(15);
  m_Prefs->setValue("v44", static_cast<double>(v.v44));
  m_Prefs->setArrayIndex(16);
  m_Prefs->setValue("v45", static_cast<double>(v.v45));
  m_Prefs->setArrayIndex(17);
  m_Prefs->setValue("v46", static_cast<double>(v.v46));

  m_Prefs->setArrayIndex(18);
  m_Prefs->setValue("v55", static_cast<double>(v.v55));
  m_Prefs->setArrayIndex(19);
  m_Prefs->setValue("v56", static_cast<double>(v.v56));

  m_Prefs->setArrayIndex(20);
  m_Prefs->setValue("v66", static_cast<double>(v.v66));

  m_Prefs->endArray();
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, FileListInfo_t v)
{
  BOOST_ASSERT(m_Prefs != NULL);
  int err = 0;
  m_Prefs->beginWriteArray(name, 8);
  m_Prefs->setArrayIndex(0);
  m_Prefs->setValue("EndIndex", static_cast<qint32>(v.EndIndex));

  m_Prefs->setArrayIndex(1);
  m_Prefs->setValue("FileExtension", v.FileExtension);

  m_Prefs->setArrayIndex(2);
  m_Prefs->setValue("FilePrefix", v.FilePrefix);

  m_Prefs->setArrayIndex(3);
  m_Prefs->setValue("FileSuffix", v.FileSuffix);

  m_Prefs->setArrayIndex(4);
  m_Prefs->setValue("InputPath", v.InputPath);

  m_Prefs->setArrayIndex(5);
  m_Prefs->setValue("Ordering", static_cast<quint32>(v.Ordering));

  m_Prefs->setArrayIndex(6);
  m_Prefs->setValue("PaddingDigits", static_cast<qint32>(v.PaddingDigits));

  m_Prefs->setArrayIndex(7);
  m_Prefs->setValue("StartIndex", static_cast<qint32>(v.StartIndex));

  m_Prefs->endArray();
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, ComparisonInput_t comp)
{
  ComparisonInputs v;
  v.addInput(comp);
  return writeValue(name, v);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, ComparisonInputs v)
{
  int count = static_cast<int>( v.size() );
  int err = 0;
  m_Prefs->beginWriteArray(name, count);
  for(int i = 0; i < count; ++i)
  {
    ComparisonInput_t d = v[i];
    m_Prefs->setArrayIndex(i);
    m_Prefs->setValue("DataContainerName", d.dataContainerName);
    m_Prefs->setValue("AttributeMatrixName", d.attributeMatrixName);
    m_Prefs->setValue("AttributeArrayName", d.attributeArrayName);
    m_Prefs->setValue("CompOperator", d.compOperator);
    m_Prefs->setValue("CompValue", d.compValue);
  }
  m_Prefs->endArray();
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, AxisAngleInput_t data)
{
  int err = 0;
  m_Prefs->beginWriteArray(name, 0);
  m_Prefs->setArrayIndex(0);
  m_Prefs->setValue("Angle", static_cast<double>(data.angle));
  m_Prefs->setValue("H", static_cast<double>(data.h));
  m_Prefs->setValue("K", static_cast<double>(data.k));
  m_Prefs->setValue("L", static_cast<double>(data.l));
  m_Prefs->endArray();
  return err;
}


// -----------------------------------------------------------------------------
// These are actually written as a binary 4x1 float array as an attribute
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, AxisAngleInput_t data, int vectorPos)
{
  int err = 0;
  m_Prefs->setArrayIndex(vectorPos);
  m_Prefs->setValue("Angle", static_cast<double>(data.angle));
  m_Prefs->setValue("H", static_cast<double>(data.h));
  m_Prefs->setValue("K", static_cast<double>(data.k));
  m_Prefs->setValue("L", static_cast<double>(data.l));
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, QVector<AxisAngleInput_t> v)
{
  int count = static_cast<int>( v.size() );
  int err = 0;
  m_Prefs->beginWriteArray(name, count);
  for(int i = 0; i < count; ++i)
  {
    AxisAngleInput_t data = v.at(i);
    err = writeValue(name, data, i);
  }
  m_Prefs->endArray();
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeArraySelections(const QString name, QSet<QString> v)
{
  int err = 0;
  int count = v.size();
  m_Prefs->beginWriteArray("ArraySelections_" + name, count);
  count = 0;
  QSetIterator<QString> i(v);
  while (i.hasNext())
  {
    QString data = i.next();
    m_Prefs->setArrayIndex(count);
    m_Prefs->setValue(name, data);
    count++;
  }

  m_Prefs->endArray();
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, DataContainerArrayProxy& v)
{
  int err = 0;
  QListIterator<DataContainerProxy> dcIter(v.list);
  // Reset the iterator to the start of the QList
  dcIter.toFront();
  QStringList flat;
  QStringList daFlags;
  QStringList dcFlags;
  QStringList attrFlags;

  while (dcIter.hasNext()) // DataContainerLevel
  {
    const DataContainerProxy& dcProxy =  dcIter.next();
    // if(dcProxy.flag == Qt::Unchecked) { continue; } // Skip to the next DataContainer if we are not reading this one.

    QMapIterator<QString, AttributeMatrixProxy> amIter(dcProxy.attributeMatricies);
    while(amIter.hasNext()) // AttributeMatrixLevel
    {
      amIter.next();

      const AttributeMatrixProxy& amProxy = amIter.value();
      // if(amProxy.flag == Qt::Unchecked) { continue; } // Skip to the next AttributeMatrix if not reading this one

      QMapIterator<QString, DataArrayProxy> dIter(amProxy.dataArrays);
      while(dIter.hasNext()) // DataArray Level
      {
        dIter.next();

        const DataArrayProxy& daProxy = dIter.value();
        //if(daProxy.flag == DREAM3D::Unchecked) { continue; } // Skip to the next DataArray if not reading this one

        QString path = QString("%1|%2|%3").arg(dcProxy.name).arg(amProxy.name).arg(daProxy.name);
        flat << path;
        dcFlags << QString("%1").arg(dcProxy.flag);
        attrFlags << QString("%1").arg(amProxy.flag);
        daFlags << QString("%1").arg(daProxy.flag);

      }
    }

  }

  qint32 count = flat.size();
  m_Prefs->beginWriteArray(name, count);
  QStringListIterator i(flat);
  for(int i = 0; i < count; i++)
  {
    QString data = flat.at(i);
    m_Prefs->setArrayIndex(i);
    m_Prefs->setValue("Path", data);

    data = dcFlags.at(i);
    m_Prefs->setValue("DCFlag", data);

    data = attrFlags.at(i);
    m_Prefs->setValue("ATTRFlag", data);

    data = daFlags.at(i);
    m_Prefs->setValue("DAFlag", data);
  }
  m_Prefs->endArray();
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QFilterParametersWriter::writeValue(const QString name, const DataArrayPath& v)
{
  int err = 0;
  QString value = v.serialize();
  m_Prefs->setValue(name, value );
  return err;
}
