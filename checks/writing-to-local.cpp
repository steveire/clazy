/*
   This file is part of the clazy static checker.

  Copyright (C) 2016 Sergio Martins <smartins@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "writing-to-local.h"
#include "Utils.h"
#include "checkmanager.h"
#include "StringUtils.h"
#include "ContextUtils.h"
#include "MacroUtils.h"

#include <clang/AST/AST.h>
#include <clang/Lex/Lexer.h>

using namespace clang;
using namespace std;


WritingToLocal::WritingToLocal(const std::string &name, const clang::CompilerInstance &ci)
    : CheckBase(name, ci)
{
}

void WritingToLocal::VisitStmt(clang::Stmt *stmt)
{
    Expr *expr = Utils::isWriteOperator(stmt);
    if (!expr || !m_lastDecl)
        return;

    MemberExpr *memberExpr = dyn_cast<MemberExpr>(expr);
    DeclRefExpr *declRef = memberExpr ? dyn_cast<DeclRefExpr>(memberExpr->getBase()) : nullptr;
    if (!declRef || ContextUtils::isInLoop(m_parentMap, memberExpr) ||
            MacroUtils::isInAnyMacro(ci(), memberExpr->getLocStart(), { "Q_GLOBAL_STATIC", "Q_GLOBAL_STATIC_WITH_ARGS", "Q_DUMMY_ACCESSOR" }) ||
            !ContextUtils::isValueDeclInFunctionContext(declRef->getDecl()))
        return;

    CXXRecordDecl *record = Utils::recordFromVarDecl(declRef->getDecl());
    if (!record || !record->hasTrivialDestructor()) // RAII class
        return;

    DeclContext *lastDeclContext = isa<DeclContext>(m_lastDecl) ? cast<DeclContext>(m_lastDecl)
                                                                : m_lastDecl->getDeclContext();


    FunctionDecl *func = ContextUtils::firstContextOfType<FunctionDecl>(lastDeclContext);
    Stmt *body = func ? func->getBody() : nullptr;

    if (Utils::addressIsTaken(ci(), body, declRef->getDecl()) ||
        Utils::addressIsTaken(ci(), body, memberExpr->getMemberDecl()))
        return;

    std::vector<DeclRefExpr*> declRefs = HierarchyUtils::getStatements<DeclRefExpr>(m_ci, body, memberExpr->getBase()->getLocStart());


    bool isReferencedLater = clazy_std::contains_if(declRefs, [declRef](const DeclRefExpr *d) {
        return d->getDecl() == declRef->getDecl();
    });

    if (isReferencedLater)
        return;

    emitWarning(stmt->getLocStart(), "Call to local variable is a no-op:");
}


REGISTER_CHECK_WITH_FLAGS("writing-to-local", WritingToLocal, HiddenCheckLevel)
