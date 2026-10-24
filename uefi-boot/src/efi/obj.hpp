#pragma once

namespace efi
{
	template <class T>
	class obj_wrap
	{
	public:
		obj_wrap() noexcept = default;
		obj_wrap(T* native) noexcept
			:	native_(native) { }

		[[nodiscard]] T* native() const noexcept
		{
			return native_;
		}

	protected:
		T* native_;
	};
}
