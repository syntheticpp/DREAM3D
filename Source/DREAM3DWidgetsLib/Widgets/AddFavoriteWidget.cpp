/* ============================================================================
 * Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
 * Copyright (c) 2012 Joseph B. Kleingers (Student Research Assistant)
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
 * Neither the name of Michael A. Groeber, Michael A. Jackson, Joseph B. Kleingers,
 * the US Air Force, BlueQuartz Software nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
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

#include "AddFavoriteWidget.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <QtGui/QMessageBox>

AddFavoriteWidget::AddFavoriteWidget(QString prompt, QString parentPath, QWidget* parent)
{
  setupUi(this);
  m_ParentPath = parentPath;

  revertToDefault();
  BtnClicked = false;
  label->setText(prompt);
}

// -----------------------------------------------------------------------------
//  Called when the OK button is clicked.
// -----------------------------------------------------------------------------
void AddFavoriteWidget::on_addFavoriteOKBtn_clicked()
{
  BtnClicked = true;
  this->close();
}

// -----------------------------------------------------------------------------
//  Called when the Overwrite button is clicked.
// -----------------------------------------------------------------------------
void AddFavoriteWidget::on_addFavoriteOverwriteBtn_clicked()
{
  QString favoriteTitle = favoriteName->text();
  QString favoritePath = m_ParentPath + QDir::separator() + favoriteTitle + ".ini";

  emit fireRemoveExistingFavorite(favoritePath);

  on_addFavoriteOKBtn_clicked();
}

// -----------------------------------------------------------------------------
//  Called when the Cancel button is clicked.
// -----------------------------------------------------------------------------
void AddFavoriteWidget::on_addFavoriteCancelBtn_clicked()
{
  BtnClicked = false;
  this->close();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool AddFavoriteWidget::getBtnClicked()
{
  return BtnClicked;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AddFavoriteWidget::getFavoriteName()
{
  return ( favoriteName->text() );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFavoriteWidget::on_favoriteName_textChanged(const QString& text)
{ 
  QString prefPath = m_ParentPath + QDir::separator() + text + ".ini"; // Generate the proper path to the favorite

  QFileInfo fileInfo(prefPath);
  bool pathExists = fileInfo.exists();

#if 0
  if ( favoriteTitle.contains(QRegExp("[^a-zA-Z_-\\d\\s]")) )
  {
    QString displayText = "The name of the Favorite that was chosen has illegal characters.\n\nNames can only have:\n\tLetters\n\tNumbers\n\tUnderscores\n\tDashes";
    displayText = displayText + "\n\nNo special characters allowed due to file system restrictions";
    // Display error message
    int reply = QMessageBox::critical(this, tr("Add New Favorite"), tr(displayText.toLatin1().data()),
                                      QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
    if(reply == QMessageBox::Cancel)
    {
      done = true;
      favoriteTitle = "";
    }
  }
  else if (prefFileInfo.exists() == true)
  {
    QString displayText = "A favorite with that name already exists.  Please choose a different favorite name.";
    // Display error message
    int reply = QMessageBox::critical(this, tr("Add New Favorite"), tr(displayText.toLatin1().data()),
                                      QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
    if(reply == QMessageBox::Cancel)
    {
      done = true;
      favoriteTitle = "";
    }
  }
  else
  {
    done = true;
  }
#endif

  if (text.isEmpty())
  {
    addFavoriteOKBtn->setEnabled(false);

    if (addFavoriteOverwriteBtn->isDefault())
    {
      toggleOKBtn();
    }
  }
  else if (pathExists == true)
  {
    errorMessageDisplay->setText("A favorite with that name already exists.  Overwrite?");
    errorMessageDisplay->setHidden(false);
    toggleOKBtn();
  }
  else if ( text.contains(QRegExp("[^a-zA-Z_-\\d\\s]")) )
  {
    addFavoriteOKBtn->setEnabled(false);

    QString linkText = "<a href=#openWindow>Learn More.</a>";
    errorMessageDisplay->setText("The name that you chose has illegal characters. " + linkText);
    errorMessageDisplay->setTextFormat(Qt::RichText);
    errorMessageDisplay->setTextInteractionFlags(Qt::TextBrowserInteraction);
    errorMessageDisplay->setHidden(false);

    if (addFavoriteOverwriteBtn->isDefault())
    {
      toggleOKBtn();
    }
  }
  else
  {
    revertToDefault();

    if (addFavoriteOverwriteBtn->isDefault())
    {
      toggleOKBtn();
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFavoriteWidget::revertToDefault()
{
  addFavoriteOKBtn->setEnabled(true);
  errorMessageDisplay->setHidden(true);
  addFavoriteOverwriteBtn->setHidden(true);
  errorMessageDisplay->setTextFormat(Qt::PlainText);
  errorMessageDisplay->setTextInteractionFlags(Qt::NoTextInteraction);
  errorMessageDisplay->changeStyleSheet(FS_DOESNOTEXIST_STYLE);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFavoriteWidget::toggleOKBtn()
{
  if (addFavoriteOKBtn->isDefault())
  {
    addFavoriteOverwriteBtn->setHidden(false);
    addFavoriteOverwriteBtn->setDefault(true);
    addFavoriteOKBtn->setHidden(true);
    addFavoriteOKBtn->setDefault(false);
  }
  else
  {
    addFavoriteOKBtn->setHidden(false);
    addFavoriteOKBtn->setDefault(true);
    addFavoriteOverwriteBtn->setHidden(true);
    addFavoriteOverwriteBtn->setDefault(false);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFavoriteWidget::on_errorMessageDisplay_linkActivated(const QString &link)
{
  QMessageBox msgBox;
  msgBox.setWindowTitle("Naming Restrictions");
  msgBox.setText("Names can only contain letters, numbers, underscores, and dashes.");
  msgBox.setInformativeText("No special characters allowed due to file system restrictions.");
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  int ret = msgBox.exec();
}







