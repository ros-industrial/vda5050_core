#ifndef VDA5050_TYPES__SUPPORT_TYPE_HPP_
#define VDA5050_TYPES__SUPPORT_TYPE_HPP_

namespace vda5050_types {

/// \brief Type of support for OptionalParameters.
enum class SupportType
{
  /// \brief Optional parameter is supported as specified.
  SUPPORTED,

  /// \brief Optional parameter is required for proper AGV operation.
  REQUIRED
};

}  // namespace vda5050_types

#endif  // VDA5050_TYPES__SUPPORT_TYPE_HPP_
