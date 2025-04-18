==============================
Logging
==============================

.. index:: logging

|Viskores| features a logging system that allows status updates and timing.
|Viskores| uses the :index:`loguru` project to provide runtime logging facilities.
A sample of the log output can be found at https://gitlab.kitware.com/snippets/427.

------------------------------
Initializing Logging
------------------------------

.. index:: logging; initialization

Logging features are enabled by calling :func:`viskores::cont::Initialize` as described in :chapref:`initialization:Initialization`.
Although calling :func:`viskores::cont::Initialize` is not strictly necessary for output messages, initialization adds the following features.

* Set human-readable names for the log levels in the output.
* Allow the stderr logging level to be set at runtime by passing a ``--viskores-log-level [level]`` argument to the executable.
* Name the main thread.
* Print a preamble with details of the program's startup (arguments, etc).

:numref:`ex:InitializeLogging` in the following section provides an example of initializing with additional logging setup.

.. index:: thread name

The logging implementation is thread-safe.
When working in a multithreaded environment, each thread may be assigned a human-readable name using :func:`viskores::cont::SetLogThreadName` (which can later be retrieved with :func:`viskores::cont::GetLogThreadName`).
This name will appear in the log output so that per-thread messages can be easily tracked.

.. doxygenfunction:: viskores::cont::SetLogThreadName
.. doxygenfunction:: viskores::cont::GetLogThreadName


------------------------------
Logging Levels
------------------------------

.. index:: logging; levels

The logging in |Viskores| provides several "levels" of logging.
Logging levels are ordered by precedence.
When selecting which log message to output, a single logging level is provided.
Any logging message with that or a higher precedence is output.
For example, if warning messages are on, then error messages are also outputted because errors are a higher precedence than warnings.
Likewise, if information messages are on, then error and warning messages are also outputted.

.. commonerrors::
   All logging levels are assigned a number, and logging levels with a higher precedence actually have a smaller number.

All logging levels are listed in the :enum:`viskores::cont::LogLevel` enum.

.. doxygenenum:: viskores::cont::LogLevel

When |Viskores| outputs an entry in its log, it annotates the message with the logging level.
|Viskores| will automatically provide descriptions for all log levels described in :enum:`viskores::cont::LogLevel`.
A custom log level can be described by calling the :func:`viskores::cont::SetLogLevelName` function.
(The log name can likewise be retrieved with :func:`viskores::cont::GetLogLevelName`.)

.. doxygenfunction:: viskores::cont::SetLogLevelName
.. doxygenfunction:: viskores::cont::GetLogLevelName

.. commonerrors::
   The :func:`viskores::cont::SetLogLevelName` function must be called before :func:`viskores::cont::Initialize` to have an effect.

.. commonerrors::
   The descriptions for each log level are only set up if :func:`viskores::cont::Initialize` is called.
   If it is not, then all log levels will be represented with a numerical value.

If :func:`viskores::cont::Initialize` is called with ``argc``/``argv``, then the user can control the logging level with the ``--viskores-log-level`` command line argument.
Alternatively, you can control which logging levels are reported with the :func:`viskores::cont::SetStderrLogLevel`.

.. doxygenfunction:: viskores::cont::SetStderrLogLevel(viskores::cont::LogLevel)
.. doxygenfunction:: viskores::cont::SetStderrLogLevel(const char *verbosity)
.. doxygenfunction:: viskores::cont::GetStderrLogLevel

.. load-example:: InitializeLogging
   :file: GuideExampleInitialization.cxx
   :caption: Initializing logging.

------------------------------
Log Entries
------------------------------

Log entries are created with a collection of macros provided in :file:`viskores/cont/Logging.h`.
In addition to basic log entries, |Viskores| logging can also provide conditional logging and scope levels of logs.

Basic Log Entries
==============================

The main logging entry points are the macros :c:macro:`VISKORES_LOG_S` and :c:macro:`VISKORES_LOG_F`, which use C++ stream and printf syntax, respectively.
Both macros take a logging level as the first argument.
The remaining arguments specify the message printed to the log.
:c:macro:`VISKORES_LOG_S` takes a single argument with a C++ stream expression (so ``<<`` operators can exist in the expression).
:c:macro:`VISKORES_LOG_F` takes a C string as its second argument that has printf-style formatting codes.
The remaining arguments fulfill those codes.

.. doxygendefine:: VISKORES_LOG_S
.. doxygendefine:: VISKORES_LOG_F

.. load-example:: BasicLogging
   :file: GuideExampleInitialization.cxx
   :caption: Basic logging.

Conditional Log Entries
==============================

The macros :c:macro:`VISKORES_LOG_IF_S` :c:macro:`VISKORES_LOG_IF_F` behave similarly to :c:macro:`VISKORES_LOG_S` and :c:macro:`VISKORES_LOG_F`, respectively, except they have an extra argument that contains the condition.
If the condition is true, then the log entry is created.
If the condition is false, then the statement is ignored and nothing is recorded in the log.

.. doxygendefine:: VISKORES_LOG_IF_S
.. doxygendefine:: VISKORES_LOG_IF_F

.. load-example:: ConditionalLogging
   :file: GuideExampleInitialization.cxx
   :caption: Conditional logging.

Scoped Log Entries
==============================

The logging back end supports the concept of scopes.
Scopes allow the nesting of log messages, which allows a complex operation to report when it starts, when it ends, and what log messages happen in the middle.
Scoped log entries are also timed so you can get an idea of how long operations take.
Scoping can happen to arbitrary depths.

.. commonerrors::
   Although the timing reported in scoped log entries can give an idea of the time each operation takes, the reported time should not be considered accurate in regards to timing parallel operations.
   If a parallel algorithm is invoked inside a log scope, the program may return from that scope before the parallel algorithm is complete.
   See :chapref:`timer:Timers` for information on more accurate timers.

Scoped log entries follow the same scoping of your C++ code.
A scoped log can be created with the :c:macro:`VISKORES_LOG_SCOPE` macro.
This macro behaves similarly to :c:macro:`VISKORES_LOG_F` except that it creates a scoped log that starts when :c:macro:`VISKORES_LOG_SCOPE` and ends when the program leaves the given scope.

.. doxygendefine:: VISKORES_LOG_SCOPE

.. load-example:: ScopedLogging
   :file: GuideExampleInitialization.cxx
   :caption: Scoped logging.

It is also common, and typically good code structure, to structure scoped concepts around functions or methods.
Thus, |Viskores| provides :c:macro:`VISKORES_LOG_SCOPE_FUNCTION`.
When placed at the beginning of a function or macro, :c:macro:`VISKORES_LOG_SCOPE_FUNCTION` will automatically create a scoped log around it.

.. doxygendefine:: VISKORES_LOG_SCOPE_FUNCTION

.. load-example:: ScopedFunctionLogging
   :file: GuideExampleInitialization.cxx
   :caption: Scoped logging in a function.


------------------------------
Helper Functions
------------------------------

The :file:`viskores/cont/Logging.h` header file also contains several helper functions that provide useful functions when reporting information about the system.

.. didyouknow::
   Although provided with the logging utilities, these functions can be useful in contexts outside of the logging as well.
   These functions are available even if |Viskores| is compiled with logging off.

The :func:`viskores::cont::TypeToString` function provides run-time type information (RTTI) based type-name information.
:func:`viskores::cont::TypeToString` is a templated function for which you have to explicitly declare the type.
:func:`viskores::cont::TypeToString` returns a ``std::string`` containing a representation of the type provided.
When logging is enabled, :func:`viskores::cont::TypeToString` uses the logging back end to demangle symbol names on supported platforms.

.. doxygenfunction:: viskores::cont::TypeToString()
.. doxygenfunction:: viskores::cont::TypeToString(const T&)
.. doxygenfunction:: viskores::cont::TypeToString(const std::type_index &)
.. doxygenfunction:: viskores::cont::TypeToString(const std::type_info &)

The :func:`viskores::cont::GetHumanReadableSize` function takes a size of memory in bytes and returns a human readable string (for example "64 bytes", "1.44 MiB", "128 GiB", etc).
:func:`viskores::cont::GetSizeString` is a similar function that returns the same thing as :func:`viskores::cont::GetHumanReadableSize` followed by ``(# bytes)`` (with # replaced with the number passed to the function).
Both :func:`viskores::cont::GetHumanReadableSize` and :func:`viskores::cont::GetSizeString` take an optional second argument that is the number of digits of precision to display.
By default, they display 2 digits of precision.

.. doxygenfunction:: viskores::cont::GetHumanReadableSize(viskores::UInt64, int)
.. doxygenfunction:: viskores::cont::GetSizeString(viskores::UInt64, int)

The :func:`viskores::cont::GetStackTrace` function returns a string containing a trace of the stack, which can be helpful for debugging.
:func:`viskores::cont::GetStackTrace` takes an optional argument for the number of stack frames to skip.
Reporting the stack trace is not available on all platforms.
On platforms that are not supported, a simple string reporting that the stack trace is unavailable is returned.

.. doxygenfunction:: viskores::cont::GetStackTrace

.. load-example:: HelperLogFunctions
   :file: GuideExampleInitialization.cxx
   :caption: Helper functions provided for logging.
