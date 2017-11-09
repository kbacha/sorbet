#ifndef SRUBY_TREEMAP_H
#define SRUBY_TREEMAP_H

#include "Trees.h"
#include "memory"
#include <type_traits> // To use 'std::integral_constant'.
#include <typeinfo>

using std::make_unique;
using std::unique_ptr;

namespace ruby_typer {
namespace ast {

class FUNC_EXAMPLE {
public:
    // all members are optional, but METHOD NAMES MATTER
    // Not including the member will skip the branch
    // you may return the same pointer that you are given
    // caller is repsonsible to handle it
    ClassDef *preTransformClassDef(Context ctx, ClassDef *original);
    Expression *postTransformClassDef(Context ctx, ClassDef *original);

    MethodDef *preTransformMethodDef(Context ctx, MethodDef *original);
    Expression *postTransformMethodDef(Context ctx, MethodDef *original);

    If *preTransformIf(Context ctx, If *original);
    Expression *postTransformIf(Context ctx, If *original);

    While *preTransformWhile(Context ctx, While *original);
    Expression *postTransformWhile(Context ctx, While *original);

    For *preTransformFor(Context ctx, For *original);
    Expression *postTransformFor(Context ctx, For *original);

    Expression *postTransformBreak(Context ctx, Break *original);

    Expression *postTransformNext(Context ctx, Next *original);

    Return *preTransformReturn(Context ctx, Return *original);
    Expression *postTransformReturn(Context ctx, Return *original);

    Yield *preTransformYield(Context ctx, Yield *original);
    Expression *postTransformYield(Context ctx, Yield *original);

    Rescue *preTransformRescue(Context ctx, Rescue *original);
    Expression *postTransformRescue(Context ctx, Rescue *original);

    Expression *postTransformIdent(Context ctx, Ident *original);
    Expression *postTransformUnresolvedIdent(Context ctx, UnresolvedIdent *original);

    Assign *preTransformAssign(Context ctx, Assign *original);
    Expression *postTransformAssign(Context ctx, Assign *original);

    Send *preTransformSend(Context ctx, Send *original);
    Expression *postTransformSend(Context ctx, Send *original);

    NamedArg *preTransformNamedArg(Context ctx, NamedArg *original);
    Expression *postTransformNamedArg(Context ctx, NamedArg *original);

    Hash *preTransformHash(Context ctx, Hash *original);
    Expression *postTransformHash(Context ctx, Hash *original);

    Array *preTransformArray(Context ctx, Array *original);
    Expression *postransformArray(Context ctx, Array *original);

    Expression *postTransformBoolLit(Context ctx, BoolLit *original);

    Expression *postTransformFloatLit(Context ctx, FloatLit *original);

    Expression *postTransformIntLit(Context ctx, IntLit *original);

    Expression *postTransformStringLit(Context ctx, StringLit *original);

    Expression *postTransformConstantLit(Context ctx, ConstantLit *original);

    ArraySplat *preTransformArraySplat(Context ctx, ArraySplat *original);
    Expression *postTransformArraySplat(Context ctx, ArraySplat *original);

    HashSplat *preTransformHashSplat(Context ctx, HashSplat *original);
    Expression *postTransformHashSplat(Context ctx, HashSplat *original);

    Expression *postTransformSelf(Context ctx, Self *original);

    Block *preTransformBlock(Context ctx, Block *original);
    Expression *postTransformBlock(Context ctx, Block *original);

    InsSeq *preTransformInsSeq(Context ctx, InsSeq *original);
    Expression *postTransformInsSeq(Context ctx, InsSeq *original);
};

/**
 * GENERATE_HAS_MEMBER(att)  // Creates 'has_member_att'.
 * `HAS_MEMBER_att<C>::value` can be used to statically test if C has a member att
 */
#define GENERATE_HAS_MEMBER(X)                                                          \
    template <typename T> class HAS_MEMBER_##X {                                        \
        struct Fallback {                                                               \
            int X;                                                                      \
        };                                                                              \
        struct Derived : T, Fallback {};                                                \
                                                                                        \
        template <typename U, U> struct Check;                                          \
                                                                                        \
        typedef char ArrayOfOne[1];                                                     \
        typedef char ArrayOfTwo[2];                                                     \
                                                                                        \
        template <typename U> static ArrayOfOne &func(Check<int Fallback::*, &U::X> *); \
        template <typename U> static ArrayOfTwo &func(...);                             \
                                                                                        \
    public:                                                                             \
        typedef HAS_MEMBER_##X type;                                                    \
        enum { value = sizeof(func<Derived>(0)) == 2 };                                 \
    };

GENERATE_HAS_MEMBER(preTransformClassDef);
GENERATE_HAS_MEMBER(preTransformMethodDef);
GENERATE_HAS_MEMBER(preTransformConstDef);
GENERATE_HAS_MEMBER(preTransformIf);
GENERATE_HAS_MEMBER(preTransformWhile);
GENERATE_HAS_MEMBER(preTransformFor);
GENERATE_HAS_MEMBER(preTransformReturn);
GENERATE_HAS_MEMBER(preTransformYield);
GENERATE_HAS_MEMBER(preTransformRescue);
GENERATE_HAS_MEMBER(preTransformAssign);
GENERATE_HAS_MEMBER(preTransformSend);
GENERATE_HAS_MEMBER(preTransformNamedArg);
GENERATE_HAS_MEMBER(preTransformHash);
GENERATE_HAS_MEMBER(preTransformArray);
GENERATE_HAS_MEMBER(preTransformArraySplat);
GENERATE_HAS_MEMBER(preTransformHashSplat);
GENERATE_HAS_MEMBER(preTransformBlock);
GENERATE_HAS_MEMBER(preTransformInsSeq);

// used to check for ABSENCE of method
GENERATE_HAS_MEMBER(preTransformBreak);
GENERATE_HAS_MEMBER(preTransformNext);
GENERATE_HAS_MEMBER(preTransformIdent);
GENERATE_HAS_MEMBER(preTransformUnresolvedIdent);
GENERATE_HAS_MEMBER(preTransformBoolLit);
GENERATE_HAS_MEMBER(preTransformStringLit);
GENERATE_HAS_MEMBER(preTransformFloatLit);
GENERATE_HAS_MEMBER(preTransformIntLit);
GENERATE_HAS_MEMBER(preTransformConstantLit);
GENERATE_HAS_MEMBER(preTransformSelf);

GENERATE_HAS_MEMBER(postTransformClassDef);
GENERATE_HAS_MEMBER(postTransformMethodDef);
GENERATE_HAS_MEMBER(postTransformConstDef);
GENERATE_HAS_MEMBER(postTransformIf);
GENERATE_HAS_MEMBER(postTransformWhile);
GENERATE_HAS_MEMBER(postTransformFor);
GENERATE_HAS_MEMBER(postTransformBreak);
GENERATE_HAS_MEMBER(postTransformNext);
GENERATE_HAS_MEMBER(postTransformReturn);
GENERATE_HAS_MEMBER(postTransformYield);
GENERATE_HAS_MEMBER(postTransformRescue);
GENERATE_HAS_MEMBER(postTransformIdent);
GENERATE_HAS_MEMBER(postTransformUnresolvedIdent);
GENERATE_HAS_MEMBER(postTransformAssign);
GENERATE_HAS_MEMBER(postTransformSend);
GENERATE_HAS_MEMBER(postTransformNamedArg);
GENERATE_HAS_MEMBER(postTransformHash);
GENERATE_HAS_MEMBER(postTransformArray);
GENERATE_HAS_MEMBER(postTransformBoolLit);
GENERATE_HAS_MEMBER(postTransformFloatLit);
GENERATE_HAS_MEMBER(postTransformIntLit);
GENERATE_HAS_MEMBER(postTransformStringLit);
GENERATE_HAS_MEMBER(postTransformConstantLit);
GENERATE_HAS_MEMBER(postTransformArraySplat);
GENERATE_HAS_MEMBER(postTransformHashSplat);
GENERATE_HAS_MEMBER(postTransformSelf);
GENERATE_HAS_MEMBER(postTransformBlock);
GENERATE_HAS_MEMBER(postTransformInsSeq);

#define GENERATE_POSTPONE_PRECLASS(X)                                          \
                                                                               \
    template <class FUNC, bool has> class PostPonePreTransform_##X {           \
    public:                                                                    \
        static X *call(Context ctx, X *cd, FUNC &what) {                       \
            Error::raise("should never be called. Incorrect use of TreeMap?"); \
            return nullptr;                                                    \
        }                                                                      \
    };                                                                         \
                                                                               \
    template <class FUNC> class PostPonePreTransform_##X<FUNC, true> {         \
    public:                                                                    \
        static X *call(Context ctx, X *cd, FUNC &func) {                       \
            return func.preTransform##X(ctx, cd);                              \
        }                                                                      \
    };                                                                         \
                                                                               \
    template <class FUNC> class PostPonePreTransform_##X<FUNC, false> {        \
    public:                                                                    \
        static X *call(Context ctx, X *cd, FUNC &func) {                       \
            return cd;                                                         \
        }                                                                      \
    };

#define GENERATE_POSTPONE_POSTCLASS(X)                                         \
                                                                               \
    template <class FUNC, bool has> class PostPonePostTransform_##X {          \
    public:                                                                    \
        static Expression *call(Context ctx, X *cd, FUNC &what) {              \
            Error::raise("should never be called. Incorrect use of TreeMap?"); \
            return nullptr;                                                    \
        }                                                                      \
    };                                                                         \
                                                                               \
    template <class FUNC> class PostPonePostTransform_##X<FUNC, true> {        \
    public:                                                                    \
        static Expression *call(Context ctx, X *cd, FUNC &func) {              \
            return func.postTransform##X(ctx, cd);                             \
        }                                                                      \
    };                                                                         \
                                                                               \
    template <class FUNC> class PostPonePostTransform_##X<FUNC, false> {       \
    public:                                                                    \
        static Expression *call(Context ctx, X *cd, FUNC &func) {              \
            return cd;                                                         \
        }                                                                      \
    };

GENERATE_POSTPONE_PRECLASS(ClassDef);
GENERATE_POSTPONE_PRECLASS(MethodDef);
GENERATE_POSTPONE_PRECLASS(ConstDef);
GENERATE_POSTPONE_PRECLASS(If);
GENERATE_POSTPONE_PRECLASS(While);
GENERATE_POSTPONE_PRECLASS(For);
GENERATE_POSTPONE_PRECLASS(Return);
GENERATE_POSTPONE_PRECLASS(Yield);
GENERATE_POSTPONE_PRECLASS(Assign);
GENERATE_POSTPONE_PRECLASS(Send);
GENERATE_POSTPONE_PRECLASS(NamedArg);
GENERATE_POSTPONE_PRECLASS(Hash);
GENERATE_POSTPONE_PRECLASS(Array);
GENERATE_POSTPONE_PRECLASS(ArraySplat);
GENERATE_POSTPONE_PRECLASS(HashSplat);
GENERATE_POSTPONE_PRECLASS(Block);
GENERATE_POSTPONE_PRECLASS(InsSeq);

GENERATE_POSTPONE_POSTCLASS(ClassDef);
GENERATE_POSTPONE_POSTCLASS(MethodDef);
GENERATE_POSTPONE_POSTCLASS(ConstDef);
GENERATE_POSTPONE_POSTCLASS(If);
GENERATE_POSTPONE_POSTCLASS(While);
GENERATE_POSTPONE_POSTCLASS(For);
GENERATE_POSTPONE_POSTCLASS(Break);
GENERATE_POSTPONE_POSTCLASS(Next);
GENERATE_POSTPONE_POSTCLASS(Return);
GENERATE_POSTPONE_POSTCLASS(Yield);
GENERATE_POSTPONE_POSTCLASS(Ident);
GENERATE_POSTPONE_POSTCLASS(UnresolvedIdent);
GENERATE_POSTPONE_POSTCLASS(Assign);
GENERATE_POSTPONE_POSTCLASS(Send);
GENERATE_POSTPONE_POSTCLASS(NamedArg);
GENERATE_POSTPONE_POSTCLASS(Hash);
GENERATE_POSTPONE_POSTCLASS(Array);
GENERATE_POSTPONE_POSTCLASS(BoolLit);
GENERATE_POSTPONE_POSTCLASS(FloatLit);
GENERATE_POSTPONE_POSTCLASS(IntLit);
GENERATE_POSTPONE_POSTCLASS(StringLit);
GENERATE_POSTPONE_POSTCLASS(ConstantLit);
GENERATE_POSTPONE_POSTCLASS(ArraySplat);
GENERATE_POSTPONE_POSTCLASS(HashSplat);
GENERATE_POSTPONE_POSTCLASS(Self);
GENERATE_POSTPONE_POSTCLASS(Block);
GENERATE_POSTPONE_POSTCLASS(InsSeq);

/**
 * Given a tree transformer FUNC transform a tree.
 * Tree is guaranteed to be visited in the definition order.
 * FUNC may maintain internal state.
 * @tparam tree transformer, see FUNC_EXAMPLE
 */
template <class FUNC> class TreeMap {
private:
    FUNC &func;

    static_assert(!HAS_MEMBER_preTransformBreak<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformNext<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformIdent<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformUnresolvedIdent<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformBoolLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformStringLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformFloatLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformIntLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformConstantLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformSelf<FUNC>::value, "use post*Transform instead");

    TreeMap(FUNC &func) : func(func) {}

    Expression *mapIt(Expression *what, Context ctx) {
        // TODO: reorder by frequency
        if (what == nullptr || dynamic_cast<EmptyTree *>(what) != nullptr || dynamic_cast<Nil *>(what) != nullptr)
            return what;
        if (ClassDef *v = dynamic_cast<ClassDef *>(what)) {
            if (HAS_MEMBER_preTransformClassDef<FUNC>::value) {
                v = PostPonePreTransform_ClassDef<FUNC, HAS_MEMBER_preTransformClassDef<FUNC>::value>::call(ctx, v,
                                                                                                            func);
            }

            for (auto &def : v->rhs) {
                auto originalDef = def.get();
                auto newDef = mapIt(originalDef, ctx.withOwner(v->symbol));
                if (newDef != originalDef) {
                    def.reset(newDef);
                }
            }

            if (HAS_MEMBER_postTransformClassDef<FUNC>::value) {
                return PostPonePostTransform_ClassDef<FUNC, HAS_MEMBER_postTransformClassDef<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
            }

            return v;
        } else if (MethodDef *v = dynamic_cast<MethodDef *>(what)) {
            if (HAS_MEMBER_preTransformMethodDef<FUNC>::value) {
                v = PostPonePreTransform_MethodDef<FUNC, HAS_MEMBER_preTransformMethodDef<FUNC>::value>::call(ctx, v,
                                                                                                              func);
            }
            auto originalRhs = v->rhs.get();
            auto newRhs = mapIt(originalRhs, ctx.withOwner(v->symbol));
            if (newRhs != originalRhs) {
                Error::check(dynamic_cast<Expression *>(newRhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expression *>(newRhs));
            }

            if (HAS_MEMBER_postTransformMethodDef<FUNC>::value) {
                return PostPonePostTransform_MethodDef<FUNC, HAS_MEMBER_postTransformMethodDef<FUNC>::value>::call(
                    ctx, v, func);
            }

            return v;
        } else if (ConstDef *v = dynamic_cast<ConstDef *>(what)) {
            if (HAS_MEMBER_preTransformConstDef<FUNC>::value) {
                v = PostPonePreTransform_ConstDef<FUNC, HAS_MEMBER_preTransformConstDef<FUNC>::value>::call(ctx, v,
                                                                                                            func);
            }
            auto originalRhs = v->rhs.get();
            auto newRhs = mapIt(originalRhs, ctx.withOwner(v->symbol));
            if (newRhs != originalRhs) {
                Error::check(dynamic_cast<Expression *>(newRhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expression *>(newRhs));
            }

            if (HAS_MEMBER_postTransformConstDef<FUNC>::value) {
                return PostPonePostTransform_ConstDef<FUNC, HAS_MEMBER_postTransformConstDef<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
            }

            return v;
        } else if (If *v = dynamic_cast<If *>(what)) {
            if (HAS_MEMBER_preTransformIf<FUNC>::value) {
                v = PostPonePreTransform_If<FUNC, HAS_MEMBER_preTransformIf<FUNC>::value>::call(ctx, v, func);
            }
            auto originalCond = v->cond.get();
            auto originalThen = v->thenp.get();
            auto originalElse = v->elsep.get();
            auto newCond = mapIt(originalCond, ctx);
            auto newThen = mapIt(originalThen, ctx);
            auto newElse = mapIt(originalElse, ctx);
            if (newCond != originalCond) {
                Error::check(dynamic_cast<Expression *>(newCond) != nullptr);
                v->cond.reset(dynamic_cast<Expression *>(newCond));
            }
            if (originalThen != newThen) {
                Error::check(dynamic_cast<Expression *>(newThen) != nullptr);
                v->thenp.reset(dynamic_cast<Expression *>(newThen));
            }
            if (originalElse != newElse) {
                Error::check(dynamic_cast<Expression *>(newElse) != nullptr);
                v->elsep.reset(dynamic_cast<Expression *>(newElse));
            }
            if (HAS_MEMBER_postTransformIf<FUNC>::value) {
                return PostPonePostTransform_If<FUNC, HAS_MEMBER_postTransformIf<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (While *v = dynamic_cast<While *>(what)) {
            if (HAS_MEMBER_preTransformWhile<FUNC>::value) {
                v = PostPonePreTransform_While<FUNC, HAS_MEMBER_preTransformWhile<FUNC>::value>::call(ctx, v, func);
            }
            auto originalCond = v->cond.get();
            auto originalBody = v->body.get();
            auto newCond = mapIt(originalCond, ctx);
            auto newBody = mapIt(originalBody, ctx);
            if (newCond != originalCond) {
                Error::check(dynamic_cast<Expression *>(newCond) != nullptr);
                v->cond.reset(dynamic_cast<Expression *>(newCond));
            }
            if (newBody != originalBody) {
                v->body.reset(newBody);
            }

            if (HAS_MEMBER_postTransformWhile<FUNC>::value) {
                return PostPonePostTransform_While<FUNC, HAS_MEMBER_postTransformWhile<FUNC>::value>::call(ctx, v,
                                                                                                           func);
            }

            return v;
        } else if (For *v = dynamic_cast<For *>(what)) {
            Error::notImplemented();
        } else if (Break *v = dynamic_cast<Break *>(what)) {
            if (HAS_MEMBER_postTransformBreak<FUNC>::value) {
                return PostPonePostTransform_Break<FUNC, HAS_MEMBER_postTransformBreak<FUNC>::value>::call(ctx, v,
                                                                                                           func);
            }
            return v;
        } else if (Next *v = dynamic_cast<Next *>(what)) {
            if (HAS_MEMBER_postTransformNext<FUNC>::value) {
                return PostPonePostTransform_Next<FUNC, HAS_MEMBER_postTransformNext<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (Return *v = dynamic_cast<Return *>(what)) {
            if (HAS_MEMBER_preTransformReturn<FUNC>::value) {
                v = PostPonePreTransform_Return<FUNC, HAS_MEMBER_preTransformReturn<FUNC>::value>::call(ctx, v, func);
            }
            auto oexpr = v->expr.get();
            auto nexpr = mapIt(oexpr, ctx);
            if (oexpr != nexpr) {
                Error::check(dynamic_cast<Expression *>(nexpr) != nullptr);
                v->expr.reset(dynamic_cast<Expression *>(nexpr));
            }

            if (HAS_MEMBER_postTransformReturn<FUNC>::value) {
                return PostPonePostTransform_Return<FUNC, HAS_MEMBER_postTransformReturn<FUNC>::value>::call(ctx, v,
                                                                                                             func);
            }

            return v;
        } else if (Yield *v = dynamic_cast<Yield *>(what)) {
            if (HAS_MEMBER_preTransformYield<FUNC>::value) {
                v = PostPonePreTransform_Yield<FUNC, HAS_MEMBER_preTransformYield<FUNC>::value>::call(ctx, v, func);
            }
            auto oexpr = v->expr.get();
            auto nexpr = mapIt(oexpr, ctx);
            if (oexpr != nexpr) {
                Error::check(dynamic_cast<Expression *>(nexpr) != nullptr);
                v->expr.reset(dynamic_cast<Expression *>(nexpr));
            }

            if (HAS_MEMBER_postTransformYield<FUNC>::value) {
                return PostPonePostTransform_Yield<FUNC, HAS_MEMBER_postTransformYield<FUNC>::value>::call(ctx, v,
                                                                                                           func);
            }

            return v;
        } else if (Ident *v = dynamic_cast<Ident *>(what)) {
            if (HAS_MEMBER_postTransformIdent<FUNC>::value) {
                return PostPonePostTransform_Ident<FUNC, HAS_MEMBER_postTransformIdent<FUNC>::value>::call(ctx, v,
                                                                                                           func);
            }
            return v;
        } else if (UnresolvedIdent *v = dynamic_cast<UnresolvedIdent *>(what)) {
            if (HAS_MEMBER_postTransformUnresolvedIdent<FUNC>::value) {
                return PostPonePostTransform_UnresolvedIdent<
                    FUNC, HAS_MEMBER_postTransformUnresolvedIdent<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (Assign *v = dynamic_cast<Assign *>(what)) {
            if (HAS_MEMBER_preTransformAssign<FUNC>::value) {
                v = PostPonePreTransform_Assign<FUNC, HAS_MEMBER_preTransformAssign<FUNC>::value>::call(ctx, v, func);
            }
            auto olhs = v->lhs.get();
            auto orhs = v->rhs.get();
            auto nlhs = mapIt(olhs, ctx);
            auto nrhs = mapIt(orhs, ctx);
            if (nlhs != olhs) {
                Error::check(dynamic_cast<Expression *>(nlhs) != nullptr);
                v->lhs.reset(dynamic_cast<Expression *>(nlhs));
            }
            if (nrhs != orhs) {
                Error::check(dynamic_cast<Expression *>(nrhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expression *>(nrhs));
            }

            if (HAS_MEMBER_postTransformAssign<FUNC>::value) {
                return PostPonePostTransform_Assign<FUNC, HAS_MEMBER_postTransformAssign<FUNC>::value>::call(ctx, v,
                                                                                                             func);
            }

            return v;
        } else if (Send *v = dynamic_cast<Send *>(what)) {
            if (HAS_MEMBER_preTransformSend<FUNC>::value) {
                v = PostPonePreTransform_Send<FUNC, HAS_MEMBER_preTransformSend<FUNC>::value>::call(ctx, v, func);
            }
            auto orecv = v->recv.get();
            auto nrecv = mapIt(orecv, ctx);
            auto &vec = v->args;
            if (nrecv != orecv) {
                Error::check(dynamic_cast<Expression *>(nrecv) != nullptr);
                v->recv.reset(dynamic_cast<Expression *>(nrecv));
            }
            auto i = 0;
            while (i < vec.size()) {
                auto &el = vec[i];
                auto oarg = el.get();
                auto narg = mapIt(oarg, ctx);
                if (oarg != narg) {
                    Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                    el.reset(dynamic_cast<Expression *>(narg));
                }
                i++;
            }

            if (v->block != nullptr) {
                auto oblock = v->block.get();
                auto nblock = mapIt(oblock, ctx);
                if (oblock != nblock) {
                    Error::check(dynamic_cast<Block *>(nblock) != nullptr);
                    v->block.reset(dynamic_cast<Block *>(nblock));
                }
            }

            if (HAS_MEMBER_postTransformSend<FUNC>::value) {
                return PostPonePostTransform_Send<FUNC, HAS_MEMBER_postTransformSend<FUNC>::value>::call(ctx, v, func);
            }

            return v;
        } else if (NamedArg *v = dynamic_cast<NamedArg *>(what)) {
            if (HAS_MEMBER_preTransformNamedArg<FUNC>::value) {
                v = PostPonePreTransform_NamedArg<FUNC, HAS_MEMBER_preTransformNamedArg<FUNC>::value>::call(ctx, v,
                                                                                                            func);
            }
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg, ctx);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expression *>(narg));
            }

            if (HAS_MEMBER_postTransformNamedArg<FUNC>::value) {
                return PostPonePostTransform_NamedArg<FUNC, HAS_MEMBER_postTransformNamedArg<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
            }

            return v;
        } else if (Hash *v = dynamic_cast<Hash *>(what)) {
            if (HAS_MEMBER_preTransformHash<FUNC>::value) {
                v = PostPonePreTransform_Hash<FUNC, HAS_MEMBER_preTransformHash<FUNC>::value>::call(ctx, v, func);
            }
            int i = 0;
            while (i < v->keys.size()) {
                auto &el = v->keys[i];
                auto oarg = el.get();
                auto narg = mapIt(oarg, ctx);
                if (oarg != narg) {
                    Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                    el.reset(dynamic_cast<Expression *>(narg));
                }
                i++;
            }

            i = 0;
            while (i < v->values.size()) {
                auto &el = v->values[i];
                auto oarg = el.get();
                auto narg = mapIt(oarg, ctx);
                if (oarg != narg) {
                    Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                    el.reset(dynamic_cast<Expression *>(narg));
                }
                i++;
            }

            if (HAS_MEMBER_postTransformArray<FUNC>::value) {
                return PostPonePostTransform_Hash<FUNC, HAS_MEMBER_postTransformHash<FUNC>::value>::call(ctx, v, func);
            }
            return what;
        } else if (Array *v = dynamic_cast<Array *>(what)) {
            if (HAS_MEMBER_preTransformArray<FUNC>::value) {
                v = PostPonePreTransform_Array<FUNC, HAS_MEMBER_preTransformArray<FUNC>::value>::call(ctx, v, func);
            }
            int i = 0;
            while (i < v->elems.size()) {
                auto &el = v->elems[i];
                auto oarg = el.get();
                auto narg = mapIt(oarg, ctx);
                if (oarg != narg) {
                    Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                    el.reset(dynamic_cast<Expression *>(narg));
                }
                i++;
            }

            if (HAS_MEMBER_postTransformArray<FUNC>::value) {
                return PostPonePostTransform_Array<FUNC, HAS_MEMBER_postTransformArray<FUNC>::value>::call(ctx, v,
                                                                                                           func);
            }
            return what;
        } else if (FloatLit *v = dynamic_cast<FloatLit *>(what)) {
            if (HAS_MEMBER_postTransformFloatLit<FUNC>::value) {
                return PostPonePostTransform_FloatLit<FUNC, HAS_MEMBER_postTransformFloatLit<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
            }
            return v;
        } else if (BoolLit *v = dynamic_cast<BoolLit *>(what)) {
            if (HAS_MEMBER_postTransformBoolLit<FUNC>::value) {
                return PostPonePostTransform_BoolLit<FUNC, HAS_MEMBER_postTransformBoolLit<FUNC>::value>::call(ctx, v,
                                                                                                               func);
            }
            return v;
        } else if (IntLit *v = dynamic_cast<IntLit *>(what)) {
            if (HAS_MEMBER_postTransformIntLit<FUNC>::value) {
                return PostPonePostTransform_IntLit<FUNC, HAS_MEMBER_postTransformIntLit<FUNC>::value>::call(ctx, v,
                                                                                                             func);
            }
            return v;
        } else if (StringLit *v = dynamic_cast<StringLit *>(what)) {
            if (HAS_MEMBER_postTransformStringLit<FUNC>::value) {
                return PostPonePostTransform_StringLit<FUNC, HAS_MEMBER_postTransformStringLit<FUNC>::value>::call(
                    ctx, v, func);
            }
            return v;
        } else if (ConstantLit *v = dynamic_cast<ConstantLit *>(what)) {
            if (HAS_MEMBER_postTransformConstantLit<FUNC>::value) {
                return PostPonePostTransform_ConstantLit<FUNC, HAS_MEMBER_postTransformConstantLit<FUNC>::value>::call(
                    ctx, v, func);
            }
            return v;
        } else if (BoolLit *v = dynamic_cast<BoolLit *>(what)) {
            // Error::notImplemented();
            return what;
        } else if (ArraySplat *v = dynamic_cast<ArraySplat *>(what)) {
            if (HAS_MEMBER_preTransformArraySplat<FUNC>::value) {
                v = PostPonePreTransform_ArraySplat<FUNC, HAS_MEMBER_preTransformArraySplat<FUNC>::value>::call(ctx, v,
                                                                                                                func);
            }
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg, ctx);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expression *>(narg));
            }

            if (HAS_MEMBER_postTransformArraySplat<FUNC>::value) {
                return PostPonePostTransform_ArraySplat<FUNC, HAS_MEMBER_postTransformArraySplat<FUNC>::value>::call(
                    ctx, v, func);
            }

            return v;
        } else if (HashSplat *v = dynamic_cast<HashSplat *>(what)) {
            if (HAS_MEMBER_preTransformHashSplat<FUNC>::value) {
                v = PostPonePreTransform_HashSplat<FUNC, HAS_MEMBER_preTransformHashSplat<FUNC>::value>::call(ctx, v,
                                                                                                              func);
            }
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg, ctx);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expression *>(narg));
            }

            if (HAS_MEMBER_postTransformHashSplat<FUNC>::value) {
                return PostPonePostTransform_HashSplat<FUNC, HAS_MEMBER_postTransformHashSplat<FUNC>::value>::call(
                    ctx, v, func);
            }
            return v;
        } else if (SymbolLit *v = dynamic_cast<SymbolLit *>(what)) {
            return what;
        } else if (Super *v = dynamic_cast<Super *>(what)) {
            // TODO Need to recursivly walk arguments
            return what;
        } else if (Self *v = dynamic_cast<Self *>(what)) {
            if (HAS_MEMBER_postTransformSelf<FUNC>::value) {
                return PostPonePostTransform_Self<FUNC, HAS_MEMBER_postTransformSelf<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (Block *v = dynamic_cast<Block *>(what)) {
            if (HAS_MEMBER_preTransformBlock<FUNC>::value) {
                v = PostPonePreTransform_Block<FUNC, HAS_MEMBER_preTransformBlock<FUNC>::value>::call(ctx, v, func);
            }

            auto originalBody = v->body.get();
            auto newBody = mapIt(originalBody, ctx.withOwner(v->symbol));
            if (newBody != originalBody) {
                Error::check(dynamic_cast<Expression *>(newBody) != nullptr);
                v->body.reset(dynamic_cast<Expression *>(newBody));
            }

            if (HAS_MEMBER_postTransformBlock<FUNC>::value) {
                return PostPonePostTransform_Block<FUNC, HAS_MEMBER_postTransformBlock<FUNC>::value>::call(ctx, v,
                                                                                                           func);
            }
            return v;
        } else if (InsSeq *v = dynamic_cast<InsSeq *>(what)) {
            if (HAS_MEMBER_preTransformInsSeq<FUNC>::value) {
                v = PostPonePreTransform_InsSeq<FUNC, HAS_MEMBER_preTransformInsSeq<FUNC>::value>::call(ctx, v, func);
            }
            auto &stats = v->stats;
            auto oexpr = v->expr.get();
            auto i = 0;
            while (i < stats.size()) {
                auto &el = stats[i];
                auto oexp = el.get();
                auto nexp = mapIt(oexp, ctx);
                if (oexp != nexp) {
                    el.reset(nexp);
                }
                i++;
            }
            auto nexpr = mapIt(oexpr, ctx);
            if (nexpr != oexpr) {
                Error::check(dynamic_cast<Expression *>(nexpr) != nullptr);
                v->expr.reset(dynamic_cast<Expression *>(nexpr));
            }

            if (HAS_MEMBER_postTransformInsSeq<FUNC>::value) {
                return PostPonePostTransform_InsSeq<FUNC, HAS_MEMBER_postTransformInsSeq<FUNC>::value>::call(ctx, v,
                                                                                                             func);
            }

            return v;
        } else if (NotSupported *v = dynamic_cast<NotSupported *>(what)) {
            return what;
        } else {
            Error::raise("should never happen. Forgot to add new tree kind?", demangle(typeid(what).name()));
        }
    }

public:
    static unique_ptr<Expression> apply(Context ctx, FUNC &func, unique_ptr<Expression> to) {
        Expression *underlying = to.get();
        TreeMap walker(func);
        Expression *res = walker.mapIt(underlying, ctx);

        if (res == underlying) {
            return to;
        } else {
            return std::unique_ptr<Expression>(res);
        }
    }
};

} // namespace ast
} // namespace ruby_typer

#endif // SRUBY_TREEMAP_H
