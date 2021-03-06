/*
   This file is part of the clazy static checker.

  Copyright (C) 2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Sérgio Martins <sergio.martins@kdab.com>

  Copyright (C) 2015 Sergio Martins <smartins@kde.org>

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

#ifndef MOREWARNINGS_UTILS_H
#define MOREWARNINGS_UTILS_H

#include <clang/AST/DeclCXX.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/DeclTemplate.h>

#include <string>
#include <vector>
#include <map>

// TODO: this is a dumping ground, most of these functions should be moved to the other *Utils classes

namespace clang {
    class CXXNamedCastExpr;
    class CXXRecordDecl;
    class CXXMemberCallExpr;
    class CXXConstructExpr;
    class CompilerInstance;
    class ClassTemplateSpecializationDecl;
    class Decl;
    class ParentMap;
    class SourceManager;
    class Stmt;
    class SourceLocation;
    class ExprWithCleanups;
    class ValueDecl;
    class ConditionalOperator;
    class CXXMethodDecl;
    class BinaryOperator;
}

namespace Utils {
    /// Returns true if childDecl is a descent from parentDecl
    bool derivesFrom(clang::CXXRecordDecl *derived, clang::CXXRecordDecl *possibleBase);

    // Returns true if the class derived is or descends from a class named parent
    bool derivesFrom(clang::CXXRecordDecl *derived, const std::string &possibleBase);

    /// Returns true if the class has at least one constexpr ctor
    bool hasConstexprCtor(clang::CXXRecordDecl *decl);

    /// Returns the type we're casting *from*
    clang::CXXRecordDecl * namedCastInnerDecl(clang::CXXNamedCastExpr *staticOrDynamicCast);

    /// Returns the type we're casting *to*
    clang::CXXRecordDecl * namedCastOuterDecl(clang::CXXNamedCastExpr *staticOrDynamicCast);

    /// Returns the class declaration from a variable declaration
    // So, if the var decl is "Foo f"; it returns the declaration of Foo
    clang::CXXRecordDecl * recordFromVarDecl(clang::Decl *);

    /// Returns the template specialization from a variable declaration
    // So, if the var decl is "QList<Foo> f;", returns the template specialization QList<Foo>
    clang::ClassTemplateSpecializationDecl * templateSpecializationFromVarDecl(clang::Decl *);

    /// Returns true if all the child member function calls are const functions.
    bool allChildrenMemberCallsConst(clang::Stmt *stm);

    /// Returns true if at least a UnaryOperator, BinaryOperator or non-const function call is found
    bool childsHaveSideEffects(clang::Stmt *stm);

    /// Receives a member call, such as "list.reserve()" and returns the declaration of the variable list
    // such as "QList<F> list"
    clang::ValueDecl * valueDeclForMemberCall(clang::CXXMemberCallExpr *);

    /// Receives an operator call, such as "list << fooo" and returns the declaration of the variable list
    // such as "QList<F> list"
    clang::ValueDecl * valueDeclForOperatorCall(clang::CXXOperatorCallExpr *);

    // overload
    clang::ValueDecl * valueDeclForCallExpr(clang::CallExpr *);

    // Returns true of this value decl is a member variable of a class or struct
    // returns null if not
    clang::CXXRecordDecl* isMemberVariable(clang::ValueDecl *);

    // Returns true if a body of statements contains a non const member call on object declared by varDecl
    // For example:
    // Foo foo; // this is the varDecl
    // while (bar) { foo.setValue(); // non-const call }
    bool containsNonConstMemberCall(clang::Stmt *body, const clang::VarDecl *varDecl);

    // Returns true if there's an assignment to varDecl in body
    // Example: our_var = something_else
    bool isAssignedTo(clang::Stmt *body, const clang::VarDecl *varDecl);

    // Returns true if a body of statements contains a function call that takes our variable (varDecl)
    // By ref or pointer
    bool isPassedToFunction(clang::Stmt *body, const clang::VarDecl *varDecl, bool byRefOrPtrOnly);

    // Returns true if we take the address of varDecl, such as: &foo
    bool addressIsTaken(const clang::CompilerInstance &ci, clang::Stmt *body, const clang::ValueDecl *valDecl);

    // QString::fromLatin1("foo")    -> true
    // QString::fromLatin1("foo", 1) -> false
    bool callHasDefaultArguments(clang::CallExpr *expr);

    // If there's a child of type StringLiteral, returns true
    // if allowEmpty is false, "" will be ignored
    bool containsStringLiteral(clang::Stmt *, bool allowEmpty = true, int depth = -1);

    bool isInsideOperatorCall(clang::ParentMap *map, clang::Stmt *s, const std::vector<std::string> &anyOf);
    bool insideCTORCall(clang::ParentMap *map, clang::Stmt *s, const std::vector<std::string> &anyOf);

    // returns true if the ternary operator has two string literal arguments, such as:
    // foo ? "bar" : "baz"
    bool ternaryOperatorIsOfStringLiteral(clang::ConditionalOperator*);

    bool isAssignOperator(clang::CXXOperatorCallExpr *op,
                          const std::string &className,
                          const std::string &argumentType, const clang::LangOptions &lo);

    bool isImplicitCastTo(clang::Stmt *, const std::string &);

    bool presumedLocationsEqual(const clang::PresumedLoc &l1, const clang::PresumedLoc &l2);


    // Returns the list of methods with name methodName that the class/struct record contains
    std::vector<clang::CXXMethodDecl*> methodsFromString(const clang::CXXRecordDecl *record, const std::string &methodName);

    // Returns the most derived class. (CXXMemberCallExpr::getRecordDecl() return the first base class with the method)
    // The returned callee is the name of the variable on which the member call was made:
    // o1->foo() => "o1"
    // foo() => "this"
    const clang::CXXRecordDecl* recordForMemberCall(clang::CXXMemberCallExpr *call, std::string &implicitCallee);

    bool isAscii(clang::StringLiteral *lt);

    // Checks if Statement s inside an operator* call
    bool isInDerefExpression(clang::Stmt *s, clang::ParentMap *map);

    // For a a chain called expression like foo().bar().baz() returns a list of calls
    // {baz(), bar(), foo()}

    // the parameter lastCallExpr to pass would be baz()
    // No need to specify the other callexprs, they are children of the first one, since the AST looks like:
    // - baz
    // -- bar
    // --- foo
    std::vector<clang::CallExpr *> callListForChain(clang::CallExpr *lastCallExpr);

    // Returns the first base class
    clang::CXXRecordDecl * rootBaseClass(clang::CXXRecordDecl *derived);

    // Returns the copy ctor for this class
    clang::CXXConstructorDecl *copyCtor(clang::CXXRecordDecl *);

    // Returns the copy-assignment operator for this class
    clang::CXXMethodDecl *copyAssign(clang::CXXRecordDecl *);

    bool hasMember(clang::CXXRecordDecl *record, const std::string &memberTypeName);

    /**
     * Returns true if record is a shared pointer (boost, Qt or stl only).
     */
    bool isSharedPointer(clang::CXXRecordDecl *record);

    /**
     * Returns true if varDecl is initialized externally.
     * Example:
     *     QList<Foo> list = getList(); // true
     *     QList<int> list = list2;     // true
     *     QList<int> list = {1, 2, 3}; // false
     *     QList<int> list;             // false
     */
    bool isInitializedExternally(clang::VarDecl *varDecl);

    /**
     * Returns true if declStmt refers to varDecl
     */
    bool referencesVarDecl(clang::DeclStmt *declStmt, clang::VarDecl *varDecl);

    /**
     * Returns true if the body of a function is empty.
     * Returns false if either function or it's body are null.
     */
    bool functionHasEmptyBody(clang::FunctionDecl *func);

    /**
     * If stm is an UnaryOperator or BinaryOperator that writes to the variable it returns the expression
     * that represents the variable (Usually a MemberExpr or DeclRefExpr for local variables).
     *
     * Otherwise returns nullptr.
     *
     * The operators that write to the variable are operator=, operator+=, operator++, etc.
     */
    clang::Expr* isWriteOperator(clang::Stmt *stm);
}

#endif
