#include "bootmgfw.hpp"
#include "../efi/file.hpp"
#include <pe/image.hpp>

#include "../efi/image.hpp"

#define BOOTMGFW_PATH L"\\efi\\microsoft\\boot\\bootmgfw.efi"
#define ORIG_BOOTMGFW_PATH L"\\efi\\microsoft\\boot\\bootmgfw.original.efi"

namespace
{
	[[nodiscard]] efi::status restore_file_attr(efi::file& boot_file, const efi::file& orig_file) noexcept
	{
        auto boot_info = boot_file.get_info();
        const auto orig_info = orig_file.get_info();

        if (!boot_info)
            return boot_info.error();

        if (!orig_info)
            return orig_info.error();

        (*boot_info)->create_time = (*orig_info)->create_time;
        (*boot_info)->modification_time = (*orig_info)->modification_time;

        return boot_file.set_info(*boot_info);
	}

	[[nodiscard]] efi::status restore_file(efi::file& boot_file, efi::file& orig_file) noexcept
	{
        const auto orig_buf = orig_file.read();

        if (!orig_buf)
            return orig_buf.error();

        efi::status stat = boot_file.write(orig_buf.value());

        if (!stat)
            return stat;

        stat = restore_file_attr(boot_file, orig_file);

        if (!stat)
            return stat;

        return orig_file.delete_();
	}
}

efi::status bootmgfw::restore_and_start()
{
    auto boot_file = efi::file::find(BOOTMGFW_PATH);
    auto orig_file = efi::file::find(ORIG_BOOTMGFW_PATH);

    if (!boot_file || !orig_file)
        return efi::not_found;

    const efi::status stat = restore_file(*boot_file, *orig_file);

    if (!stat)
        return stat;

    // boot_file now holds original contents
    auto boot_img = efi::image::load(*boot_file);

    if (!boot_img)
        return boot_img.error();

    return boot_img->start();
}
