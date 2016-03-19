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

#ifndef CLANG_LAZY_AUTO_AUTO_AUTO_H
#define CLANG_LAZY_AUTO_AUTO_AUTO_H

#include "checkbase.h"

namespace clang {
class Stmt;
}

/**
 * Uses auto almost always
 */
class AutoAutoAuto : public CheckBase {
public:
  explicit AutoAutoAuto(const std::string &name,
                        const clang::CompilerInstance &ci);
  void VisitStmt(clang::Stmt *stmt) override;

private:
  bool compatibleDecl(clang::VarDecl *varDecl, clang::VarDecl *firstDecl);

private:
  std::vector<clang::VarDecl *> ignoredDecls;
};

#endif
