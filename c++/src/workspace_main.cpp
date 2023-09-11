//#include your non unity file here
//#include "TestPlatformAPI/helpers/math.h"

#if __GLFW
int main(int argc, char** argv) {
    return 0;
}
#elif PLATFORM_DIRECTX9
#include "TestPlatformAPI/helpers/directx9/core.h"
int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow) {
    return 0;
}
#endif