cmake_minimum_required(VERSION 3.30)

project(EFI_BOOTX64)

set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
enable_language(ASM_NASM)

add_library(EFI_BOOTX64_ASM STATIC
    "src/setup.asm"
)

add_executable(EFI_BOOTX64
    "src/entry.cpp"
    "src/efi/efi_fs.cpp"
    "src/efi/efi_image_services.cpp"
    "src/efi/efi_misc.cpp"
    "src/ldstdio/printf.cpp"
    "src/ldstdio/puts.cpp"
    "src/ldstdio/readkey.cpp"
    "src/ldstdio/vsnprintf.cpp"
    "src/ldstdlib/itoa.cpp"
    "src/ldstdlib/memory.cpp"
    "src/loader/acpi_check.cpp"
    "src/loader/basic_graphics.cpp"
    "src/loader/kernel_loader.cpp"
    "src/loader/paging.cpp"
    "src/loader/pci.cpp"
    "src/loader/psf_font.cpp"
    "src/loader/system_config.cpp"
)

set_target_properties(EFI_BOOTX64 PROPERTIES OUTPUT_NAME "BOOTX64.EFI")

target_include_directories(EFI_BOOTX64 PRIVATE include)
target_include_directories(EFI_BOOTX64 PRIVATE ../common/include)

target_compile_options(EFI_BOOTX64 PRIVATE
    -mabi=ms
    -Wall
	-Wextra
	-Wpedantic
	-fms-extensions
	-mno-red-zone
	-ffreestanding
	-nostdlib
    -mgeneral-regs-only
)
target_link_options(EFI_BOOTX64 PRIVATE
    -Wl,--subsystem,10
    -Wl,-e,EfiEntry
    -nolibc
    -nostdlib
)

target_link_libraries(EFI_BOOTX64 PRIVATE EFI_BOOTX64_ASM)
target_compile_features(EFI_BOOTX64 PRIVATE cxx_std_23)