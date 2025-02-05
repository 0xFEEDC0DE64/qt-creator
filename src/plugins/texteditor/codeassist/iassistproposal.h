/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

#include "assistenums.h"
#include "iassistproposalmodel.h"

#include <texteditor/texteditor_global.h>

#include <utils/id.h>

namespace TextEditor {

class IAssistProposalWidget;
class TextEditorWidget;

class TEXTEDITOR_EXPORT IAssistProposal
{
public:
    IAssistProposal(Utils::Id id,int basePosition);
    virtual ~IAssistProposal();

    int basePosition() const;
    bool isFragile() const;
    bool supportsPrefix() const;
    virtual bool hasItemsToPropose(const QString &, AssistReason) const { return true; }
    virtual bool isCorrective(TextEditorWidget *editorWidget) const;
    virtual void makeCorrection(TextEditorWidget *editorWidget);
    virtual TextEditor::ProposalModelPtr model() const = 0;
    virtual IAssistProposalWidget *createWidget() const = 0;

    void setFragile(bool fragile);
    void setSupportsPrefix(bool supportsPrefix);

    Utils::Id id() const { return m_id; }

    AssistReason reason() const { return m_reason; }
    void setReason(const AssistReason &reason) { m_reason = reason; }

protected:
    Utils::Id m_id;
    int m_basePosition;
    bool m_isFragile = false;
    bool m_supportsPrefix = true;
    AssistReason m_reason;
};

} // TextEditor
