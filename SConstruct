import os

def export(fn):
    Export({fn.__name__: fn})
    return fn

@export
def sajson(env):
    env.Append(
        CPPPATH=['#/include'])

@export
def unittestpp(env):
    env.Append(
        CPPPATH=['#/third-party/UnitTest++/src'],
        LIBPATH=['#/${BUILDDIR}/libraries'],
        LIBS=['unittestpp'])

def gcc(env):
    env['CC'] = 'gcc'
    env['CXX'] = 'g++'

def clang(env):
    env['CC'] = 'clang-3.5'
    env['CXX'] = 'clang++-3.5'

def dbg(env):
    env.Append(CCFLAGS=['-g'])

def opt(env):
    env.Append(
        CCFLAGS=['-O2'],
        LINKFLAGS=['-O2', '-s'])

def m32(env):
    env.Append(
        CCFLAGS=['-m32'],
        LINKFLAGS=['-m32'])

def m64(env):
    env.Append(
        CCFLAGS=['-m64'],
        LINKFLAGS=['-m64'])

env = Environment(
    ENV=os.environ,
    CXXFLAGS=['-std=c++11', '-Wall', '-Werror', '-Wno-unused-private-field'])

builds = [
    ('gcc-32-opt', [gcc, m32, opt]),
    ('gcc-32-dbg', [gcc, m32, dbg]),
    ('gcc-64-opt', [gcc, m64, opt]),
    ('gcc-64-dbg', [gcc, m64, dbg]),
    #('clang-32-opt', [clang, m32, opt]),
    #('clang-32-dbg', [clang, m32, dbg]),
    ('clang-64-opt', [clang, m64, opt]),
    ('clang-64-dbg', [clang, m64, dbg]),
]

for name, tools in builds:
    e = env.Clone(tools=tools)
    e.Append(BUILDDIR=os.path.join('build', name))
    e.SConscript('SConscript', variant_dir='$BUILDDIR', duplicate=0, exports={'env': e})
