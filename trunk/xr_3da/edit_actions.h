#pragma once
#include <SDL3/SDL_keycode.h>
#include <fastdelegate.h>

enum SDL_Scancode;

namespace text_editor
{
	class callback_base
	{
	private:
		typedef		fastdelegate::FastDelegate0<void>		Callback;

	public:
		callback_base() = default;
		callback_base(Callback const& callback, const SDL_Keymod& mod);
		~callback_base();
		bool on_key_press() const;
		void AddAdditionalCallback(const callback_base* const prev_action);

		SDL_Keymod keyModCurrentCallback{};
		SDL_Keymod allKeyModsAdditionalCallback{};
	protected:
		Callback m_callback;
		xr_vector<const callback_base*> additionalCallbacks;

	}; // class callback_base

}// namespace text_editor
