/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories
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
#include "StatsGenODFWidget.h"

#if QWT_VERSION >= 0x060000
#include "backwards.h"
#endif

//-- C++ Includes
#include <iostream>

//-- Qt Includes
#include <QtCore/QVector>
#include <QtCore/QtConcurrentMap>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QSettings>

#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QAbstractItemDelegate>
#include <QtGui/QProgressDialog>


#include "EbsdLib/TSL/AngConstants.h"
#include "EbsdLib/TSL/AngReader.h"
#include "EbsdLib/HKL/CtfConstants.h"
#include "EbsdLib/HKL/CtfReader.h"



#include "DREAM3DLib/Common/Texture.hpp"
#include "DREAM3DLib/Common/StatsGen.hpp"
#include "OrientationLib/IO/AngleFileLoader.h"

#include "StatsGenerator/TableModels/SGODFTableModel.h"
#include "StatsGenerator/StatsGenMDFWidget.h"
#include "StatsGenerator/TextureDialog.h"

#include "OrientationLib/Utilities/PoleFigureUtilities.h"
#include "QtSupport/PoleFigureImageUtilities.h"


#define SHOW_POLE_FIGURES 1
#define COLOR_POLE_FIGURES 1


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGenODFWidget::StatsGenODFWidget(QWidget* parent) :
  QWidget(parent),
  m_EnableAxisDecorations(false),
  m_Initializing(true),
  m_PhaseIndex(-1),
  m_CrystalStructure(Ebsd::CrystalStructure::Cubic_High),
  m_ODFTableModel(NULL),
  m_MDFWidget(NULL)
{
  m_OpenDialogLastDirectory = QDir::homePath();
  this->setupUi(this);
  this->setupGui();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGenODFWidget::~StatsGenODFWidget()
{
  if (NULL != m_ODFTableModel)
  {
    m_ODFTableModel->deleteLater();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::extractStatsData(int index, StatsData* statsData, unsigned int phaseType)
{
  VectorOfFloatArray arrays;
  if(phaseType == DREAM3D::PhaseType::PrimaryPhase)
  {
    PrimaryStatsData* pp = PrimaryStatsData::SafePointerDownCast(statsData);
    arrays = pp->getODF_Weights();
  }
  if(phaseType == DREAM3D::PhaseType::PrecipitatePhase)
  {
    PrecipitateStatsData* pp = PrecipitateStatsData::SafePointerDownCast(statsData);
    arrays = pp->getODF_Weights();
  }
  if(phaseType == DREAM3D::PhaseType::TransformationPhase)
  {
    TransformationStatsData* tp = TransformationStatsData::SafePointerDownCast(statsData);
    arrays = tp->getODF_Weights();
  }
  if (arrays.size() > 0)
  {
    QVector<float> e1(static_cast<int>(arrays[0]->getNumberOfTuples()));
    ::memcpy( e1.data(), arrays[0]->getVoidPointer(0), sizeof(float)*e1.size() );

    QVector<float> e2(static_cast<int>(arrays[1]->getNumberOfTuples()));
    ::memcpy( e2.data(), arrays[1]->getVoidPointer(0), sizeof(float)*e2.size() );

    QVector<float> e3(static_cast<int>(arrays[2]->getNumberOfTuples()));
    ::memcpy( e3.data(), arrays[2]->getVoidPointer(0), sizeof(float)*e3.size() );

    QVector<float> weights(static_cast<int>(arrays[3]->getNumberOfTuples()));
    ::memcpy( weights.data(), arrays[3]->getVoidPointer(0), sizeof(float)*weights.size() );

    QVector<float> sigmas(static_cast<int>(arrays[4]->getNumberOfTuples()));
    ::memcpy( sigmas.data(), arrays[4]->getVoidPointer(0), sizeof(float)*sigmas.size() );

    // Convert from Radians to Degrees for the Euler Angles
    for(int i = 0; i < e1.size(); ++i)
    {
      e1[i] = e1[i] * 180.0f / M_PI;
      e2[i] = e2[i] * 180.0f / M_PI;
      e3[i] = e3[i] * 180.0f / M_PI;
    }

    if(e1.size() > 0)
    {
      // Load the data into the table model
      m_ODFTableModel->setTableData(e1, e2, e3, weights, sigmas);
    }
  }
  // Write the MDF Data if we have that functionality enabled
  if (m_MDFWidget != NULL)
  {
    m_MDFWidget->extractStatsData(index, statsData, phaseType);
  }
  updatePlots();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int StatsGenODFWidget::getOrientationData(StatsData* statsData, unsigned int phaseType)
{
  int retErr = 0;

  QVector<float> e1s;
  QVector<float> e2s;
  QVector<float> e3s;
  QVector<float> weights;
  QVector<float> sigmas;
  QVector<float> odf;

  // Initialize xMax and yMax....
  e1s = m_ODFTableModel->getData(SGODFTableModel::Euler1);
  e2s = m_ODFTableModel->getData(SGODFTableModel::Euler2);
  e3s = m_ODFTableModel->getData(SGODFTableModel::Euler3);
  weights = m_ODFTableModel->getData(SGODFTableModel::Weight);
  sigmas = m_ODFTableModel->getData(SGODFTableModel::Sigma);

  for (QVector<float>::size_type i = 0; i < e1s.size(); i++)
  {
    e1s[i] = e1s[i] * M_PI / 180.0;
    e2s[i] = e2s[i] * M_PI / 180.0;
    e3s[i] = e3s[i] * M_PI / 180.0;
  }
  size_t numEntries = e1s.size();

  if ( Ebsd::CrystalStructure::Cubic_High == m_CrystalStructure)
  {
    odf.resize(CubicOps::k_OdfSize);
    Texture::CalculateCubicODFData(e1s.data(), e2s.data(), e3s.data(),
                                   weights.data(), sigmas.data(), true,
                                   odf.data(), numEntries);
  }
  else if ( Ebsd::CrystalStructure::Hexagonal_High == m_CrystalStructure)
  {
    odf.resize(HexagonalOps::k_OdfSize);
    Texture::CalculateHexODFData(e1s.data(), e2s.data(), e3s.data(),
                                 weights.data(), sigmas.data(), true,
                                 odf.data(), numEntries);
  }
  if (odf.size() > 0)
  {
    FloatArrayType::Pointer p = FloatArrayType::FromQVector(odf, DREAM3D::StringConstants::ODF);
    if(phaseType == DREAM3D::PhaseType::PrimaryPhase)
    {
      PrimaryStatsData* pp = PrimaryStatsData::SafePointerDownCast(statsData);
      pp->setODF(p);
    }
    if(phaseType == DREAM3D::PhaseType::PrecipitatePhase)
    {
      PrecipitateStatsData* pp = PrecipitateStatsData::SafePointerDownCast(statsData);
      pp->setODF(p);
    }
    if(phaseType == DREAM3D::PhaseType::TransformationPhase)
    {
      TransformationStatsData* tp = TransformationStatsData::SafePointerDownCast(statsData);
      tp->setODF(p);
    }
    if (e1s.size() > 0)
    {
      FloatArrayType::Pointer euler1 = FloatArrayType::FromQVector(e1s, DREAM3D::StringConstants::Euler1);
      FloatArrayType::Pointer euler2 = FloatArrayType::FromQVector(e2s, DREAM3D::StringConstants::Euler2);
      FloatArrayType::Pointer euler3 = FloatArrayType::FromQVector(e3s, DREAM3D::StringConstants::Euler3);
      FloatArrayType::Pointer sigma = FloatArrayType::FromQVector(sigmas, DREAM3D::StringConstants::Sigma);
      FloatArrayType::Pointer weight = FloatArrayType::FromQVector(weights, DREAM3D::StringConstants::Weight);

      VectorOfFloatArray odfWeights;
      odfWeights.push_back(euler1);
      odfWeights.push_back(euler2);
      odfWeights.push_back(euler3);
      odfWeights.push_back(sigma);
      odfWeights.push_back(weight);
      if(phaseType == DREAM3D::PhaseType::PrimaryPhase)
      {
        PrimaryStatsData* pp = PrimaryStatsData::SafePointerDownCast(statsData);
        pp->setODF_Weights(odfWeights);
      }
      if(phaseType == DREAM3D::PhaseType::PrecipitatePhase)
      {
        PrecipitateStatsData* pp = PrecipitateStatsData::SafePointerDownCast(statsData);
        pp->setODF_Weights(odfWeights);
      }
      if(phaseType == DREAM3D::PhaseType::TransformationPhase)
      {
        TransformationStatsData* tp = TransformationStatsData::SafePointerDownCast(statsData);
        tp->setODF_Weights(odfWeights);
      }
    }
  }
  // Write the MDF Data if we have that functionality enabled
  if (m_MDFWidget != NULL)
  {
    m_MDFWidget->getMisorientationData(statsData, phaseType);
  }
  return retErr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::enableMDFTab(bool b)
{
  if (NULL != m_MDFWidget)
  {
    m_MDFWidget->deleteLater();
  }
  m_MDFWidget = new StatsGenMDFWidget();
  m_MDFWidget->setODFTableModel(m_ODFTableModel);
  tabWidget->addTab(m_MDFWidget, QString("MDF"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::setCrystalStructure(unsigned int value)
{
  if (m_CrystalStructure != value)
  {
    this->m_CrystalStructure = value;
    if (m_MDFWidget != NULL)
    {
      m_MDFWidget->setCrystalStructure(m_CrystalStructure);
    }
    m_ODFTableModel->setCrystalStructure(m_CrystalStructure);
    QAbstractItemDelegate* idelegate = m_ODFTableModel->getItemDelegate();
    m_ODFTableView->setItemDelegate(idelegate);
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
unsigned int StatsGenODFWidget::getCrystalStructure()
{
  return m_CrystalStructure;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::setPhaseIndex(int value)
{
  this->m_PhaseIndex = value;
  if (m_MDFWidget != NULL)
  {
    m_MDFWidget->setPhaseIndex(m_PhaseIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int StatsGenODFWidget::getPhaseIndex()
{
  return m_PhaseIndex;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::setupGui()
{
  // Setup the TableView and Table Models
  QHeaderView* headerView = new QHeaderView(Qt::Horizontal, m_ODFTableView);
  headerView->setResizeMode(QHeaderView::Interactive);
  m_ODFTableView->setHorizontalHeader(headerView);
  headerView->show();

  m_OdfBulkTableModel = new SGODFTableModel;
  m_OdfBulkTableModel->setCrystalStructure(m_CrystalStructure);
  m_OdfBulkTableModel->setInitialValues();
  m_ODFTableView->setModel(m_OdfBulkTableModel);

  m_ODFTableModel = new SGODFTableModel;
  m_ODFTableModel->setCrystalStructure(m_CrystalStructure);
  m_ODFTableModel->setInitialValues();
  m_ODFTableView->setModel(m_ODFTableModel);
  QAbstractItemDelegate* idelegate = m_ODFTableModel->getItemDelegate();
  m_ODFTableView->setItemDelegate(idelegate);

  addODFTextureBtn->setToolTip("Add ODF Weight");
  deleteODFTextureBtn->setToolTip("Delete ODF Weight");

  m_PlotCurves.push_back(new QwtPlotCurve);
  m_PlotCurves.push_back(new QwtPlotCurve);
  m_PlotCurves.push_back(new QwtPlotCurve);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::initQwtPlot(QString xAxisName, QString yAxisName, QwtPlot* plot)
{
  plot->setAxisTitle(QwtPlot::xBottom, xAxisName);
  plot->setAxisTitle(QwtPlot::yLeft, yAxisName);
  plot->setCanvasBackground(QColor(Qt::white));
  //plot->setCanvasLineWidth(2);
  //plot->canvas()->setFrameShape(QFrame::NoFrame);

  // Lock the Axis Min/Max to -1 to 1 effectively cropping the plot. If there are
  // data points outside of that range they will never be shown.
  plot->setAxisScale(QwtPlot::xBottom, -1.0, 1.0);
  plot->setAxisScale(QwtPlot::yLeft, -1.0, 1.0);

  // These set the plot axis to NOT show anything except the axis labels.
  plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, m_EnableAxisDecorations);
  plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, m_EnableAxisDecorations);
  plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Labels, m_EnableAxisDecorations);
  plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, m_EnableAxisDecorations);
  plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, m_EnableAxisDecorations);
  plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Labels, m_EnableAxisDecorations);


  drawODFPlotGrid(plot);
}

// -----------------------------------------------------------------------------
//  We should just subclass QwtPlotGrid and do all the drawing there.
// -----------------------------------------------------------------------------
void StatsGenODFWidget::drawODFPlotGrid(QwtPlot* plot)
{
  // Draw the accepted Inverse Pole Grid Style

  // Draw the Horizontal and Vertical Lines that form the central cross
  m_PlotGrid = new QwtPlotMarker();
  m_PlotGrid->attach(plot);
  m_PlotGrid->setLineStyle(QwtPlotMarker::Cross);
  m_PlotGrid->setLinePen(QPen(Qt::darkMagenta, 1, Qt::SolidLine));
  m_PlotGrid->setXValue(0);
  m_PlotGrid->setYValue(0);

  // Draw the outer Circle
  QwtArray<double> circleX(900); // 900 because our plots are hard set to 450 pixels
  QwtArray<double> circleY(900);
  float inc = 2.0f / 449.0f;

  for(int i = 0; i < 450; ++i)
  {
    circleX[i] = 1.0 - (i * inc);
    circleX[450 + i] = -1.0 + (i * inc);

    circleY[i] = sqrt(1.0 - (circleX[i] * circleX[i]));
    circleY[450 + i] = -circleY[i];
  }
  m_CircleGrid = new QwtPlotCurve;
#if QWT_VERSION >= 0x060000
  m_CircleGrid->setSamples(circleX, circleY);
#else
  m_CircleGrid->setData(circleX, circleY);
#endif
  QColor c = QColor(Qt::darkMagenta);
  c.setAlpha(255);
  m_CircleGrid->setPen(QPen(c));
  m_CircleGrid->setStyle(QwtPlotCurve::Lines);
  m_CircleGrid->attach(plot);

  // Draw the Rotated Central cross
  {
    QwtArray<double> rotCrossX(2);
    QwtArray<double> rotCrossY(2);
    rotCrossX[0] = 0.7071067811f;
    rotCrossY[0] = sqrt(1.0 - (rotCrossX[0] * rotCrossX[0]));
    rotCrossX[1] = -0.7071067811f;
    rotCrossY[1] = -sqrt(1.0 - (rotCrossX[1] * rotCrossX[1]));
    m_RotCross0 = new QwtPlotCurve;
#if QWT_VERSION >= 0x060000
    m_RotCross0->setSamples(rotCrossX, rotCrossY);
#else
    m_RotCross0->setData(rotCrossX, rotCrossY);
#endif
    QColor c = QColor(Qt::darkMagenta);
    c.setAlpha(200);
    m_RotCross0->setPen(QPen(c));
    m_RotCross0->setStyle(QwtPlotCurve::Lines);
    m_RotCross0->attach(plot);
  }

  {
    QwtArray<double> rotCrossX(2);
    QwtArray<double> rotCrossY(2);
    rotCrossX[0] = 0.7071067811f;
    rotCrossY[0] = -sqrt(1.0 - (rotCrossX[0] * rotCrossX[0]));
    rotCrossX[1] = -0.7071067811f;
    rotCrossY[1] = sqrt(1.0 - (rotCrossX[1] * rotCrossX[1]));
    m_RotCross1 = new QwtPlotCurve;
#if QWT_VERSION >= 0x060000
    m_RotCross1->setSamples(rotCrossX, rotCrossY);
#else
    m_RotCross1->setData(rotCrossX, rotCrossY);
#endif
    QColor c = QColor(Qt::darkMagenta);
    c.setAlpha(200);
    m_RotCross1->setPen(QPen(c));
    m_RotCross1->setStyle(QwtPlotCurve::Lines);
    m_RotCross1->attach(plot);
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::updatePlots()
{
  on_m_CalculateODFBtn_clicked();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::on_m_CalculateODFBtn_clicked()
{
  int err = 0;

  QwtArray<float> e1s;
  QwtArray<float> e2s;
  QwtArray<float> e3s;
  QwtArray<float> weights;
  QwtArray<float> sigmas;
  QwtArray<float> odf;
  SGODFTableModel* tableModel = NULL;

  if(weightSpreadGroupBox->isChecked() )
  {
    tableModel = m_ODFTableModel;
  }
  else
  {
    tableModel = m_OdfBulkTableModel;
  }


  e1s = tableModel->getData(SGODFTableModel::Euler1);
  e2s = tableModel->getData(SGODFTableModel::Euler2);
  e3s = tableModel->getData(SGODFTableModel::Euler3);
  weights = tableModel->getData(SGODFTableModel::Weight);
  sigmas = tableModel->getData(SGODFTableModel::Sigma);


  // Convert from Degrees to Radians
  for(int i = 0; i < e1s.size(); i++)
  {
    e1s[i] = e1s[i] * M_PI / 180.0;
    e2s[i] = e2s[i] * M_PI / 180.0;
    e3s[i] = e3s[i] * M_PI / 180.0;
  }
  size_t numEntries = e1s.size();

  int imageSize = pfImageSize->value();
  int lamberSize = pfLambertSize->value();
  int numColors = 16;
  int npoints = pfSamplePoints->value();
  QVector<size_t> dims(1, 3);
  FloatArrayType::Pointer eulers = FloatArrayType::CreateArray(npoints, dims, "Eulers");
  PoleFigureConfiguration_t config;
  QVector<UInt8ArrayType::Pointer> figures;

  if ( Ebsd::CrystalStructure::Cubic_High == m_CrystalStructure)
  {
    // We now need to resize all the arrays here to make sure they are all allocated
    odf.resize(CubicOps::k_OdfSize);
    Texture::CalculateCubicODFData(e1s.data(), e2s.data(), e3s.data(),
                                   weights.data(), sigmas.data(), true,
                                   odf.data(), numEntries);

    err = StatsGen::GenCubicODFPlotData(odf.data(), eulers->getPointer(0), npoints);

    CubicOps ops;
    config.eulers = eulers.get();
    config.imageDim = imageSize;
    config.lambertDim = lamberSize;
    config.numColors = numColors;

    figures = ops.generatePoleFigure(config);
  }
  else if ( Ebsd::CrystalStructure::Hexagonal_High == m_CrystalStructure)
  {
    // We now need to resize all the arrays here to make sure they are all allocated
    odf.resize(HexagonalOps::k_OdfSize);
    Texture::CalculateHexODFData(e1s.data(), e2s.data(), e3s.data(),
                                 weights.data(), sigmas.data(), true,
                                 odf.data(), numEntries);

    err = StatsGen::GenHexODFPlotData(odf.data(), eulers->getPointer(0), npoints);

    HexagonalOps ops;
    config.eulers = eulers.get();
    config.imageDim = imageSize;
    config.lambertDim = lamberSize;
    config.numColors = numColors;

    figures = ops.generatePoleFigure(config);
  }
  else if ( Ebsd::CrystalStructure::OrthoRhombic == m_CrystalStructure)
  {
    //    // We now need to resize all the arrays here to make sure they are all allocated
    odf.resize(OrthoRhombicOps::k_OdfSize);
    Texture::CalculateOrthoRhombicODFData(e1s.data(), e2s.data(), e3s.data(),
                                          weights.data(), sigmas.data(), true,
                                          odf.data(), numEntries);

    err = StatsGen::GenOrthoRhombicODFPlotData(odf.data(), eulers->getPointer(0), npoints);

    OrthoRhombicOps ops;
    config.eulers = eulers.get();
    config.imageDim = imageSize;
    config.lambertDim = lamberSize;
    config.numColors = numColors;

    figures = ops.generatePoleFigure(config);
  }

  if (err == 1)
  {
    //TODO: Present Error Message
    return;
  }

  QImage image = PoleFigureImageUtilities::Create3ImagePoleFigure(figures[0].get(), figures[1].get(), figures[2].get(), config, imageLayout->currentIndex());
  m_PoleFigureLabel->setPixmap(QPixmap::fromImage(image));

  // Enable the MDF tab
  if (m_MDFWidget != NULL)
  {
    m_MDFWidget->setEnabled(true);
    m_MDFWidget->updateMDFPlot(odf);
  }
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::on_addODFTextureBtn_clicked()
{
  TextureDialog t(m_CrystalStructure, NULL);
  int r = t.exec();
  if (r == QDialog::Accepted)
  {
    if (!m_ODFTableModel->insertRow(m_ODFTableModel->rowCount())) { return; }
    // Gather values from the dialog and push them to the Table Model
    float e1 = 0.0;
    float e2 = 0.0;
    float e3 = 0.0;
    float weight = 1.0;
    float sigma = 1.0;

    t.getODFEntry(e1, e2, e3, weight, sigma);
    int row = m_ODFTableModel->rowCount() - 1;
    m_ODFTableModel->setRowData(row, e1, e2, e3, weight, sigma);

    m_ODFTableView->resizeColumnsToContents();
    m_ODFTableView->scrollToBottom();
    m_ODFTableView->setFocus();
    QModelIndex index = m_ODFTableModel->index(m_ODFTableModel->rowCount() - 1, 0);
    m_ODFTableView->setCurrentIndex(index);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::on_selectAnglesFile_clicked()
{
  QString proposedFile = m_OpenDialogLastDirectory;
  QString file = QFileDialog::getOpenFileName(this, tr("Select Angles File"), proposedFile, tr("Text Document (*.txt *.ang *.ctf)"));
  angleFilePath->setText(file);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::on_loadODFTextureBtn_clicked()
{

  QString file = angleFilePath->text();
  if(true == file.isEmpty())
  {
    return;
  }

  QFileInfo fi(angleFilePath->text());
  if(fi.exists() == false)
  {
    return;
  }

  size_t count = 0;
  QVector<float> e1s(count);
  QVector<float> e2s(count);
  QVector<float> e3s(count);
  QVector<float> weights(count);
  QVector<float> sigmas(count);

  QProgressDialog progress("Loading Data ....", "Cancel", 0, 3, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(2000);

  if (fi.suffix().compare(Ebsd::Ang::FileExt) == 0)
  {
    progress.setValue(1);
    progress.setLabelText("[1/3] Reading File ...");
    AngReader loader;
    loader.setFileName(angleFilePath->text());
    int err = loader.readFile();
    if(err < 0)
    {
      QMessageBox::critical(this, "Error loading the ANG file", loader.getErrorMessage(), QMessageBox::Ok);
      return;
    }
    float* phi1 = loader.getPhi1Pointer();
    float* phi = loader.getPhiPointer();
    float* phi2 = loader.getPhi2Pointer();

    int xDim = loader.getXDimension();
    int yDim = loader.getYDimension();
    count = xDim * yDim;
    e1s.resize(count);
    e2s.resize(count);
    e3s.resize(count);
    weights.resize(count);
    sigmas.resize(count);
    for(size_t i = 0; i < count; ++i)
    {
      e1s[i] = phi1[i];
      e2s[i] = phi[i];
      e3s[i] = phi2[i];
      weights[i] = 1.0;
      sigmas[i] = 0.0;
    }
  }
  else if (fi.suffix().compare(Ebsd::Ctf::FileExt) == 0)
  {
    progress.setValue(1);
    progress.setLabelText("[1/3] Reading File ...");
    CtfReader loader;
    loader.setFileName(angleFilePath->text());
    int err = loader.readFile();
    if(err < 0)
    {
      QMessageBox::critical(this, "Error loading the CTF file", loader.getErrorMessage(), QMessageBox::Ok);
      return;
    }
    float* phi1 = loader.getEuler1Pointer();
    float* phi = loader.getEuler2Pointer();
    float* phi2 = loader.getEuler3Pointer();

    int xDim = loader.getXDimension();
    int yDim = loader.getYDimension();
    count = xDim * yDim;
    e1s.resize(count);
    e2s.resize(count);
    e3s.resize(count);
    weights.resize(count);
    sigmas.resize(count);
    for(size_t i = 0; i < count; ++i)
    {
      e1s[i] = phi1[i];
      e2s[i] = phi[i];
      e3s[i] = phi2[i];
      weights[i] = 1.0;
      sigmas[i] = 0.0;
    }
  }
  else
  {
    progress.setValue(1);
    progress.setLabelText("[1/3] Reading File ...");
    AngleFileLoader::Pointer loader = AngleFileLoader::New();
    loader->setInputFile(angleFilePath->text());
    loader->setAngleRepresentation(angleRepresentation->currentIndex());
    loader->setFileAnglesInDegrees(anglesInDegrees->isChecked());
    loader->setOutputAnglesInDegrees(true);
    QString delim;
    int index = delimiter->currentIndex();
    switch(index)
    {
      case 0:
        delim = " ";
        break;
      case 1:
        delim = "\t";
        break;
      case 2:
        delim = ",";
        break;
      case 3:
        delim = ";";
        break;
      default:
        delim = " ";

    }

    loader->setDelimiter(delim);
    FloatArrayType::Pointer data = loader->loadData();
    if (loader->getErrorCode() < 0)
    {
      QMessageBox::critical(this, "Error Loading Angle data", loader->getErrorMessage(), QMessageBox::Ok);
      return;
    }
    count = data->getNumberOfTuples();
    e1s.resize(count);
    e2s.resize(count);
    e3s.resize(count);
    weights.resize(count);
    sigmas.resize(count);
    for(size_t i = 0; i < count; ++i)
    {
      e1s[i] = data->getComponent(i, 0);
      e2s[i] = data->getComponent(i, 1);
      e3s[i] = data->getComponent(i, 2);
      weights[i] = data->getComponent(i, 3);
      sigmas[i] = data->getComponent(i, 4);
    }

  }

  progress.setValue(2);
  progress.setLabelText("[2/3] Rendering Pole Figure ...");

  m_OdfBulkTableModel->removeRows(0, m_OdfBulkTableModel->rowCount());

#if 1
  m_OdfBulkTableModel->blockSignals(true);
  m_OdfBulkTableModel->setColumnData(SGODFTableModel::Euler1, e1s);
  m_OdfBulkTableModel->setColumnData(SGODFTableModel::Euler2, e2s);
  m_OdfBulkTableModel->setColumnData(SGODFTableModel::Euler3, e3s);
  m_OdfBulkTableModel->setColumnData(SGODFTableModel::Weight, weights);
  m_OdfBulkTableModel->setColumnData(SGODFTableModel::Sigma, sigmas);
  m_OdfBulkTableModel->blockSignals(false);
#endif
  on_m_CalculateODFBtn_clicked();
  progress.setValue(3);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::on_deleteODFTextureBtn_clicked()
{
  QItemSelectionModel* selectionModel = m_ODFTableView->selectionModel();
  if (!selectionModel->hasSelection()) { return; }
  QModelIndex index = selectionModel->currentIndex();
  if (!index.isValid()) { return; }
  m_ODFTableModel->removeRow(index.row(), index.parent());
  if (m_ODFTableModel->rowCount() > 0)
  {
    m_ODFTableView->resizeColumnsToContents();
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SGODFTableModel* StatsGenODFWidget::tableModel()
{
  return m_ODFTableModel;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGenMDFWidget* StatsGenODFWidget::getMDFWidget()
{
  return m_MDFWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::on_bulkLoadGroupBox_clicked ( bool checked )
{
  weightSpreadGroupBox->setChecked(!checked);
  m_ODFTableView->setModel(m_OdfBulkTableModel);
  QAbstractItemDelegate* idelegate = m_OdfBulkTableModel->getItemDelegate();
  m_ODFTableView->setItemDelegate(idelegate);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::on_weightSpreadGroupBox_clicked ( bool checked )
{
  bulkLoadGroupBox->setChecked(!checked);
  m_ODFTableView->setModel(m_ODFTableModel);
  QAbstractItemDelegate* idelegate = m_ODFTableModel->getItemDelegate();
  m_ODFTableView->setItemDelegate(idelegate);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::on_savePoleFigureImage_clicked()
{

  QString Ftype = "Image Files";
  QString ext = "*.png";
  QString s = "Image Files (*.tiff *.png *.bmp);;All Files(*.*)";
  QString defaultName = m_OpenDialogLastDirectory + QDir::separator() + "Untitled.png";
  QString file = QFileDialog::getSaveFileName(this, tr("Save File As"), defaultName, s);

  if(true == file.isEmpty())
  {
    return;
  }
  // bool ok = false;
  file = QDir::toNativeSeparators(file);
  // Store the last used directory into the private instance variable
  QFileInfo fi(file);
  m_OpenDialogLastDirectory = fi.path();

  QImage image = m_PoleFigureLabel->pixmap()->toImage();
  image.save(file);
}
