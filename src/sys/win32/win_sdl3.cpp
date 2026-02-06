/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#include "win_local.h"
#include "../../framework/Common.h"
#include "../../framework/Session.h"
#include "../../renderer/tr_local.h"

#include <SDL3/SDL.h>

#include <cstdint>

// WGL_ARB_extensions_string
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;

// WGL_EXT_swap_interval
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

// WGL_ARB_pixel_format
PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC wglGetPixelFormatAttribfvARB;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;

// WGL_ARB_pbuffer
PFNWGLCREATEPBUFFERARBPROC wglCreatePbufferARB;
PFNWGLGETPBUFFERDCARBPROC wglGetPbufferDCARB;
PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB;
PFNWGLDESTROYPBUFFERARBPROC wglDestroyPbufferARB;
PFNWGLQUERYPBUFFERARBPROC wglQueryPbufferARB;

// WGL_ARB_render_texture
PFNWGLBINDTEXIMAGEARBPROC wglBindTexImageARB;
PFNWGLRELEASETEXIMAGEARBPROC wglReleaseTexImageARB;
PFNWGLSETPBUFFERATTRIBARBPROC wglSetPbufferAttribARB;

static SDL_Window *s_sdlWindow = NULL;
static SDL_GLContext s_sdlContext = NULL;
static bool s_sdlVideoActive = false;
static bool s_sdlTextInputActive = false;

static const unsigned char s_scantokey[128] = {
	0,          27,    '1',       '2',        '3',    '4',         '5',      '6',
	'7',        '8',    '9',       '0',        '-',    '=',          K_BACKSPACE, 9,
	'q',        'w',    'e',       'r',        't',    'y',         'u',      'i',
	'o',        'p',    '[',       ']',        K_ENTER,K_CTRL,      'a',      's',
	'd',        'f',    'g',       'h',        'j',    'k',         'l',      ';',
	'\'',       '`',    K_SHIFT,   '\\',       'z',    'x',         'c',      'v',
	'b',        'n',    'm',       ',',        '.',    '/',         K_SHIFT,  K_KP_STAR,
	K_ALT,      ' ',    K_CAPSLOCK,K_F1,       K_F2,   K_F3,        K_F4,     K_F5,
	K_F6,       K_F7,   K_F8,      K_F9,       K_F10,  K_PAUSE,     K_SCROLL, K_HOME,
	K_UPARROW,  K_PGUP, K_KP_MINUS,K_LEFTARROW,K_KP_5, K_RIGHTARROW,K_KP_PLUS,K_END,
	K_DOWNARROW,K_PGDN, K_INS,     K_DEL,      0,      0,           0,        K_F11,
	K_F12,      0,      0,         K_LWIN,     K_RWIN, K_MENU,      0,        0,
	0,          0,      0,         0,          0,      0,           0,        0,
	0,          0,      0,         0,          0,      0,           0,        0,
	0,          0,      0,         0,          0,      0,           0,        0,
	0,          0,      0,         0,          0,      0,           0,        0
};

static const unsigned char s_scantoshift[128] = {
	0,           27,    '!',       '@',        '#',    '$',         '%',      '^',
	'&',        '*',    '(',       ')',        '_',    '+',         K_BACKSPACE, 9,
	'Q',        'W',    'E',       'R',        'T',    'Y',         'U',      'I',
	'O',        'P',    '{',       '}',        K_ENTER,K_CTRL,      'A',      'S',
	'D',        'F',    'G',       'H',        'J',    'K',         'L',      ':',
	'|',        '~',    K_SHIFT,   '\\',       'Z',    'X',         'C',      'V',
	'B',        'N',    'M',       '<',        '>',    '?',         K_SHIFT,  K_KP_STAR,
	K_ALT,      ' ',    K_CAPSLOCK,K_F1,       K_F2,   K_F3,        K_F4,     K_F5,
	K_F6,       K_F7,   K_F8,      K_F9,       K_F10,  K_PAUSE,     K_SCROLL, K_HOME,
	K_UPARROW,  K_PGUP, K_KP_MINUS,K_LEFTARROW,K_KP_5, K_RIGHTARROW,K_KP_PLUS,K_END,
	K_DOWNARROW,K_PGDN, K_INS,     K_DEL,      0,      0,           0,        K_F11,
	K_F12,      0,      0,         K_LWIN,     K_RWIN, K_MENU,      0,        0,
	0,          0,      0,         0,          0,      0,           0,        0,
	0,          0,      0,         0,          0,      0,           0,        0,
	0,          0,      0,         0,          0,      0,           0,        0,
	0,          0,      0,         0,          0,      0,           0,        0
};

static unsigned char s_rightAltKey = K_ALT;

typedef struct {
	int		key;
	bool	down;
	int		time;
} sdlKeyboardEvent_t;

typedef struct {
	int		action;
	int		value;
	int		time;
} sdlMouseEvent_t;

static const int SDL3_INPUT_QUEUE_SIZE = 512;
static const int SDL3_INPUT_QUEUE_MASK = SDL3_INPUT_QUEUE_SIZE - 1;

static_assert((SDL3_INPUT_QUEUE_SIZE & SDL3_INPUT_QUEUE_MASK) == 0, "input queue size must be power-of-two");

static sdlKeyboardEvent_t s_keyboardQueue[SDL3_INPUT_QUEUE_SIZE];
static sdlMouseEvent_t s_mouseQueue[SDL3_INPUT_QUEUE_SIZE];
static int s_keyboardHead = 0;
static int s_keyboardTail = 0;
static int s_mouseHead = 0;
static int s_mouseTail = 0;

static sdlKeyboardEvent_t s_polledKeyboard[SDL3_INPUT_QUEUE_SIZE];
static sdlMouseEvent_t s_polledMouse[SDL3_INPUT_QUEUE_SIZE];
static int s_polledKeyboardCount = 0;
static int s_polledMouseCount = 0;

void* GLimp_ExtensionPointer(const char* name);

bool QGL_Init(const char *dllname);
void QGL_Shutdown(void);

static int SDL3_EventMilliseconds(Uint64 timestampNs) {
	static const Uint64 SDL3_MAX_EVENT_MS = 0x7fffffffULL;
	if (timestampNs == 0) {
		return Sys_Milliseconds();
	}
	Uint64 timeMs = SDL_NS_TO_MS(timestampNs);
	if (timeMs > SDL3_MAX_EVENT_MS) {
		return static_cast<int>(SDL3_MAX_EVENT_MS);
	}
	return static_cast<int>(timeMs);
}

static void SDL3_ClearInputQueues(void) {
	Sys_EnterCriticalSection(CRITICAL_SECTION_ONE);
	s_keyboardHead = s_keyboardTail = 0;
	s_mouseHead = s_mouseTail = 0;
	s_polledKeyboardCount = 0;
	s_polledMouseCount = 0;
	Sys_LeaveCriticalSection(CRITICAL_SECTION_ONE);
}

static void SDL3_QueueKeyboardInput(int key, bool down, int time) {
	const int next = (s_keyboardHead + 1) & SDL3_INPUT_QUEUE_MASK;

	Sys_EnterCriticalSection(CRITICAL_SECTION_ONE);
	if (next == s_keyboardTail) {
		s_keyboardTail = (s_keyboardTail + 1) & SDL3_INPUT_QUEUE_MASK;
	}
	s_keyboardQueue[s_keyboardHead].key = key;
	s_keyboardQueue[s_keyboardHead].down = down;
	s_keyboardQueue[s_keyboardHead].time = time;
	s_keyboardHead = next;
	Sys_LeaveCriticalSection(CRITICAL_SECTION_ONE);
}

static void SDL3_QueueMouseInput(int action, int value, int time) {
	const int next = (s_mouseHead + 1) & SDL3_INPUT_QUEUE_MASK;

	Sys_EnterCriticalSection(CRITICAL_SECTION_ONE);
	if (next == s_mouseTail) {
		s_mouseTail = (s_mouseTail + 1) & SDL3_INPUT_QUEUE_MASK;
	}
	s_mouseQueue[s_mouseHead].action = action;
	s_mouseQueue[s_mouseHead].value = value;
	s_mouseQueue[s_mouseHead].time = time;
	s_mouseHead = next;
	Sys_LeaveCriticalSection(CRITICAL_SECTION_ONE);
}

static int SDL3_MapScancode(SDL_Scancode scancode) {
	switch (scancode) {
		case SDL_SCANCODE_ESCAPE: return K_ESCAPE;
		case SDL_SCANCODE_RETURN: return K_ENTER;
		case SDL_SCANCODE_TAB: return K_TAB;
		case SDL_SCANCODE_BACKSPACE: return K_BACKSPACE;
		case SDL_SCANCODE_SPACE: return K_SPACE;

		case SDL_SCANCODE_LCTRL:
		case SDL_SCANCODE_RCTRL: return K_CTRL;
		case SDL_SCANCODE_LSHIFT:
		case SDL_SCANCODE_RSHIFT: return K_SHIFT;
		case SDL_SCANCODE_LALT: return K_ALT;
		case SDL_SCANCODE_RALT: return s_rightAltKey;
		case SDL_SCANCODE_LGUI: return K_LWIN;
		case SDL_SCANCODE_RGUI: return K_RWIN;
		case SDL_SCANCODE_APPLICATION: return K_MENU;
		case SDL_SCANCODE_MENU: return K_MENU;
		case SDL_SCANCODE_POWER: return K_POWER;

		case SDL_SCANCODE_CAPSLOCK: return K_CAPSLOCK;
		case SDL_SCANCODE_SCROLLLOCK: return K_SCROLL;
		case SDL_SCANCODE_PAUSE: return K_PAUSE;
		case SDL_SCANCODE_PRINTSCREEN: return K_PRINT_SCR;

		case SDL_SCANCODE_UP: return K_UPARROW;
		case SDL_SCANCODE_DOWN: return K_DOWNARROW;
		case SDL_SCANCODE_LEFT: return K_LEFTARROW;
		case SDL_SCANCODE_RIGHT: return K_RIGHTARROW;
		case SDL_SCANCODE_INSERT: return K_INS;
		case SDL_SCANCODE_DELETE: return K_DEL;
		case SDL_SCANCODE_HOME: return K_HOME;
		case SDL_SCANCODE_END: return K_END;
		case SDL_SCANCODE_PAGEUP: return K_PGUP;
		case SDL_SCANCODE_PAGEDOWN: return K_PGDN;

		case SDL_SCANCODE_F1: return K_F1;
		case SDL_SCANCODE_F2: return K_F2;
		case SDL_SCANCODE_F3: return K_F3;
		case SDL_SCANCODE_F4: return K_F4;
		case SDL_SCANCODE_F5: return K_F5;
		case SDL_SCANCODE_F6: return K_F6;
		case SDL_SCANCODE_F7: return K_F7;
		case SDL_SCANCODE_F8: return K_F8;
		case SDL_SCANCODE_F9: return K_F9;
		case SDL_SCANCODE_F10: return K_F10;
		case SDL_SCANCODE_F11: return K_F11;
		case SDL_SCANCODE_F12: return K_F12;
		case SDL_SCANCODE_F13: return K_F13;
		case SDL_SCANCODE_F14: return K_F14;
		case SDL_SCANCODE_F15: return K_F15;

		case SDL_SCANCODE_NUMLOCKCLEAR: return K_KP_NUMLOCK;
		case SDL_SCANCODE_KP_DIVIDE: return K_KP_SLASH;
		case SDL_SCANCODE_KP_MULTIPLY: return K_KP_STAR;
		case SDL_SCANCODE_KP_MINUS: return K_KP_MINUS;
		case SDL_SCANCODE_KP_PLUS: return K_KP_PLUS;
		case SDL_SCANCODE_KP_ENTER: return K_KP_ENTER;
		case SDL_SCANCODE_RETURN2: return K_KP_ENTER;
		case SDL_SCANCODE_KP_1: return K_KP_END;
		case SDL_SCANCODE_KP_2: return K_KP_DOWNARROW;
		case SDL_SCANCODE_KP_3: return K_KP_PGDN;
		case SDL_SCANCODE_KP_4: return K_KP_LEFTARROW;
		case SDL_SCANCODE_KP_5: return K_KP_5;
		case SDL_SCANCODE_KP_6: return K_KP_RIGHTARROW;
		case SDL_SCANCODE_KP_7: return K_KP_HOME;
		case SDL_SCANCODE_KP_8: return K_KP_UPARROW;
		case SDL_SCANCODE_KP_9: return K_KP_PGUP;
		case SDL_SCANCODE_KP_0: return K_KP_INS;
		case SDL_SCANCODE_KP_PERIOD: return K_KP_DEL;
		case SDL_SCANCODE_KP_COMMA: return ',';
		case SDL_SCANCODE_KP_EQUALS: return K_KP_EQUALS;
		case SDL_SCANCODE_KP_EQUALSAS400: return K_KP_EQUALS;

		case SDL_SCANCODE_A: return 'a';
		case SDL_SCANCODE_B: return 'b';
		case SDL_SCANCODE_C: return 'c';
		case SDL_SCANCODE_D: return 'd';
		case SDL_SCANCODE_E: return 'e';
		case SDL_SCANCODE_F: return 'f';
		case SDL_SCANCODE_G: return 'g';
		case SDL_SCANCODE_H: return 'h';
		case SDL_SCANCODE_I: return 'i';
		case SDL_SCANCODE_J: return 'j';
		case SDL_SCANCODE_K: return 'k';
		case SDL_SCANCODE_L: return 'l';
		case SDL_SCANCODE_M: return 'm';
		case SDL_SCANCODE_N: return 'n';
		case SDL_SCANCODE_O: return 'o';
		case SDL_SCANCODE_P: return 'p';
		case SDL_SCANCODE_Q: return 'q';
		case SDL_SCANCODE_R: return 'r';
		case SDL_SCANCODE_S: return 's';
		case SDL_SCANCODE_T: return 't';
		case SDL_SCANCODE_U: return 'u';
		case SDL_SCANCODE_V: return 'v';
		case SDL_SCANCODE_W: return 'w';
		case SDL_SCANCODE_X: return 'x';
		case SDL_SCANCODE_Y: return 'y';
		case SDL_SCANCODE_Z: return 'z';

		case SDL_SCANCODE_1: return '1';
		case SDL_SCANCODE_2: return '2';
		case SDL_SCANCODE_3: return '3';
		case SDL_SCANCODE_4: return '4';
		case SDL_SCANCODE_5: return '5';
		case SDL_SCANCODE_6: return '6';
		case SDL_SCANCODE_7: return '7';
		case SDL_SCANCODE_8: return '8';
		case SDL_SCANCODE_9: return '9';
		case SDL_SCANCODE_0: return '0';

		case SDL_SCANCODE_MINUS: return '-';
		case SDL_SCANCODE_EQUALS: return '=';
		case SDL_SCANCODE_LEFTBRACKET: return '[';
		case SDL_SCANCODE_RIGHTBRACKET: return ']';
		case SDL_SCANCODE_BACKSLASH:
		case SDL_SCANCODE_NONUSHASH:
		case SDL_SCANCODE_NONUSBACKSLASH: return '\\';
		case SDL_SCANCODE_SEMICOLON: return ';';
		case SDL_SCANCODE_APOSTROPHE: return '\'';
		case SDL_SCANCODE_GRAVE: return '`';
		case SDL_SCANCODE_COMMA: return ',';
		case SDL_SCANCODE_PERIOD: return '.';
		case SDL_SCANCODE_SLASH: return '/';
		default: return 0;
	}
}

static int SDL3_MapControlChar(int key, bool down, SDL_Keymod modState) {
	if (!down) {
		return 0;
	}

	// Keep SDL text handling aligned with legacy WM_CHAR behavior for control keys.
	switch (key) {
		case K_BACKSPACE: return '\b';
		case K_TAB: return '\t';
		case K_ENTER:
		case K_KP_ENTER: return '\r';
		default:
			break;
	}

	if ((modState & SDL_KMOD_CTRL) != 0 && (modState & SDL_KMOD_ALT) == 0) {
		if (key >= 'a' && key <= 'z') {
			return (key - 'a') + 1;
		}
	}

	return 0;
}

static int SDL3_MapMouseButton(Uint8 button) {
	switch (button) {
		case SDL_BUTTON_LEFT: return K_MOUSE1;
		case SDL_BUTTON_RIGHT: return K_MOUSE2;
		case SDL_BUTTON_MIDDLE: return K_MOUSE3;
		case SDL_BUTTON_X1: return K_MOUSE4;
		case SDL_BUTTON_X2: return K_MOUSE5;
		default: return 0;
	}
}

static void SDL3_RefreshWindowPlacement(void) {
	if (!s_sdlWindow) {
		return;
	}

	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	int pixelWidth = 0;
	int pixelHeight = 0;

	if (SDL_GetWindowPosition(s_sdlWindow, &x, &y) && !win32.cdsFullscreen) {
		win32.win_xpos.SetInteger(x);
		win32.win_ypos.SetInteger(y);
		win32.win_xpos.ClearModified();
		win32.win_ypos.ClearModified();
	}

	if (SDL_GetWindowSize(s_sdlWindow, &width, &height) && !win32.cdsFullscreen) {
		(void)width;
		(void)height;
	}

	if (!SDL_GetWindowSizeInPixels(s_sdlWindow, &pixelWidth, &pixelHeight)) {
		pixelWidth = width;
		pixelHeight = height;
	}

	if (pixelWidth > 0 && pixelHeight > 0) {
		glConfig.vidWidth = pixelWidth;
		glConfig.vidHeight = pixelHeight;
	}
}

static bool SDL3_ApplyScreenParms(glimpParms_t parms) {
	if (!s_sdlWindow) {
		return false;
	}

	if (parms.fullScreen) {
		SDL_DisplayID display = SDL_GetDisplayForWindow(s_sdlWindow);
		if (display == 0) {
			display = SDL_GetPrimaryDisplay();
		}

		SDL_DisplayMode mode;
		memset(&mode, 0, sizeof(mode));
		mode.displayID = display;
		mode.w = parms.width;
		mode.h = parms.height;
		mode.refresh_rate = parms.displayHz > 0 ? static_cast<float>(parms.displayHz) : 0.0f;

		if (!SDL_SetWindowFullscreenMode(s_sdlWindow, &mode)) {
			common->Printf("SDL3: failed to set fullscreen mode: %s\n", SDL_GetError());
			(void)SDL_SetWindowFullscreenMode(s_sdlWindow, NULL);
		}

		if (!SDL_SetWindowFullscreen(s_sdlWindow, true)) {
			common->Printf("SDL3: failed to enter fullscreen: %s\n", SDL_GetError());
			return false;
		}
	} else {
		if (!SDL_SetWindowFullscreen(s_sdlWindow, false)) {
			common->Printf("SDL3: failed to leave fullscreen: %s\n", SDL_GetError());
			return false;
		}
		(void)SDL_SetWindowFullscreenMode(s_sdlWindow, NULL);

		if (!SDL_SetWindowSize(s_sdlWindow, parms.width, parms.height)) {
			common->Printf("SDL3: failed to resize window: %s\n", SDL_GetError());
		}

		if (!SDL_SetWindowPosition(s_sdlWindow, win32.win_xpos.GetInteger(), win32.win_ypos.GetInteger())) {
			common->Printf("SDL3: failed to move window: %s\n", SDL_GetError());
		}
	}

	win32.cdsFullscreen = parms.fullScreen;
	glConfig.isFullscreen = parms.fullScreen;
	SDL3_RefreshWindowPlacement();

	return true;
}

static void SDL3_LoadWGLExtensions(void) {
	wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)GLimp_ExtensionPointer("wglGetExtensionsStringARB");
	if (wglGetExtensionsStringARB && win32.hDC) {
		glConfig.wgl_extensions_string = (const char *)wglGetExtensionsStringARB(win32.hDC);
	} else {
		glConfig.wgl_extensions_string = "";
	}

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)GLimp_ExtensionPointer("wglSwapIntervalEXT");
	r_swapInterval.SetModified();

	wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)GLimp_ExtensionPointer("wglGetPixelFormatAttribivARB");
	wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)GLimp_ExtensionPointer("wglGetPixelFormatAttribfvARB");
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)GLimp_ExtensionPointer("wglChoosePixelFormatARB");

	wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)GLimp_ExtensionPointer("wglCreatePbufferARB");
	wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)GLimp_ExtensionPointer("wglGetPbufferDCARB");
	wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)GLimp_ExtensionPointer("wglReleasePbufferDCARB");
	wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)GLimp_ExtensionPointer("wglDestroyPbufferARB");
	wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC)GLimp_ExtensionPointer("wglQueryPbufferARB");

	wglBindTexImageARB = (PFNWGLBINDTEXIMAGEARBPROC)GLimp_ExtensionPointer("wglBindTexImageARB");
	wglReleaseTexImageARB = (PFNWGLRELEASETEXIMAGEARBPROC)GLimp_ExtensionPointer("wglReleaseTexImageARB");
	wglSetPbufferAttribARB = (PFNWGLSETPBUFFERATTRIBARBPROC)GLimp_ExtensionPointer("wglSetPbufferAttribARB");
}

static void SDL3_InitDesktopMode(void) {
	const SDL_DisplayID display = SDL_GetPrimaryDisplay();
	const SDL_DisplayMode *desktopMode = SDL_GetDesktopDisplayMode(display);
	if (desktopMode) {
		win32.desktopBitsPixel = SDL_BITSPERPIXEL(desktopMode->format);
		win32.desktopWidth = desktopMode->w;
		win32.desktopHeight = desktopMode->h;
	} else {
		win32.desktopBitsPixel = 32;
		win32.desktopWidth = 1920;
		win32.desktopHeight = 1080;
	}
}

static void SDL3_UpdateNativeWindowHandles(void) {
	win32.hWnd = NULL;
	win32.hDC = NULL;
	win32.hGLRC = NULL;

	if (!s_sdlWindow) {
		return;
	}

	SDL_PropertiesID props = SDL_GetWindowProperties(s_sdlWindow);
	if (props == 0) {
		return;
	}

	win32.hWnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	win32.hDC = (HDC)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HDC_POINTER, NULL);
}

static void SDL3_HandleWindowEvent(const SDL_WindowEvent &event, int eventTime) {
	switch (event.type) {
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "quit\n");
			break;

		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			win32.activeApp = true;
			idKeyInput::ClearStates();
			com_editorActive = false;
			Sys_GrabMouseCursor(true);
			if (session != NULL) {
				session->SetPlayingSoundWorld();
			}
			break;

		case SDL_EVENT_WINDOW_FOCUS_LOST:
			win32.activeApp = false;
			win32.movingWindow = false;
			if (session != NULL) {
				session->SetPlayingSoundWorld();
			}
			break;

		case SDL_EVENT_WINDOW_MOVED:
			if (!win32.cdsFullscreen) {
				win32.win_xpos.SetInteger(event.data1);
				win32.win_ypos.SetInteger(event.data2);
				win32.win_xpos.ClearModified();
				win32.win_ypos.ClearModified();
			}
			break;

		case SDL_EVENT_WINDOW_MINIMIZED:
			win32.activeApp = false;
			break;

		case SDL_EVENT_WINDOW_RESTORED:
			win32.activeApp = true;
			break;

		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			if (event.data1 > 0 && event.data2 > 0) {
				glConfig.vidWidth = event.data1;
				glConfig.vidHeight = event.data2;
			}
			break;

		default:
			break;
	}

	(void)eventTime;
}

bool Sys_SDL_PumpEvents(void) {
	if (!s_sdlVideoActive || !s_sdlWindow) {
		return false;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		const int eventTime = SDL3_EventMilliseconds(event.common.timestamp);

		if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST) {
			SDL3_HandleWindowEvent(event.window, eventTime);
			continue;
		}

		switch (event.type) {
			case SDL_EVENT_QUIT:
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "quit\n");
				break;

			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP: {
				const bool down = (event.type == SDL_EVENT_KEY_DOWN) && event.key.down;
				const int key = SDL3_MapScancode(event.key.scancode);

				if (down && !event.key.repeat && key == K_ENTER && (event.key.mod & SDL_KMOD_ALT) != 0) {
					cvarSystem->SetCVarBool("r_fullscreen", !renderSystem->IsFullScreen());
					cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "vid_restart\n");
					break;
				}

				if (key != 0) {
					// Keep parity with the old Win32 path: these are queued from keyboard polling.
					if (key != K_PRINT_SCR && key != K_CTRL && key != K_ALT && key != K_RIGHT_ALT) {
						Sys_QueEvent(eventTime, SE_KEY, key, down, 0, NULL);
					}

					const int controlChar = SDL3_MapControlChar(key, down, event.key.mod);
					if (controlChar != 0) {
						Sys_QueEvent(eventTime, SE_CHAR, controlChar, 0, 0, NULL);
					}

					SDL3_QueueKeyboardInput(key, down, eventTime);
				}
				break;
			}

			case SDL_EVENT_TEXT_INPUT: {
				const char *text = event.text.text;
				for (int i = 0; text[i] != '\0'; ++i) {
					const unsigned char ch = static_cast<unsigned char>(text[i]);
					Sys_QueEvent(eventTime, SE_CHAR, ch, 0, 0, NULL);
				}
				break;
			}

			case SDL_EVENT_MOUSE_MOTION:
				if (win32.mouseGrabbed) {
					const int dx = static_cast<int>(event.motion.xrel);
					const int dy = static_cast<int>(event.motion.yrel);

					if (dx != 0 || dy != 0) {
						Sys_QueEvent(eventTime, SE_MOUSE, dx, dy, 0, NULL);
						if (dx != 0) {
							SDL3_QueueMouseInput(M_DELTAX, dx, eventTime);
						}
						if (dy != 0) {
							SDL3_QueueMouseInput(M_DELTAY, dy, eventTime);
						}
					}
				}
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
				if (win32.mouseGrabbed) {
					const int key = SDL3_MapMouseButton(event.button.button);
					if (key != 0) {
						const bool down = event.button.down;
						Sys_QueEvent(eventTime, SE_KEY, key, down, 0, NULL);
						SDL3_QueueMouseInput(M_ACTION1 + (key - K_MOUSE1), down ? 1 : 0, eventTime);
					}
				}
				break;

			case SDL_EVENT_MOUSE_WHEEL: {
				float deltaY = event.wheel.y;
				if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
					deltaY = -deltaY;
				}

				int wheelSteps = static_cast<int>(deltaY);
				if (wheelSteps != 0) {
					const int wheelKey = wheelSteps < 0 ? K_MWHEELDOWN : K_MWHEELUP;
					const int absSteps = abs(wheelSteps);
					for (int i = 0; i < absSteps; ++i) {
						Sys_QueEvent(eventTime, SE_KEY, wheelKey, true, 0, NULL);
						Sys_QueEvent(eventTime, SE_KEY, wheelKey, false, 0, NULL);
					}
					SDL3_QueueMouseInput(M_DELTAZ, wheelSteps, eventTime);
				}
				break;
			}

			default:
				break;
		}
	}

	return true;
}

/*
====================
MapKey

Map from windows to Doom keynums
====================
*/
int MapKey(int key) {
	int result;
	int modified;
	bool isExtended;

	modified = (key >> 16) & 255;

	if (modified > 127) {
		return 0;
	}

	isExtended = ((key & (1 << 24)) != 0);

	if (isExtended) {
		switch (modified) {
			case 0x35:
				return K_KP_SLASH;
			default:
				break;
		}
	}

	const unsigned char *scanToKey = Sys_GetScanTable();
	result = scanToKey[modified];

	if (isExtended) {
		switch (result) {
			case K_PAUSE:
				return K_KP_NUMLOCK;
			case 0x0D:
				return K_KP_ENTER;
			case 0x2F:
				return K_KP_SLASH;
			case 0xAF:
				return K_KP_PLUS;
			case K_KP_STAR:
				return K_PRINT_SCR;
			case K_ALT:
				return K_RIGHT_ALT;
			default:
				break;
		}
	} else {
		switch (result) {
			case K_HOME:
				return K_KP_HOME;
			case K_UPARROW:
				return K_KP_UPARROW;
			case K_PGUP:
				return K_KP_PGUP;
			case K_LEFTARROW:
				return K_KP_LEFTARROW;
			case K_RIGHTARROW:
				return K_KP_RIGHTARROW;
			case K_END:
				return K_KP_END;
			case K_DOWNARROW:
				return K_KP_DOWNARROW;
			case K_PGDN:
				return K_KP_PGDN;
			case K_INS:
				return K_KP_INS;
			case K_DEL:
				return K_KP_DEL;
			default:
				break;
		}
	}

	return result;
}

void Sys_InitScanTable(void) {
	idStr lang = cvarSystem->GetCVarString("sys_lang");
	if (lang.Length() == 0) {
		lang = "english";
	}

	// Keep legacy Win32 behavior: English maps RightAlt to K_ALT for bind compatibility.
	s_rightAltKey = K_ALT;
	if (lang.Icmp("spanish") == 0 ||
		lang.Icmp("french") == 0 ||
		lang.Icmp("german") == 0 ||
		lang.Icmp("italian") == 0) {
		s_rightAltKey = K_RIGHT_ALT;
	}
}

const unsigned char *Sys_GetScanTable(void) {
	return s_scantokey;
}

unsigned char Sys_GetConsoleKey(bool shifted) {
	return shifted ? s_scantoshift[41] : s_scantokey[41];
}

unsigned char Sys_MapCharForKey(int key) {
	return static_cast<unsigned char>(key);
}

void IN_ActivateMouse(void) {
	if (!s_sdlWindow || !win32.in_mouse.GetBool() || win32.mouseGrabbed) {
		return;
	}

	if (!SDL_SetWindowMouseGrab(s_sdlWindow, true)) {
		common->Printf("SDL3: failed to grab mouse: %s\n", SDL_GetError());
		return;
	}
	if (!SDL_SetWindowRelativeMouseMode(s_sdlWindow, true)) {
		common->Printf("SDL3: failed to enable relative mouse mode: %s\n", SDL_GetError());
		(void)SDL_SetWindowMouseGrab(s_sdlWindow, false);
		return;
	}

	(void)SDL_HideCursor();
	(void)SDL_GetRelativeMouseState(NULL, NULL);
	win32.mouseGrabbed = true;
}

void IN_DeactivateMouse(void) {
	if (!s_sdlWindow || !win32.mouseGrabbed) {
		return;
	}

	(void)SDL_SetWindowRelativeMouseMode(s_sdlWindow, false);
	(void)SDL_SetWindowMouseGrab(s_sdlWindow, false);
	(void)SDL_ShowCursor();
	win32.mouseGrabbed = false;
}

void IN_DeactivateMouseIfWindowed(void) {
	if (!win32.cdsFullscreen) {
		IN_DeactivateMouse();
	}
}

void IN_Frame(void) {
	bool shouldGrab = true;

	if (!win32.in_mouse.GetBool()) {
		shouldGrab = false;
	}

	if (!win32.cdsFullscreen) {
		if (win32.mouseReleased) {
			shouldGrab = false;
		}
		if (win32.movingWindow) {
			shouldGrab = false;
		}
		if (!win32.activeApp) {
			shouldGrab = false;
		}
	}

	if (shouldGrab != win32.mouseGrabbed) {
		if (win32.mouseGrabbed) {
			IN_DeactivateMouse();
		} else {
			IN_ActivateMouse();
		}
	}
}

void Sys_GrabMouseCursor(bool grabIt) {
#ifndef ID_DEDICATED
	win32.mouseReleased = !grabIt;
	if (!grabIt) {
		IN_Frame();
	}
#else
	(void)grabIt;
#endif
}

void Sys_InitInput(void) {
	common->Printf("\n------- Input Initialization -------\n");
	win32.activeApp = true;
	win32.mouseReleased = false;
	win32.movingWindow = false;
	win32.mouseGrabbed = false;

	if (s_sdlWindow && !s_sdlTextInputActive) {
		if (SDL_StartTextInput(s_sdlWindow)) {
			s_sdlTextInputActive = true;
		} else {
			common->Printf("SDL3: text input could not be enabled: %s\n", SDL_GetError());
		}
	}

	if (win32.in_mouse.GetBool()) {
		Sys_GrabMouseCursor(false);
		common->Printf("mouse: SDL3 initialized.\n");
	} else {
		common->Printf("Mouse control not active.\n");
	}

	win32.in_mouse.ClearModified();
	SDL3_ClearInputQueues();
	common->Printf("------------------------------------\n");
}

void Sys_ShutdownInput(void) {
	IN_DeactivateMouse();

	if (s_sdlWindow && s_sdlTextInputActive) {
		(void)SDL_StopTextInput(s_sdlWindow);
		s_sdlTextInputActive = false;
	}

	SDL3_ClearInputQueues();
}

int Sys_PollKeyboardInputEvents(void) {
	Sys_EnterCriticalSection(CRITICAL_SECTION_ONE);

	s_polledKeyboardCount = 0;
	while (s_keyboardTail != s_keyboardHead && s_polledKeyboardCount < SDL3_INPUT_QUEUE_SIZE) {
		s_polledKeyboard[s_polledKeyboardCount] = s_keyboardQueue[s_keyboardTail];
		s_polledKeyboardCount++;
		s_keyboardTail = (s_keyboardTail + 1) & SDL3_INPUT_QUEUE_MASK;
	}

	Sys_LeaveCriticalSection(CRITICAL_SECTION_ONE);
	return s_polledKeyboardCount;
}

int Sys_ReturnKeyboardInputEvent(const int n, int &ch, bool &state) {
	if (n < 0 || n >= s_polledKeyboardCount) {
		ch = 0;
		state = false;
		return 0;
	}

	ch = s_polledKeyboard[n].key;
	state = s_polledKeyboard[n].down;

	if (ch == K_PRINT_SCR || ch == K_CTRL || ch == K_ALT || ch == K_RIGHT_ALT) {
		Sys_QueEvent(s_polledKeyboard[n].time, SE_KEY, ch, state, 0, NULL);
	}

	return ch;
}

void Sys_EndKeyboardInputEvents(void) {
}

int Sys_PollMouseInputEvents(void) {
	Sys_EnterCriticalSection(CRITICAL_SECTION_ONE);

	s_polledMouseCount = 0;
	while (s_mouseTail != s_mouseHead && s_polledMouseCount < SDL3_INPUT_QUEUE_SIZE) {
		s_polledMouse[s_polledMouseCount] = s_mouseQueue[s_mouseTail];
		s_polledMouseCount++;
		s_mouseTail = (s_mouseTail + 1) & SDL3_INPUT_QUEUE_MASK;
	}

	Sys_LeaveCriticalSection(CRITICAL_SECTION_ONE);
	return s_polledMouseCount;
}

int Sys_ReturnMouseInputEvent(const int n, int &action, int &value) {
	if (n < 0 || n >= s_polledMouseCount) {
		return 0;
	}

	action = s_polledMouse[n].action;
	value = s_polledMouse[n].value;

	if (action == M_DELTAZ && value == 0) {
		return 0;
	}

	return 1;
}

void Sys_EndMouseInputEvents(void) {
}

void GLimp_SetGamma(unsigned short red[256], unsigned short green[256], unsigned short blue[256]) {
	(void)red;
	(void)green;
	(void)blue;
}

bool GLimp_Init(glimpParms_t parms) {
	const char *driverName;

	common->Printf("Initializing OpenGL subsystem (SDL3 backend)\n");

	if (!s_sdlVideoActive) {
		if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
			common->Printf("SDL3: failed to initialize video subsystem: %s\n", SDL_GetError());
			return false;
		}
		s_sdlVideoActive = true;
	}

	SDL3_InitDesktopMode();

	SDL_GL_ResetAttributes();
	(void)SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	(void)SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	(void)SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	(void)SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	(void)SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	(void)SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	(void)SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	if (parms.stereo) {
		(void)SDL_GL_SetAttribute(SDL_GL_STEREO, 1);
	}
	if (parms.multiSamples > 1) {
		(void)SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		(void)SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, parms.multiSamples);
	}

	SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	s_sdlWindow = SDL_CreateWindow(GAME_NAME, parms.width, parms.height, flags);
	if (!s_sdlWindow) {
		common->Printf("SDL3: could not create window: %s\n", SDL_GetError());
		return false;
	}

	if (!parms.fullScreen) {
		(void)SDL_SetWindowPosition(s_sdlWindow, win32.win_xpos.GetInteger(), win32.win_ypos.GetInteger());
	}

	s_sdlContext = SDL_GL_CreateContext(s_sdlWindow);
	if (!s_sdlContext) {
		common->Printf("SDL3: could not create OpenGL context: %s\n", SDL_GetError());
		SDL_DestroyWindow(s_sdlWindow);
		s_sdlWindow = NULL;
		return false;
	}

	if (!SDL_GL_MakeCurrent(s_sdlWindow, s_sdlContext)) {
		common->Printf("SDL3: could not make context current: %s\n", SDL_GetError());
		(void)SDL_GL_DestroyContext(s_sdlContext);
		SDL_DestroyWindow(s_sdlWindow);
		s_sdlContext = NULL;
		s_sdlWindow = NULL;
		return false;
	}

	driverName = r_glDriver.GetString()[0] ? r_glDriver.GetString() : "opengl32";
	if (!QGL_Init(driverName)) {
		common->Printf("^3GLimp_Init() could not load r_glDriver \"%s\"^0\n", driverName);
		GLimp_Shutdown();
		return false;
	}

	if (!SDL3_ApplyScreenParms(parms)) {
		GLimp_Shutdown();
		return false;
	}

	SDL3_UpdateNativeWindowHandles();
	SDL3_LoadWGLExtensions();

	win32.activeApp = true;
	win32.wglErrors = 0;
	GLimp_EnableLogging((r_logFile.GetInteger() != 0));

	return true;
}

bool GLimp_SetScreenParms(glimpParms_t parms) {
	return SDL3_ApplyScreenParms(parms);
}

void GLimp_Shutdown(void) {
	common->Printf("Shutting down OpenGL subsystem (SDL3 backend)\n");

	IN_DeactivateMouse();
	if (s_sdlWindow && s_sdlTextInputActive) {
		(void)SDL_StopTextInput(s_sdlWindow);
		s_sdlTextInputActive = false;
	}

	if (s_sdlContext) {
		(void)SDL_GL_MakeCurrent(s_sdlWindow, NULL);
		(void)SDL_GL_DestroyContext(s_sdlContext);
		s_sdlContext = NULL;
	}

	if (s_sdlWindow) {
		SDL_DestroyWindow(s_sdlWindow);
		s_sdlWindow = NULL;
	}

	if (s_sdlVideoActive) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		s_sdlVideoActive = false;
	}

	win32.hWnd = NULL;
	win32.hDC = NULL;
	win32.hGLRC = NULL;
	win32.cdsFullscreen = false;
	glConfig.isFullscreen = false;

	SDL3_ClearInputQueues();
	QGL_Shutdown();
}

void GLimp_SwapBuffers(void) {
	if (r_swapInterval.IsModified()) {
		r_swapInterval.ClearModified();
		if (!SDL_GL_SetSwapInterval(r_swapInterval.GetInteger())) {
			common->Printf("SDL3: failed to set swap interval: %s\n", SDL_GetError());
		}
	}

	if (s_sdlWindow && !SDL_GL_SwapWindow(s_sdlWindow)) {
		common->Printf("SDL3: failed to swap window buffers: %s\n", SDL_GetError());
	}
}

/*
===========================================================

SMP acceleration

===========================================================
*/

static void GLimp_RenderThreadWrapper(void) {
	win32.glimpRenderThread();
	(void)SDL_GL_MakeCurrent(s_sdlWindow, NULL);
}

bool GLimp_SpawnRenderThread(void (*function)(void)) {
	SYSTEM_INFO info;

	GetSystemInfo(&info);
	if (info.dwNumberOfProcessors < 2) {
		return false;
	}

	win32.renderCommandsEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	win32.renderCompletedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	win32.renderActiveEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	win32.glimpRenderThread = function;

	win32.renderThreadHandle = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)GLimp_RenderThreadWrapper,
		0,
		0,
		&win32.renderThreadId);

	if (!win32.renderThreadHandle) {
		common->Error("GLimp_SpawnRenderThread: failed");
	}

	SetThreadPriority(win32.renderThreadHandle, THREAD_PRIORITY_ABOVE_NORMAL);
	return true;
}

void *GLimp_BackEndSleep(void) {
	void *data;

	ResetEvent(win32.renderActiveEvent);
	SetEvent(win32.renderCompletedEvent);
	WaitForSingleObject(win32.renderCommandsEvent, INFINITE);

	ResetEvent(win32.renderCompletedEvent);
	ResetEvent(win32.renderCommandsEvent);
	data = win32.smpData;
	SetEvent(win32.renderActiveEvent);

	return data;
}

void GLimp_FrontEndSleep(void) {
	WaitForSingleObject(win32.renderCompletedEvent, INFINITE);
}

void GLimp_WakeBackEnd(void *data) {
	int r;

	win32.smpData = data;
	r = WaitForSingleObject(win32.renderActiveEvent, 0);
	if (r == WAIT_OBJECT_0) {
		common->FatalError("GLimp_WakeBackEnd: already signaled");
	}

	r = WaitForSingleObject(win32.renderCommandsEvent, 0);
	if (r == WAIT_OBJECT_0) {
		common->FatalError("GLimp_WakeBackEnd: commands already signaled");
	}

	SetEvent(win32.renderCommandsEvent);

	r = WaitForSingleObject(win32.renderActiveEvent, 5000);
	if (r == WAIT_TIMEOUT) {
		common->FatalError("GLimp_WakeBackEnd: WAIT_TIMEOUT");
	}
}

void GLimp_ActivateContext(void) {
	if (!s_sdlWindow || !s_sdlContext) {
		return;
	}

	if (!SDL_GL_MakeCurrent(s_sdlWindow, s_sdlContext)) {
		win32.wglErrors++;
	}
}

void GLimp_DeactivateContext(void) {
	glFinish();
	if (!s_sdlWindow) {
		return;
	}

	if (!SDL_GL_MakeCurrent(s_sdlWindow, NULL)) {
		win32.wglErrors++;
	}
}

void *GLimp_ExtensionPointer(const char *name) {
	void *proc = (void *)SDL_GL_GetProcAddress(name);
	if (!proc && qwglGetProcAddress) {
		proc = (void *)qwglGetProcAddress(name);
	}

	if (!proc) {
		common->Printf("Couldn't find proc address for: %s\n", name);
	}
	return proc;
}
