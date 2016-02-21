/*
   This file is part of the clazy static checker.

  Copyright (C) 2016 Stephen Kelly <steveire@gmail.com>

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

#include "auto-auto-auto.h"
#include "FixItUtils.h"
#include "StringUtils.h"
#include "Utils.h"
#include "checkmanager.h"

#include <clang/AST/AST.h>
#include <clang/Lex/Lexer.h>

using namespace clang;
using namespace std;

enum Fixit {
  FixitNone = 0,
  FixitUseAuto = 0x1,
};

AutoAutoAuto::AutoAutoAuto(const std::string &name,
                           const clang::CompilerInstance &ci)
    : CheckBase(name, ci) {}

void AutoAutoAuto::VisitStmt(clang::Stmt *stmt) {
  const SourceLocation start = stmt->getLocStart();

  if (sm().isMacroBodyExpansion(start))
    return;

  if (sm().isMacroArgExpansion(start))
    return;

  DeclStmt *declStmt = dyn_cast<DeclStmt>(stmt);
  if (!declStmt)
    return;

  auto rng = declStmt->decls();

  auto firstDecl = dyn_cast<VarDecl>(*rng.begin());

  for (auto child : rng) {
    VarDecl *varDecl = dyn_cast<VarDecl>(child);

    if (!varDecl)
      return;

    if (!compatibleDecl(varDecl, firstDecl)) {
      return;
    }
  }

  std::vector<FixItHint> fixits;

  if (isFixitEnabled(FixitUseAuto)) {

    std::vector<SourceLocation> StarLocations;

    for (auto child : rng) {
      VarDecl *varDecl = dyn_cast<VarDecl>(child);
      auto Q =
          varDecl->getTypeSourceInfo()->getTypeLoc().getAs<PointerTypeLoc>();
      while (!Q.isNull()) {
        StarLocations.push_back(Q.getStarLoc());
        Q = Q.getNextTypeLoc().getAs<PointerTypeLoc>();
      }
    }

    std::string replacement = "auto";

    if (firstDecl->getType().isConstQualified()) {
      replacement = "const " + replacement;
    }

    replacement += " " + firstDecl->getName().str();

    if (firstDecl->isStaticLocal())
      replacement = "static " + replacement;

    SourceLocation start = firstDecl->getLocStart();
    SourceLocation end = firstDecl->getLocation();
    fixits.push_back(FixItUtils::createReplacement({start, end}, replacement));

    // Remove '*' from declarations using the saved star locations.
    for (const auto &Loc : StarLocations) {
      fixits.push_back(FixItHint::CreateReplacement(Loc, ""));
    }
  }

  emitWarning(stmt->getLocStart(), "auto can be used here.", fixits);
}

bool AutoAutoAuto::compatibleDecl(clang::VarDecl *varDecl,
                                  clang::VarDecl *firstDecl) {

  if (varDecl->getInitStyle() != VarDecl::CInit)
    return false;

  if (varDecl->isExceptionVariable())
    return false;

  ParmVarDecl *parmVarDecl = dyn_cast<ParmVarDecl>(varDecl);
  if (parmVarDecl)
    return false;

  const Type *type = varDecl->getType().getTypePtrOrNull();

  if (dyn_cast<AutoType>(type))
    return false;

  if (varDecl->getType()->isAnyPointerType() &&
      dyn_cast<AutoType>(varDecl->getType()->getPointeeType()))
    return false;

  if (dyn_cast<ConstantArrayType>(type))
    return false;

  const Expr *exprInit = varDecl->getInit();
  if (!exprInit) {
    return false;
  }

  if (const auto *E = dyn_cast<ExprWithCleanups>(exprInit))
    exprInit = E->getSubExpr();

  // Drill down to the as-written initializer.

  const Expr *E = exprInit->IgnoreImplicit();

  const auto *Construct = dyn_cast<CXXConstructExpr>(exprInit);
  if (Construct) {

    // Ensure that the constructor receives a single argument.
    if (Construct->getNumArgs() != 1)
      return false;

    // Drill down to the as-written initializer.
    E = (*Construct->arg_begin())->IgnoreParenImpCasts();
  }
  if (E != E->IgnoreConversionOperator()) {
    // We hit a conversion operator. Early-out now as they imply an implicit
    // conversion from a different type. Could also mean an explicit
    // conversion from the same type but that's pretty rare.
    return false;
  }

  if (auto T = dyn_cast<CXXBindTemporaryExpr>(E)) {
    E = T->getSubExpr();
  }

  if (dyn_cast<ImplicitCastExpr>(E)) {
    return false;
  }
  if (dyn_cast<InitListExpr>(E)) {
    return false;
  }

  if (const auto *NestedConstruct = dyn_cast<CXXConstructExpr>(E)) {
    // If we ran into an implicit conversion contructor, can't convert.
    //
    // FIXME: The following only checks if the constructor can be used
    // implicitly, not if it actually was. Cases where the converting
    // constructor was used explicitly won't get converted.
    if (NestedConstruct->getConstructor()->isConvertingConstructor(false))
      return false;
  }

  if (varDecl->getType()->isPointerType() &&
      varDecl->getType()->getPointeeType()->isFunctionType()) {
    return false;
  }

  if ((varDecl->getType()->getUnqualifiedDesugaredType() ==
       E->getType()->getUnqualifiedDesugaredType()) ||
      (varDecl->getType()->isPointerType() && E->getType()->isPointerType())) {

    if (varDecl->getType() == firstDecl->getType()) {
      return true;
    }
  }

  if (varDecl != firstDecl) {
    emitWarning(
        varDecl->getLocStart(),
        "single statement declares multiple variables of different types");
  }
  return false;
}

const char *const s_checkName = "auto-auto-auto";
REGISTER_CHECK_WITH_FLAGS(s_checkName, AutoAutoAuto, HiddenCheckLevel)
REGISTER_FIXIT(FixitUseAuto, "fix-auto-auto-auto", s_checkName)
