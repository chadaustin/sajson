import os

def sajson(env):
    env.Append(
        CPPPATH=['#/include'])

def unittestpp(env):
    env.Append(
        CPPPATH=['#/third-party/UnitTest++/src'],
        LIBPATH=['#/build/libraries'],
        LIBS=['unittestpp'])

env = Environment(
    ENV=os.environ,
    CC='clang-mp-3.1',
    CXX='clang++-mp-3.1',
    CCFLAGS=['-g', '-O2'],
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

unittestpp_env = env32.Clone()
unittestpp_env.Append(
    CPPPATH=['#/third-party/UnitTest++/src'])
unittestpp_env.Library(
    'build/libraries/unittestpp',
    [ 'third-party/UnitTest++/src/AssertException.cpp',
      'third-party/UnitTest++/src/Checks.cpp',
      'third-party/UnitTest++/src/CurrentTest.cpp',
      'third-party/UnitTest++/src/DeferredTestReporter.cpp',
      'third-party/UnitTest++/src/DeferredTestResult.cpp',
      'third-party/UnitTest++/src/MemoryOutStream.cpp',
      'third-party/UnitTest++/src/ReportAssert.cpp',
      'third-party/UnitTest++/src/Test.cpp',
      'third-party/UnitTest++/src/TestDetails.cpp',
      'third-party/UnitTest++/src/TestList.cpp',
      'third-party/UnitTest++/src/TestReporter.cpp',
      'third-party/UnitTest++/src/TestReporterStdout.cpp',
      'third-party/UnitTest++/src/TestResults.cpp',
      'third-party/UnitTest++/src/TestRunner.cpp',
      'third-party/UnitTest++/src/TimeConstraint.cpp',
      'third-party/UnitTest++/src/XmlTestReporter.cpp',
      'third-party/UnitTest++/src/Posix/SignalTranslator.cpp',
      'third-party/UnitTest++/src/Posix/TimeHelpers.cpp' ])


test_env = env32.Clone(tools=[unittestpp])
test_env.Program('build/test', ['tests/test.cpp'])


