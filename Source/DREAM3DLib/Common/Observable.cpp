/* ============================================================================
 * Copyright (c) 2011, Michael A. Jackson (BlueQuartz Software)
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
#include "Observable.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Observable::Observable() :
  QObject(NULL)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Observable::~Observable()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Observable::broadcastPipelineMessage(const PipelineMessage& msg)
{
  emit filterGeneratedMessage(msg);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Observable::notifyErrorMessage(const QString& humanLabel, const QString& str, int code)
{
  PipelineMessage pm = PipelineMessage::CreateErrorMessage(getNameOfClass(), humanLabel, str, code);
  emit filterGeneratedMessage(pm);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Observable::notifyStatusMessage(const QString& humanLabel, const QString& str)
{
  PipelineMessage pm = PipelineMessage::CreateStatusMessage(getNameOfClass(), humanLabel, str);
  emit filterGeneratedMessage(pm);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Observable::notifyStatusMessage(const QString& prefix, const QString& humanLabel, const QString& str)
{
  PipelineMessage pm = PipelineMessage::CreateStatusMessage(getNameOfClass(), humanLabel, str);
  pm.setPrefix(prefix);
  emit filterGeneratedMessage(pm);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Observable::notifyWarningMessage(const QString& humanLabel, const QString& str, int code)
{
  PipelineMessage pm = PipelineMessage::CreateWarningMessage(getNameOfClass(), humanLabel, str, code);
  emit filterGeneratedMessage(pm);
}
