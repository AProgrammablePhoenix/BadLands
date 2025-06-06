#pragma once

#include <efi/efi_datatypes.h>

#include <loader/system_config.hpp>

namespace Loader {
    EFI_PHYSICAL_ADDRESS LocatePCI(const EFI_SYSTEM_CONFIGURATION& sysconfig);
}
