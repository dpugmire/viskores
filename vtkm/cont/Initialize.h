//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef vtk_m_cont_Initialize_h
#define vtk_m_cont_Initialize_h

#include <vtkm/cont/DeviceAdapterTag.h>
#include <vtkm/cont/vtkm_cont_export.h>
#include <vtkm/internal/ExportMacros.h>

#include <string>
#include <type_traits>
#include <vector>

namespace vtkm
{
namespace cont
{

struct InitializeResult
{
  /// The device passed into `--vtkm-device` argument. If no device was specified, then
  /// this value is set to `DeviceAdapterTagUndefined`. Note that if the user specifies
  /// "any" device, then this value can be set to `DeviceAdapterTagAny`, which is a
  /// pseudo-tag that allows any supported device.
  DeviceAdapterId Device = DeviceAdapterTagUndefined{};

  /// A usage statement for arguments parsed by VTK-m. If the calling code wants to print
  /// a usage statement documenting the options that can be provided on the command line,
  /// then this string can be added to document the options supported by VTK-m.
  std::string Usage;
};

enum class InitializeOptions
{
  /// Placeholder used when no options are enabled. This is the value used when the third argument
  /// to `vtkm::cont::Initialize` is not provided.
  None = 0x00,

  /// Issue an error if the device argument is not specified.
  RequireDevice = 0x01,

  /// If no device is specified, treat it as if the user gave `--vtkm-device=Any`. This means that
  /// `DeviceAdapterTagUndefined` will never be returned in the result.
  DefaultAnyDevice = 0x02,

  /// Add a help argument. If `-h` or `--vtkm-help` is provided, prints a usage statement. Of course,
  /// the usage statement will only print out arguments processed by VTK-m, which is why help is not
  /// given by default. Alternatively, a string with usage help is returned from `vtkm::cont::Initialize`
  /// so that the calling program can provide VTK-m's help in its own usage statement.
  AddHelp = 0x04,

  /// If an unknown option is encountered, the program terminates with an error and a usage
  /// statement is printed. If this option is not provided, any unknown options are returned
  /// in `argv`. If this option is used, it is a good idea to use AddHelp as well.
  ErrorOnBadOption = 0x08,

  /// If an extra argument is encountered, the program terminates with an error and a usage
  /// statement is printed. If this option is not provided, any unknown arguments are returned
  /// in `argv`.
  ErrorOnBadArgument = 0x10,

  /// If supplied, Initialize treats its own arguments as the only ones supported by the
  /// application and provides an error if not followed exactly. This is a convenience
  /// option that is a combination of `ErrorOnBadOption`, `ErrorOnBadArgument`, and `AddHelp`.
  Strict = ErrorOnBadOption | ErrorOnBadArgument | AddHelp
};

// Allow options to be used as a bitfield
inline InitializeOptions operator|(const InitializeOptions& lhs, const InitializeOptions& rhs)
{
  using T = std::underlying_type<InitializeOptions>::type;
  return static_cast<InitializeOptions>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
inline InitializeOptions operator&(const InitializeOptions& lhs, const InitializeOptions& rhs)
{
  using T = std::underlying_type<InitializeOptions>::type;
  return static_cast<InitializeOptions>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

/**
 * Initialize the VTKm library, parsing arguments when provided:
 * - Sets log level names when logging is configured.
 * - Sets the calling thread as the main thread for logging purposes.
 * - Sets the default log level to the argument provided to `--vtkm-log-level`.
 * - Forces usage of the device name passed to `--vtkm-device`.
 * - Prints usage when `-h` or `--vtkm-help` is passed.
 *
 * The parameterless version only sets up log level names.
 *
 * Additional options may be supplied via the @a opts argument, such as
 * requiring the `--vtkm-device` option.
 *
 * Results are available in the returned InitializeResult.
 *
 * @note This method may call exit() on parse error.
 * @{
 */
VTKM_CONT_EXPORT
VTKM_CONT
InitializeResult Initialize(int& argc,
                            char* argv[],
                            InitializeOptions opts = InitializeOptions::None);
VTKM_CONT_EXPORT
VTKM_CONT
InitializeResult Initialize();
/**@}*/
}
} // end namespace vtkm::cont


#endif // vtk_m_cont_Initialize_h
