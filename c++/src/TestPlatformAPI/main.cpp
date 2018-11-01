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
#include <stdio.h>

#endif

#if PLATFORM_GLFW
int main(int argc, char** argv) {

    int returnValue = 1;
    returnValue = Platform::GLFW::main<Game::Instance>(argc, argv);
    
    return returnValue;
}
#endif

#if PLATFORM_DIRECTX9
struct Game {
};

void start(Game& game, Platform::GameConfig& config, const Platform::State& platform) {

};
void update(Game& game, Platform::GameConfig& config, const Platform::State& platform) {

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

#include <windows.h>
#include <d3d9.h>
//#pragma comment (lib, "d3d9.lib")

namespace Platform {
namespace DIRECTX9 {

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		if (message == WM_DESTROY) {
			PostQuitMessage(0);
			return 0;
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

		HWND hWnd = CreateWindowEx(
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
		ShowWindow(hWnd, nCmdShow);

		LPDIRECT3D9 d3d;
		LPDIRECT3DDEVICE9 d3ddev;
		{
			d3d = Direct3DCreate9(D3D_SDK_VERSION);
			D3DPRESENT_PARAMETERS d3dpp;
			ZeroMemory(&d3dpp, sizeof(d3dpp));
			d3dpp.Windowed = true;
			d3dpp.hDeviceWindow = hWnd;
			d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
			d3dpp.BackBufferWidth = windowConfig.window_width;
			d3dpp.BackBufferHeight = windowConfig.window_height;

			d3d->CreateDevice(
				  D3DADAPTER_DEFAULT
				, D3DDEVTYPE_HAL
				, hWnd
				, D3DCREATE_SOFTWARE_VERTEXPROCESSING
				, &d3dpp
				, &d3ddev
			);
		}

		while (1) {
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (msg.message == WM_QUIT) {
				break;
			}

			//render_frame();
		}

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