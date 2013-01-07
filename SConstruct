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

@export
def cpp11(env):
    env.Append(
        CXXFLAGS=['-std=c++0x'])

env = Environment(
    ENV=os.environ,
    CC='clang-mp-3.1',
    CXX='clang++-mp-3.1',
    CCFLAGS=['-g', '-O2'],
    LINKFLAGS=['-O2'],
    CXXFLAGS=['-Wall', '-Werror'])

env32 = env.Clone()
env32.Append(
    BUILDDIR='build32',
    CCFLAGS=['-m32'],
    LINKFLAGS=['-m32'])
SConscript('SConscript', variant_dir='build32', duplicate=0, exports={'env': env32})

env64 = env.Clone()
env64.Append(
    BUILDDIR='build64',
    CCFLAGS=['-m64'],
    LINKFLAGS=['-m64'])
SConscript('SConscript', variant_dir='build64', duplicate=0, exports={'env': env64})
