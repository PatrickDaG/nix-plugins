#include <lix/config.h>

#include <lix/libexpr/primops.hh>
#include <lix/libexpr/eval-inline.hh>
#include <lix/libexpr/eval-settings.hh>

#include <lix/libstore/globals.hh>

#include <dlfcn.h>

#include "nix-plugins-config.h"

#if HAVE_BOEHMGC

#include <gc/gc.h>
#include <gc/gc_cpp.h>

#endif


using namespace nix;

struct ExtraBuiltinsSettings : Config {
    Setting<Path> extraBuiltinsFile{this,
        settings.nixConfDir + "/extra-builtins.nix",
            "extra-builtins-file",
            "The path to a nix expression defining extra expression-language level builtins."};
};

static ExtraBuiltinsSettings extraBuiltinsSettings;

static GlobalConfig::Register rp(&extraBuiltinsSettings);

static void extraBuiltins(EvalState & state, const PosIdx pos,
    Value ** _args, Value & v)
{
    static auto extraBuiltinsFile = SourcePath(CanonPath(extraBuiltinsSettings.extraBuiltinsFile.to_string()));
	state.ctx.paths.allowPath(extraBuiltinsFile.canonical().abs());

    try {
        auto fun = state.ctx.mem.allocValue();
        state.evalFile(extraBuiltinsFile, *fun);
        Value * arg;
        if (evalSettings.enableNativeCode) {
            arg = state.ctx.builtins.env.values[0];
        } else {
            auto attrs = state.ctx.buildBindings(2);

            auto sExec = state.ctx.symbols.create("exec");
            attrs.alloc(sExec).mkPrimOp(new PrimOp {
                .name = "exec",
                .arity = 1,
                .fun = prim_exec,
            });

            auto sImportNative = state.ctx.symbols.create("importNative");
            attrs.alloc(sImportNative).mkPrimOp(new PrimOp {
                .name = "importNative",
                .arity = 2,
                .fun = prim_importNative,
            });

            arg = state.ctx.mem.allocValue();
            arg->mkAttrs(attrs);
        }
        v.mkApp(fun, arg);
        state.forceValue(v, pos);
    } catch (SysError & e) {
        if (e.errNo != ENOENT)
            throw;
        v.mkNull();
    }
}

static RegisterPrimOp rp1({
    .name = "__extraBuiltins",
    .arity = 0,
    .fun = extraBuiltins,
});

static void cflags(EvalState & state, const PosIdx _pos,
    Value ** _args, Value & v)
{
    auto attrs = state.ctx.buildBindings(3);
    attrs.alloc("NIX_INCLUDE_DIRS").mkString(NIX_INCLUDE_DIRS);
    attrs.alloc("NIX_CFLAGS_OTHER").mkString(NIX_CFLAGS_OTHER);
    attrs.alloc("BOOST_INCLUDE_DIR").mkString(BOOST_INCLUDE_DIR);
    v.mkAttrs(attrs);
}

static RegisterPrimOp rp2({
    .name = "__nix-cflags",
    .arity = 0,
    .fun = cflags,
});
