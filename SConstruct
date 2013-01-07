import os

def sajson(env):
    env.Append(
        CPPPATH=['#/include'])

env = Environment(
    ENV=os.environ,
    CC='clang-mp-3.1',
    CXX='clang++-mp-3.1',
    CCFLAGS=['-g', '-O2', '-fno-exceptions'],
    LINKFLAGS=['-O2'],
    CXXFLAGS=['-std=c++0x', '-Wall', '-Werror'])
sajson(env)

env32 = env.Clone()
env32.Append(
    CCFLAGS=['-m32'],
    LINKFLAGS=['-m32'])

env64 = env.Clone()
env64.Append(
    CCFLAGS=['-m64'],
    LINKFLAGS=['-m64'])

env32.Program('build/test', ['tests/test.cpp'])


