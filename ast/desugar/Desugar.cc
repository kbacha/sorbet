#include <algorithm>

#include "../ast.h"
#include "Desugar.h"
#include "ast/ast.h"
#include "common/common.h"

namespace ruby_typer {
namespace ast {
namespace desugar {

using namespace std;

namespace {

unique_ptr<Expression> node2TreeImpl(Context ctx, unique_ptr<parser::Node> &what);

unique_ptr<Expression> mkSend(Loc loc, unique_ptr<Expression> &recv, NameRef fun, vector<unique_ptr<Expression>> &args,
                              u4 flags = 0) {
    Error::check(dynamic_cast<Expression *>(recv.get()));
    auto send = make_unique<Send>(loc, move(recv), fun, move(args));
    send->flags = flags;
    return move(send);
}

unique_ptr<Expression> mkSend1(Loc loc, unique_ptr<Expression> &recv, NameRef fun, unique_ptr<Expression> &arg1) {
    auto nargs = vector<unique_ptr<Expression>>();
    nargs.emplace_back(move(arg1));
    return make_unique<Send>(loc, move(recv), fun, move(nargs));
}

unique_ptr<Expression> mkSend1(Loc loc, unique_ptr<Expression> &&recv, NameRef fun, unique_ptr<Expression> &&arg1) {
    return mkSend1(loc, recv, fun, arg1);
}

unique_ptr<Expression> mkSend1(Loc loc, unique_ptr<Expression> &&recv, NameRef fun, unique_ptr<Expression> &arg1) {
    return mkSend1(loc, recv, fun, arg1);
}

unique_ptr<Expression> mkSend1(Loc loc, unique_ptr<Expression> &recv, NameRef fun, unique_ptr<Expression> &&arg1) {
    return mkSend1(loc, recv, fun, arg1);
}

unique_ptr<Expression> mkSend0(Loc loc, unique_ptr<Expression> &recv, NameRef fun) {
    auto nargs = vector<unique_ptr<Expression>>();
    return make_unique<Send>(loc, move(recv), fun, move(nargs));
}

unique_ptr<Expression> mkSend0(Loc loc, unique_ptr<Expression> &&recv, NameRef fun) {
    return mkSend0(loc, recv, fun);
}

unique_ptr<Expression> mkIdent(Loc loc, SymbolRef symbol) {
    return make_unique<Ident>(loc, symbol);
}

unique_ptr<Expression> cpRef(Loc loc, Reference &name) {
    if (UnresolvedIdent *nm = dynamic_cast<UnresolvedIdent *>(&name))
        return make_unique<UnresolvedIdent>(loc, nm->kind, nm->name);
    if (Ident *id = dynamic_cast<Ident *>(&name))
        return make_unique<Ident>(loc, id->symbol);
    Error::notImplemented();
}

unique_ptr<Expression> mkAssign(Loc loc, unique_ptr<Expression> &lhs, unique_ptr<Expression> &rhs) {
    return make_unique<Assign>(loc, move(lhs), move(rhs));
}

unique_ptr<Expression> mkAssign(Loc loc, SymbolRef symbol, unique_ptr<Expression> &rhs) {
    auto id = mkIdent(loc, symbol);
    return mkAssign(loc, id, rhs);
}

unique_ptr<Expression> mkAssign(Loc loc, SymbolRef symbol, unique_ptr<Expression> &&rhs) {
    return mkAssign(loc, symbol, rhs);
}

unique_ptr<Expression> mkIf(Loc loc, unique_ptr<Expression> &cond, unique_ptr<Expression> &thenp,
                            unique_ptr<Expression> &elsep) {
    return make_unique<If>(loc, move(cond), move(thenp), move(elsep));
}

unique_ptr<Expression> mkIf(Loc loc, unique_ptr<Expression> &&cond, unique_ptr<Expression> &&thenp,
                            unique_ptr<Expression> &&elsep) {
    return mkIf(loc, cond, thenp, elsep);
}

unique_ptr<Expression> mkEmptyTree(Loc loc) {
    return make_unique<EmptyTree>(loc);
}

unique_ptr<Expression> mkInsSeq(Loc loc, vector<unique_ptr<Expression>> &&stats, unique_ptr<Expression> &&expr) {
    return make_unique<InsSeq>(loc, move(stats), move(expr));
}

unique_ptr<Expression> mkInsSeq1(Loc loc, unique_ptr<Expression> stat, unique_ptr<Expression> &&expr) {
    vector<unique_ptr<Expression>> stats;
    stats.emplace_back(move(stat));
    return make_unique<InsSeq>(loc, move(stats), move(expr));
}

unique_ptr<Expression> mkTrue(Loc loc) {
    return make_unique<BoolLit>(loc, true);
}

unique_ptr<Expression> mkFalse(Loc loc) {
    return make_unique<BoolLit>(loc, false);
}

unique_ptr<MethodDef> buildMethod(Context ctx, Loc loc, NameRef name, unique_ptr<parser::Node> &argnode,
                                  unique_ptr<parser::Node> &body) {
    vector<unique_ptr<Expression>> args;
    if (auto *oargs = dynamic_cast<parser::Args *>(argnode.get())) {
        for (auto &arg : oargs->args) {
            args.emplace_back(node2TreeImpl(ctx, arg));
        }
    } else if (argnode.get() == nullptr) {
        // do nothing
    } else {
        Error::notImplemented();
    }
    return make_unique<MethodDef>(loc, ctx.state.defn_todo(), name, args, node2TreeImpl(ctx, body), false);
}

unique_ptr<Expression> node2TreeImpl(Context ctx, unique_ptr<parser::Node> &what) {
    if (what.get() == nullptr) {
        return make_unique<EmptyTree>(Loc::none(0));
    }
    if (what->loc.is_none()) {
        DEBUG_ONLY(Error::check(false, "parse-tree node has no location: ", what->toString(ctx)));
    }
    unique_ptr<Expression> result;
    typecase(
        what.get(),
        [&](parser::And *a) {
            auto lhs = node2TreeImpl(ctx, a->left);
            if (auto i = dynamic_cast<Reference *>(lhs.get())) {
                auto cond = cpRef(what->loc, *i);
                auto iff = mkIf(what->loc, move(cond), node2TreeImpl(ctx, a->right), move(lhs));
                result.swap(iff);
            } else {
                SymbolRef tempSym = ctx.state.newTemporary(UniqueNameKind::Desugar, Names::andAnd(), ctx.owner);
                auto temp = mkAssign(what->loc, tempSym, lhs);

                auto iff = mkIf(what->loc, mkIdent(what->loc, tempSym), node2TreeImpl(ctx, a->right),
                                mkIdent(what->loc, tempSym));
                auto wrapped = mkInsSeq1(what->loc, move(temp), move(iff));

                result.swap(wrapped);
            }
        },
        [&](parser::Or *a) {
            auto lhs = node2TreeImpl(ctx, a->left);
            if (auto i = dynamic_cast<Reference *>(lhs.get())) {
                auto cond = cpRef(what->loc, *i);
                mkIf(what->loc, move(cond), move(lhs), node2TreeImpl(ctx, a->right));
            } else {
                SymbolRef tempSym = ctx.state.newTemporary(UniqueNameKind::Desugar, Names::orOr(), ctx.owner);
                auto temp = mkAssign(what->loc, tempSym, lhs);

                auto iff = mkIf(what->loc, mkIdent(what->loc, tempSym), mkIdent(what->loc, tempSym),
                                node2TreeImpl(ctx, a->right));
                auto wrapped = mkInsSeq1(what->loc, move(temp), move(iff));

                result.swap(wrapped);
            }
        },
        [&](parser::AndAsgn *a) {
            auto recv = node2TreeImpl(ctx, a->left);
            auto arg = node2TreeImpl(ctx, a->right);
            if (auto s = dynamic_cast<Send *>(recv.get())) {
                Error::check(s->args.empty());
                SymbolRef tempSym = ctx.state.newTemporary(UniqueNameKind::Desugar, s->fun, ctx.owner);
                auto temp = mkAssign(what->loc, tempSym, move(s->recv));
                recv.reset();
                auto cond = mkSend0(what->loc, mkIdent(what->loc, tempSym), s->fun);
                auto body = mkSend1(what->loc, mkIdent(what->loc, tempSym), s->fun.addEq(), arg);
                auto elsep = mkIdent(what->loc, tempSym);
                auto iff = mkIf(what->loc, cond, body, elsep);
                auto wrapped = mkInsSeq1(what->loc, move(temp), move(iff));
                result.swap(wrapped);
            } else if (auto i = dynamic_cast<Reference *>(recv.get())) {
                auto cond = cpRef(what->loc, *i);
                auto body = mkAssign(what->loc, recv, arg);
                auto elsep = cpRef(what->loc, *i);
                auto iff = mkIf(what->loc, cond, body, elsep);
                result.swap(iff);
            } else {
                Error::notImplemented();
            }
        },
        [&](parser::OrAsgn *a) {
            auto recv = node2TreeImpl(ctx, a->left);
            auto arg = node2TreeImpl(ctx, a->right);
            if (auto s = dynamic_cast<Send *>(recv.get())) {
                Error::check(s->args.empty());
                SymbolRef tempSym = ctx.state.newTemporary(UniqueNameKind::Desugar, s->fun, ctx.owner);
                auto temp = mkAssign(what->loc, tempSym, move(s->recv));
                recv.reset();
                auto cond = mkSend0(what->loc, mkIdent(what->loc, tempSym), s->fun);
                auto body = mkSend1(what->loc, mkIdent(what->loc, tempSym), s->fun.addEq(), arg);
                auto elsep = mkIdent(what->loc, tempSym);
                auto iff = mkIf(what->loc, cond, elsep, body);
                auto wrapped = mkInsSeq1(what->loc, move(temp), move(iff));
                result.swap(wrapped);
            } else if (auto i = dynamic_cast<Reference *>(recv.get())) {
                auto cond = cpRef(what->loc, *i);
                auto body = mkAssign(what->loc, recv, arg);
                auto elsep = cpRef(what->loc, *i);
                auto iff = mkIf(what->loc, cond, elsep, body);
                result.swap(iff);

            } else {
                Error::notImplemented();
            }
        },
        [&](parser::Send *a) {
            u4 flags = 0;
            auto rec = node2TreeImpl(ctx, a->receiver);
            if (dynamic_cast<EmptyTree *>(rec.get()) != nullptr) {
                rec = make_unique<Self>(what->loc, SymbolRef(0));
                flags |= Send::PRIVATE_OK;
            }
            vector<unique_ptr<Expression>> args;
            for (auto &stat : a->args) {
                args.emplace_back(node2TreeImpl(ctx, stat));
            };

            auto send = mkSend(what->loc, rec, a->method, args, flags);
            result.swap(send);
        },
        [&](parser::Self *a) {
            unique_ptr<Expression> self = make_unique<Self>(what->loc, SymbolRef(0));
            result.swap(self);
        },
        [&](parser::DString *a) {
            auto it = a->nodes.begin();
            auto end = a->nodes.end();
            Error::check(it != end);
            unique_ptr<Expression> res;
            unique_ptr<Expression> first = node2TreeImpl(ctx, *it);
            if (dynamic_cast<StringLit *>(first.get()) == nullptr) {
                res = mkSend0(what->loc, first, Names::to_s());
            } else {
                res = move(first);
            }
            ++it;
            for (; it != end; ++it) {
                auto &stat = *it;
                unique_ptr<Expression> narg = node2TreeImpl(ctx, stat);
                if (dynamic_cast<StringLit *>(narg.get()) == nullptr) {
                    narg = mkSend0(what->loc, narg, Names::to_s());
                }
                auto n = mkSend1(what->loc, move(res), Names::concat(), move(narg));
                res.reset(n.release());
            };

            result.swap(res);
        },
        [&](parser::Symbol *a) {
            unique_ptr<Expression> res = make_unique<ast::SymbolLit>(what->loc, a->val);
            result.swap(res);
        },
        [&](parser::String *a) {
            unique_ptr<Expression> res = make_unique<StringLit>(what->loc, a->val);
            result.swap(res);
        },
        [&](parser::FileLiteral *a) {
            unique_ptr<Expression> res = make_unique<StringLit>(what->loc, Names::currentFile());
            result.swap(res);
        },
        [&](parser::Const *a) {
            auto scope = node2TreeImpl(ctx, a->scope);
            unique_ptr<Expression> res = make_unique<ConstantLit>(what->loc, move(scope), a->name);
            result.swap(res);
        },
        [&](parser::ConstLhs *a) {
            auto scope = node2TreeImpl(ctx, a->scope);
            unique_ptr<Expression> res = make_unique<ConstantLit>(what->loc, move(scope), a->name);
            result.swap(res);
        },
        [&](parser::Cbase *a) {
            unique_ptr<Expression> res = mkIdent(what->loc, GlobalState::defn_root());
            result.swap(res);
        },
        [&](parser::Begin *a) {
            if (a->stmts.size() > 0) {
                vector<unique_ptr<Expression>> stats;
                auto end = a->stmts.end();
                --end;
                for (auto it = a->stmts.begin(); it != end; ++it) {
                    auto &stat = *it;
                    stats.emplace_back(node2TreeImpl(ctx, stat));
                };
                auto &last = a->stmts.back();
                auto expr = node2TreeImpl(ctx, last);
                if (auto *epx = dynamic_cast<Expression *>(expr.get())) {
                    auto block = mkInsSeq(what->loc, move(stats), move(expr));
                    result.swap(block);
                } else {
                    stats.emplace_back(move(expr));
                    auto block = mkInsSeq(what->loc, move(stats), mkEmptyTree(what->loc));
                    result.swap(block);
                }
            } else {
                unique_ptr<Expression> res = mkEmptyTree(what->loc);
                result.swap(res);
            }
        },
        [&](parser::Kwbegin *a) {
            if (a->stmts.size() > 0) {
                vector<unique_ptr<Expression>> stats;
                auto end = a->stmts.end();
                --end;
                for (auto it = a->stmts.begin(); it != end; ++it) {
                    auto &stat = *it;
                    stats.emplace_back(node2TreeImpl(ctx, stat));
                };
                auto &last = a->stmts.back();
                auto expr = node2TreeImpl(ctx, last);
                if (auto *epx = dynamic_cast<Expression *>(expr.get())) {
                    auto block = mkInsSeq(what->loc, move(stats), move(expr));
                    result.swap(block);
                } else {
                    stats.emplace_back(move(expr));
                    auto block = mkInsSeq(what->loc, move(stats), mkEmptyTree(what->loc));
                    result.swap(block);
                }
            } else {
                unique_ptr<Expression> res = mkEmptyTree(what->loc);
                result.swap(res);
            }
        },
        [&](parser::Module *module) {
            ClassDef::RHS_store body;
            if (auto *a = dynamic_cast<parser::Begin *>(module->body.get())) {
                for (auto &stat : a->stmts) {
                    body.emplace_back(node2TreeImpl(ctx, stat));
                };
            } else {
                body.emplace_back(node2TreeImpl(ctx, module->body));
            }
            ClassDef::ANCESTORS_store ancestors;
            unique_ptr<Expression> res =
                make_unique<ClassDef>(what->loc, ctx.state.defn_todo(), node2TreeImpl(ctx, module->name), ancestors,
                                      body, ClassDefKind::Module);
            result.swap(res);
        },
        [&](parser::Class *claz) {
            ClassDef::RHS_store body;
            if (auto *a = dynamic_cast<parser::Begin *>(claz->body.get())) {
                for (auto &stat : a->stmts) {
                    body.emplace_back(node2TreeImpl(ctx, stat));
                };
            } else {
                body.emplace_back(node2TreeImpl(ctx, claz->body));
            }
            ClassDef::ANCESTORS_store ancestors;
            if (claz->superclass != nullptr) {
                ancestors.emplace_back(node2TreeImpl(ctx, claz->superclass));
            }

            unique_ptr<Expression> res = make_unique<ClassDef>(
                what->loc, ctx.state.defn_todo(), node2TreeImpl(ctx, claz->name), ancestors, body, ClassDefKind::Class);
            result.swap(res);
        },
        [&](parser::Arg *arg) {
            unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name);
            result.swap(res);
        },
        [&](parser::Restarg *arg) {
            unique_ptr<Expression> res = make_unique<RestArg>(
                what->loc, make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name));
            result.swap(res);
        },
        [&](parser::Kwrestarg *arg) {
            unique_ptr<Expression> res = make_unique<RestArg>(
                what->loc, make_unique<KeywordArg>(
                               what->loc, make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name)));
            result.swap(res);
        },
        [&](parser::Kwarg *arg) {
            unique_ptr<Expression> res = make_unique<KeywordArg>(
                what->loc, make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name));
            result.swap(res);
        },
        [&](parser::Blockarg *arg) {
            unique_ptr<Expression> res = make_unique<BlockArg>(
                what->loc, make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name));
            result.swap(res);
        },
        [&](parser::Kwoptarg *arg) {
            unique_ptr<Expression> res = make_unique<OptionalArg>(
                what->loc,
                make_unique<KeywordArg>(what->loc,
                                        make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name)),
                node2TreeImpl(ctx, arg->default_));
            result.swap(res);
        },
        [&](parser::Optarg *arg) {
            unique_ptr<Expression> res = make_unique<OptionalArg>(
                what->loc, make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name),
                node2TreeImpl(ctx, arg->default_));
            result.swap(res);
        },
        [&](parser::Shadowarg *arg) {
            unique_ptr<Expression> res = make_unique<ShadowArg>(
                what->loc, make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name));
            result.swap(res);
        },
        [&](parser::DefMethod *method) {
            unique_ptr<Expression> res = buildMethod(ctx, what->loc, method->name, method->args, method->body);
            result.swap(res);
        },
        [&](parser::DefS *method) {
            parser::Self *self = dynamic_cast<parser::Self *>(method->singleton.get());
            if (self == nullptr) {
                ctx.state.errors.error(method->loc, ErrorClass::InvalidSingletonDef,
                                       "`def EXPRESSION.method' is only supported for `def self.method'");
                unique_ptr<Expression> res = make_unique<EmptyTree>(what->loc);
                result.swap(res);
                return;
            }
            unique_ptr<MethodDef> meth = buildMethod(ctx, what->loc, method->name, method->args, method->body);
            meth->isSelf = true;
            unique_ptr<Expression> res(meth.release());
            result.swap(res);
        },
        [&](parser::Block *block) {
            vector<unique_ptr<Expression>> args;
            auto recv = node2TreeImpl(ctx, block->send);
            Error::check(dynamic_cast<Send *>(recv.get()));
            unique_ptr<Send> send(dynamic_cast<Send *>(recv.release()));
            if (auto *oargs = dynamic_cast<parser::Args *>(block->args.get())) {
                for (auto &arg : oargs->args) {
                    args.emplace_back(node2TreeImpl(ctx, arg));
                }
            } else if (block->args.get() == nullptr) {
                // do nothing
            } else {
                Error::notImplemented();
            }

            send->block = make_unique<Block>(what->loc, args, node2TreeImpl(ctx, block->body));
            unique_ptr<Expression> res(send.release());

            result.swap(res);
        },
        [&](parser::While *wl) {
            auto cond = node2TreeImpl(ctx, wl->cond);
            auto body = node2TreeImpl(ctx, wl->body);
            unique_ptr<Expression> res = make_unique<While>(what->loc, move(cond), move(body));
            result.swap(res);
        },
        [&](parser::WhilePost *wl) {
            auto cond = node2TreeImpl(ctx, wl->cond);
            auto body = node2TreeImpl(ctx, wl->body);
            unique_ptr<Expression> res = make_unique<While>(what->loc, move(cond), move(body));
            result.swap(res);
        },
        [&](parser::Until *wl) {
            auto cond = mkSend0(what->loc, node2TreeImpl(ctx, wl->cond), Names::bang());
            auto body = node2TreeImpl(ctx, wl->body);
            unique_ptr<Expression> res = make_unique<While>(what->loc, move(cond), move(body));
            result.swap(res);
        },
        [&](parser::UntilPost *wl) {
            auto cond = mkSend0(what->loc, node2TreeImpl(ctx, wl->cond), Names::bang());
            auto body = node2TreeImpl(ctx, wl->body);
            unique_ptr<Expression> res = make_unique<While>(what->loc, move(cond), move(body));
            result.swap(res);
        },
        [&](parser::Nil *wl) {
            unique_ptr<Expression> res = make_unique<Nil>(what->loc);
            result.swap(res);
        },
        [&](parser::IVar *var) {
            unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Instance, var->name);
            result.swap(res);
        },
        [&](parser::LVar *var) {
            unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, var->name);
            result.swap(res);
        },
        [&](parser::GVar *var) {
            unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Global, var->name);
            result.swap(res);
        },
        [&](parser::CVar *var) {
            unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Class, var->name);
            result.swap(res);
        },
        [&](parser::LVarLhs *var) {
            unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, var->name);
            result.swap(res);
        },
        [&](parser::GVarLhs *var) {
            unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Global, var->name);
            result.swap(res);
        },
        [&](parser::CVarLhs *var) {
            unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Class, var->name);
            result.swap(res);
        },
        [&](parser::IVarLhs *var) {
            unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Instance, var->name);
            result.swap(res);
        },
        [&](parser::Assign *asgn) {
            auto lhs = node2TreeImpl(ctx, asgn->lhs);
            auto rhs = node2TreeImpl(ctx, asgn->rhs);
            auto res = mkAssign(what->loc, lhs, rhs);
            result.swap(res);
        },
        [&](parser::Super *super) {
            vector<unique_ptr<Expression>> args;
            for (auto &stat : super->args) {
                args.emplace_back(node2TreeImpl(ctx, stat));
            };

            unique_ptr<Expression> res = make_unique<Super>(what->loc, move(args));
            result.swap(res);
        },
        [&](parser::Integer *integer) {
            unique_ptr<Expression> res = make_unique<IntLit>(what->loc, stoi(integer->val));
            result.swap(res);
        },
        [&](parser::Array *array) {
            vector<unique_ptr<Expression>> elems;
            for (auto &stat : array->elts) {
                elems.emplace_back(node2TreeImpl(ctx, stat));
            };

            unique_ptr<Expression> res = make_unique<Array>(what->loc, elems);
            result.swap(res);
        },
        [&](parser::Hash *hash) {
            vector<unique_ptr<Expression>> keys;
            vector<unique_ptr<Expression>> values;
            unique_ptr<Expression> lastMerge;

            for (auto &pairAsExpression : hash->pairs) {
                parser::Pair *pair = dynamic_cast<parser::Pair *>(pairAsExpression.get());
                if (pair != nullptr) {
                    auto key = node2TreeImpl(ctx, pair->key);
                    auto value = node2TreeImpl(ctx, pair->value);
                    keys.emplace_back(move(key));
                    values.emplace_back(move(value));
                } else {
                    parser::Kwsplat *splat = dynamic_cast<parser::Kwsplat *>(pairAsExpression.get());
                    Error::check(splat);

                    // Desguar
                    //   {a: 'a', **x, remaining}
                    // into
                    //   {a: 'a'}.merge(x).merge(remaining)
                    if (keys.size() == 0) {
                        if (lastMerge != nullptr) {
                            lastMerge = mkSend1(what->loc, lastMerge, Names::merge(), node2TreeImpl(ctx, splat->expr));
                        } else {
                            lastMerge = node2TreeImpl(ctx, splat->expr);
                        }
                    } else {
                        unique_ptr<Expression> current = make_unique<Hash>(what->loc, keys, values);
                        if (lastMerge != nullptr) {
                            lastMerge = mkSend1(what->loc, lastMerge, Names::merge(), current);
                        } else {
                            lastMerge = move(current);
                        }
                        lastMerge = mkSend1(what->loc, lastMerge, Names::merge(), node2TreeImpl(ctx, splat->expr));
                    }
                }
            };

            unique_ptr<Expression> res;
            if (keys.size() == 0) {
                if (lastMerge != nullptr) {
                    res = move(lastMerge);
                } else {
                    // Empty array
                    res = make_unique<Hash>(what->loc, keys, values);
                }
            } else {
                res = make_unique<Hash>(what->loc, keys, values);
                if (lastMerge != nullptr) {
                    res = mkSend1(what->loc, lastMerge, Names::merge(), res);
                }
            }

            result.swap(res);
        },
        [&](parser::Return *ret) {
            if (ret->exprs.size() > 1) {
                vector<unique_ptr<Expression>> elems;
                for (auto &stat : ret->exprs) {
                    elems.emplace_back(node2TreeImpl(ctx, stat));
                };
                unique_ptr<Expression> arr = make_unique<Array>(what->loc, elems);
                unique_ptr<Expression> res = make_unique<Return>(what->loc, move(arr));
                result.swap(res);
            } else if (ret->exprs.size() == 1) {
                unique_ptr<Expression> res = make_unique<Return>(what->loc, node2TreeImpl(ctx, ret->exprs[0]));
                result.swap(res);
            } else {
                unique_ptr<Expression> res = make_unique<Return>(what->loc, mkEmptyTree(what->loc));
                result.swap(res);
            }
        },
        [&](parser::Return *ret) {
            if (ret->exprs.size() > 1) {
                vector<unique_ptr<Expression>> elems;
                for (auto &stat : ret->exprs) {
                    elems.emplace_back(node2TreeImpl(ctx, stat));
                };
                unique_ptr<Expression> arr = make_unique<Array>(what->loc, elems);
                unique_ptr<Expression> res = make_unique<Return>(what->loc, move(arr));
                result.swap(res);
            } else if (ret->exprs.size() == 1) {
                unique_ptr<Expression> res = make_unique<Return>(what->loc, node2TreeImpl(ctx, ret->exprs[0]));
                result.swap(res);
            } else {
                unique_ptr<Expression> res = make_unique<Return>(what->loc, mkEmptyTree(what->loc));
                result.swap(res);
            }
        },
        [&](parser::Break *ret) {
            if (ret->exprs.size() > 1) {
                vector<unique_ptr<Expression>> elems;
                for (auto &stat : ret->exprs) {
                    elems.emplace_back(node2TreeImpl(ctx, stat));
                };
                unique_ptr<Expression> arr = make_unique<Array>(what->loc, elems);
                unique_ptr<Expression> res = make_unique<Break>(what->loc, move(arr));
                result.swap(res);
            } else if (ret->exprs.size() == 1) {
                unique_ptr<Expression> res = make_unique<Break>(what->loc, node2TreeImpl(ctx, ret->exprs[0]));
                result.swap(res);
            } else {
                unique_ptr<Expression> res = make_unique<Break>(what->loc, mkEmptyTree(what->loc));
                result.swap(res);
            }
        },
        [&](parser::Next *ret) {
            if (ret->exprs.size() > 1) {
                vector<unique_ptr<Expression>> elems;
                for (auto &stat : ret->exprs) {
                    elems.emplace_back(node2TreeImpl(ctx, stat));
                };
                unique_ptr<Expression> arr = make_unique<Array>(what->loc, elems);
                unique_ptr<Expression> res = make_unique<Next>(what->loc, move(arr));
                result.swap(res);
            } else if (ret->exprs.size() == 1) {
                unique_ptr<Expression> res = make_unique<Next>(what->loc, node2TreeImpl(ctx, ret->exprs[0]));
                result.swap(res);
            } else {
                unique_ptr<Expression> res = make_unique<Next>(what->loc, mkEmptyTree(what->loc));
                result.swap(res);
            }
        },
        [&](parser::Yield *ret) {
            if (ret->exprs.size() > 1) {
                vector<unique_ptr<Expression>> elems;
                for (auto &stat : ret->exprs) {
                    elems.emplace_back(node2TreeImpl(ctx, stat));
                };
                unique_ptr<Expression> arr = make_unique<Array>(what->loc, elems);
                unique_ptr<Expression> res = make_unique<Yield>(what->loc, move(arr));
                result.swap(res);
            } else if (ret->exprs.size() == 1) {
                unique_ptr<Expression> res = make_unique<Yield>(what->loc, node2TreeImpl(ctx, ret->exprs[0]));
                result.swap(res);
            } else {
                unique_ptr<Expression> res = make_unique<Yield>(what->loc, mkEmptyTree(what->loc));
                result.swap(res);
            }
        },
        [&](parser::If *a) {
            auto cond = node2TreeImpl(ctx, a->condition);
            auto thenp = node2TreeImpl(ctx, a->then_);
            auto elsep = node2TreeImpl(ctx, a->else_);
            auto iff = mkIf(what->loc, cond, thenp, elsep);
            result.swap(iff);
        },
        [&](parser::Masgn *masgn) {
            parser::Mlhs *lhs = dynamic_cast<parser::Mlhs *>(masgn->lhs.get());
            Error::check(lhs != nullptr);
            SymbolRef tempSym = ctx.state.newTemporary(UniqueNameKind::Desugar, Names::assignTemp(), ctx.owner);
            vector<unique_ptr<Expression>> stats;
            stats.emplace_back(mkAssign(what->loc, tempSym, node2TreeImpl(ctx, masgn->rhs)));
            int i = 0;
            for (auto &c : lhs->exprs) {
                unique_ptr<Expression> lh = node2TreeImpl(ctx, c);
                if (ast::Send *snd = dynamic_cast<ast::Send *>(lh.get())) {
                    Error::check(snd->args.size() == 0);
                    unique_ptr<Expression> getElement =
                        mkSend1(what->loc, mkIdent(what->loc, tempSym), Names::squareBrackets(),
                                make_unique<IntLit>(what->loc, i));
                    snd->args.emplace_back(move(getElement));
                    stats.emplace_back(move(lh));
                } else if (ast::Reference *ref = dynamic_cast<ast::Reference *>(lh.get())) {
                    auto access = mkSend1(what->loc, mkIdent(what->loc, tempSym), Names::squareBrackets(),
                                          make_unique<IntLit>(what->loc, i));
                    unique_ptr<Expression> assign = mkAssign(what->loc, lh, access);
                    stats.emplace_back(move(assign));
                } else if (ast::NotSupported *snd = dynamic_cast<ast::NotSupported *>(lh.get())) {
                    stats.emplace_back(move(lh));
                } else {
                    Error::notImplemented();
                }
                i++;
            }
            unique_ptr<Expression> res = mkInsSeq(what->loc, move(stats), mkIdent(what->loc, tempSym));
            result.swap(res);
        },
        [&](parser::True *t) {
            auto res = mkTrue(what->loc);
            result.swap(res);
        },
        [&](parser::False *t) {
            auto res = mkFalse(what->loc);
            result.swap(res);
        },

        [&](parser::Node *a) { result.reset(new NotSupported(what->loc, a->nodeName())); });
    Error::check(result.get());
    return result;
}
} // namespace

unique_ptr<Expression> node2Tree(Context ctx, unique_ptr<parser::Node> &what) {
    auto result = node2TreeImpl(ctx, what);
    auto verifiedResult = Verifier::run(ctx, move(result));
    return verifiedResult;
}
} // namespace desugar
} // namespace ast
} // namespace ruby_typer
