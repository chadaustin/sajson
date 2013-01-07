Import('*')

unittestpp_env = env.Clone()
unittestpp_env.Append(
    CPPPATH=['#/third-party/UnitTest++/src'])
unittestpp_env.Library(
    'libraries/unittestpp',
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

test_env = env.Clone(tools=[unittestpp, sajson])
test_env.Program('test', ['tests/test.cpp'])
