// C libs
#include <math.h>
#include <cstring>
#include "stdint.h"
#include <assert.h>

// Debug
//#include <memory>
//#include <cxxabi.h>
//#include "debug/types.h"

#define UNITYBUILD

// Core
#include "helpers/types.h"
#include "helpers/math.h"
#include "helpers/easing.h"
#include "helpers/vec.h"
#include "helpers/angle.h"
#include "helpers/vec_ops.h"
#include "helpers/transform.h"
#include "helpers/color.h"
#define __WASTELADNS_HASH_DEBUG__
#include "helpers/hash.h"
#include "helpers/input.h"
#include "helpers/platform.h"
#include "helpers/camera.h"

// Platform-specific
#if PLATFORM_GLFW
#include "helpers/glfw/core.h"
#include "helpers/glfw/input.h"
#include "helpers/glfw/main.h"
#endif

#if PLATFORM_GLFW

#define __WASTELADNS_DEBUG_TEXT__
#include "helpers/renderer_debug.h"
#include "helpers/renderer.h"

#include "game.h"

namespace Platform {
    void loadLaunchConfig(Platform::WindowConfig& config) {
        // hardcoded for now
        config.window_width = 640;
        config.window_height = 480;
        config.fullscreen = false;
        config.title = "RPG Test";
    };
    template<>
    void start<Game::Instance>(Game::Instance& game, Platform::GameConfig& config, const Platform::State& platform) {
        Game::start(game, config, platform);
    };
    template<>
    void update<Game::Instance>(Game::Instance& game, Platform::GameConfig& config, const Platform::State& platform) {
        Game::update(game, config, platform);
    };
};

#endif

#if PLATFORM_GLFW
int main(int argc, char** argv) {

    int returnValue = 1;
    returnValue = Platform::GLFW::main<Game::Instance>(argc, argv);
    
    return returnValue;
}
#endif

#if PLATFORM_DIRECTX9

#include <stdio.h>
#include <windows.h>
#undef min
#undef max
#undef DELETE
#include <d3d9.h>

struct Time {
	struct Config {
		f64 targetFramerate;
		f64 maxFrameLength;
	};

	Config config;
	f64 lastFrame;
	f64 lastFrameDelta;
	f64 nextFrame;
	s64 frameCount;
	bool paused;
};
struct Game {
	Time time;
};

void start(Game& game, Platform::GameConfig& config, const Platform::State& platform) {
	game.time = {};
	game.time.config.maxFrameLength = 0.1;
	game.time.config.targetFramerate = 1.0 / 60.0;
	game.time.lastFrame = platform.time.now;

	config = {};
	config.nextFrame = platform.time.now;
	config.requestFlags = Platform::RequestFlags::PollKeyboard;

};
void update(Game& game, Platform::GameConfig& config, const Platform::State& platform) {

	f64 raw_dt = platform.time.now - game.time.lastFrame;
	game.time.lastFrameDelta = Math::min(raw_dt, game.time.config.maxFrameLength);
	game.time.lastFrame = platform.time.now;
	config.nextFrame = platform.time.now + game.time.config.targetFramerate;
	
	char str[256];
	sprintf_s(str, "%6f % 6f\n", raw_dt, platform.time.running);

	OutputDebugString(str);
};

namespace Platform {
	void loadLaunchConfig(Platform::WindowConfig& config) {
		// hardcoded for now
		config.window_width = 640;
		config.window_height = 480;
		config.fullscreen = false;
		config.title = "Platform API Test";
	};
	template<>
	void start<Game>(Game& game, Platform::GameConfig& config, const Platform::State& platform) {
		::start(game, config, platform);
	};
	template<>
	void update<Game>(Game& game, Platform::GameConfig& config, const Platform::State& platform) {
		::update(game, config, platform);
	};
};

namespace Platform {
namespace DIRECTX9 {

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (message == WM_KEYUP && wParam == VK_ESCAPE) {
				PostQuitMessage(0);
				return 0;
			}
			break;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	template <typename _GameData>
	int main(HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nCmdShow) {

		(void)lpCmdLine;
		(void)hPrevInstance;

		HWND hWnd;
		Platform::State platform = {};

		{
			WNDCLASSEX wc;
			ZeroMemory(&wc, sizeof(WNDCLASSEX));
			wc.cbSize = sizeof(WNDCLASSEX);
			// Redraw on horizontal or vertical resize
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = hInstance;
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.lpszClassName = "WindowClass";
			RegisterClassEx(&wc);

			Platform::WindowConfig windowConfig;
			loadLaunchConfig(windowConfig);

			hWnd = CreateWindowEx(
				  NULL
				, "WindowClass"
				, windowConfig.title
				, WS_OVERLAPPEDWINDOW
				, 0, 0, windowConfig.window_width, windowConfig.window_height
				, NULL /*parent*/
				, NULL /*menu*/
				, hInstance
				, NULL
			);
			if (hWnd == NULL) {
				return 0;
			}
			ShowWindow(hWnd, nCmdShow);

			platform.screen.width = windowConfig.window_width;
			platform.screen.height = windowConfig.window_height;
			platform.screen.desiredRatio = platform.screen.width / (f32)platform.screen.height;
			platform.screen.fullscreen = windowConfig.fullscreen;
		}

		u64 frequency;
		if (!QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
			return 0;
		}

		LPDIRECT3D9 d3d;
		LPDIRECT3DDEVICE9 d3ddev;
		{
			d3d = Direct3DCreate9(D3D_SDK_VERSION);
			D3DPRESENT_PARAMETERS d3dpp;
			ZeroMemory(&d3dpp, sizeof(d3dpp));
			d3dpp.Windowed = true; // TODO: support fullscreen
			d3dpp.hDeviceWindow = hWnd;
			d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
			d3dpp.BackBufferWidth = platform.screen.width;
			d3dpp.BackBufferHeight = platform.screen.height;

			d3d->CreateDevice(
				  D3DADAPTER_DEFAULT
				, D3DDEVTYPE_HAL
				, hWnd
				, D3DCREATE_SOFTWARE_VERTEXPROCESSING
				, &d3dpp
				, &d3ddev
			);
		}

		u64 start;
		QueryPerformanceCounter((LARGE_INTEGER*)&start);

		platform.time.running = 0.0;
		platform.time.now = platform.time.start = start / (f64) frequency;

		_GameData game;
		Platform::GameConfig config;
		Platform::start<_GameData>(game, config, platform);

		MSG msg;

		do {
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (platform.time.now >= config.nextFrame) {

				Platform::update<_GameData>(game, config, platform);
			}

			u64 now;
			QueryPerformanceCounter((LARGE_INTEGER*)&now);
			platform.time.now = now / (f64) frequency;
			platform.time.running = platform.time.now - platform.time.start;
		} while (msg.message != WM_QUIT);

		return 1;
	}
}
}

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow) {

	int returnValue = 1;
	returnValue = Platform::DIRECTX9::main<Game>(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	return returnValue;

}

#endif