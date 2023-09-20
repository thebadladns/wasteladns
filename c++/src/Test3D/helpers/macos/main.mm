
#import <Cocoa/Cocoa.h>
//#include <dlfcn.h>
#import <mach/mach_time.h> // for mach_absolute_time

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
//#include "../../main.cpp"
//#ifndef UNITYBUILD
//#include "core.h"
//#include "input.h"
//#endif

@interface WindowDelegate : NSObject<NSWindowDelegate>
@end

@implementation WindowDelegate
-(void)windowWillClose:(NSNotification*)notification
{
    
}

@end // WindowDelegate

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
//    for (_GLFWwindow* window = _glfw.windowListHead;  window;  window = window->next)
//        _glfwInputWindowCloseRequest(window);
//
    return NSTerminateCancel;
}

- (void)applicationDidChangeScreenParameters:(NSNotification *) notification
{
//    for (_GLFWwindow* window = _glfw.windowListHead;  window;  window = window->next)
//    {
//        if (window->context.client != GLFW_NO_API)
//            [window->context.nsgl.object update];
//    }
//
//    _glfwPollMonitorsCocoa();
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
//    if (_glfw.hints.init.ns.menubar)
//    {
//        // Menu bar setup must go between sharedApplication and finishLaunching
//        // in order to properly emulate the behavior of NSApplicationMain
//
//        if ([[NSBundle mainBundle] pathForResource:@"MainMenu" ofType:@"nib"])
//        {
//            [[NSBundle mainBundle] loadNibNamed:@"MainMenu"
//                                          owner:NSApp
//                                topLevelObjects:&_glfw.ns.nibObjects];
//        }
//        else
//            createMenuBar();
//    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
//    _glfwPostEmptyEventCocoa();
//    [NSApp stop:nil];
}

- (void)applicationDidHide:(NSNotification *)notification
{
//    for (int i = 0;  i < _glfw.monitorCount;  i++)
//        _glfwRestoreVideoModeCocoa(_glfw.monitors[i]);
}

@end // AppDelegate


int main(int , char** ) {
    @autoreleasepool {

        // todo: consider opengl via dlfcn.h
        // https://github.com/andr3wmac/Torque6/blob/db6cd08f18f4917e0c6557b2766fb40d8e2bee39/lib/bgfx/include/bx/os.h
        // https://github.com/andr3wmac/Torque6/blob/db6cd08f18f4917e0c6557b2766fb40d8e2bee39/lib/bgfx/src/glcontext_nsgl.mm
//                dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LOCAL|RTLD_LAZY);
        
        
        
        [NSApplication sharedApplication];
        AppDelegate* delegate = [[AppDelegate alloc] init];
        [NSApp setDelegate:delegate];
        
        // Need for menu bar and window focus
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        [NSApp setPresentationOptions:NSApplicationPresentationDefault];
        [NSApp activateIgnoringOtherApps:YES];
        
        // Press and Hold prevents some keys from emitting repeated characters
        NSDictionary* defaults = @{@"ApplePressAndHoldEnabled":@NO};
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
        
        Platform::State platform;
        id window;
        NSOpenGLContext* openGLContext;
        {
            Platform::WindowConfig config;
            Game::loadLaunchConfig(config);
            NSString* nstitle = [NSString stringWithUTF8String:config.title];
            
            // Menu
            {
                id menuBar = [[NSMenu alloc] init];
                id appMenuItem = [[NSMenuItem alloc] init];
                [menuBar addItem:appMenuItem];
                id appMenu = [[NSMenu alloc] init];
                [NSApp setMainMenu:menuBar];
                id quitMenuItem = [[NSMenuItem alloc]
                                   initWithTitle:[@"Quit " stringByAppendingString:nstitle]
                                   action:@selector(terminate:) keyEquivalent:@"q"];
                [appMenu addItem:quitMenuItem];
                [appMenuItem setSubmenu: appMenu];
            }
            
            // Window
            window = [[NSWindow alloc]
                      initWithContentRect:NSMakeRect(0, 0, config.window_width, config.window_height) styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:NO];
            // since the autorelease pool will release it too
            [window setReleasedWhenClosed:NO];
            [window setTitle:nstitle];
            [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
            
            WindowDelegate* windowDelegate = [[WindowDelegate alloc] init];
            [window setDelegate:windowDelegate];
            
            NSView* contentView = [window contentView];
            
            NSOpenGLPixelFormatAttribute pixelFormatAttributes[] = {
                NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
                NSOpenGLPFADepthSize,    (NSOpenGLPixelFormatAttribute) 24,
                NSOpenGLPFADoubleBuffer, (NSOpenGLPixelFormatAttribute) true,
                NSOpenGLPFAAccelerated,  (NSOpenGLPixelFormatAttribute) true,
                0, 0,
            };
            
            NSOpenGLPixelFormatAttribute glAttributes[] = {
                NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
                NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute) 24,
                NSOpenGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute) 8,
                NSOpenGLPFADoubleBuffer, true,
                NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute) 1,
                NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute) 4,
                NSOpenGLPFAAccelerated,  (NSOpenGLPixelFormatAttribute) true,
            };
            NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:glAttributes];
            
            openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
            
            GLint swap = 1;
            [openGLContext setValues:&swap forParameter:NSOpenGLCPSwapInterval];
            [openGLContext setView:contentView];
            [openGLContext makeCurrentContext];
            
            gladLoadGL();
            
            [(NSWindow*)window center];
            [window orderFrontRegardless];
            
            [NSApp finishLaunching];
            
            NSSize windowSize = [contentView frame].size;
            
            platform.screen.width = windowSize.width;
            platform.screen.height = windowSize.height;
            platform.screen.desiredRatio = platform.screen.width / (f32)platform.screen.height;
            platform.screen.fullscreen = config.fullscreen;
        }

        ::Input::Keyboard::PollData keyboardPollData;
        ::Input::Keyboard::load(keyboardPollData.mapping);
        memset(platform.input.keyboard.last, 0, sizeof(u8) * ::Input::Keyboard::Keys::COUNT);
        memset(platform.input.keyboard.current, 0, sizeof(u8) * ::Input::Keyboard::Keys::COUNT);
        
        keyboardPollData.queue = {};
        ::Input::Mouse::PollData mousePollData = {};
        
        mach_timebase_info_data_t ticks_to_nanos;
        mach_timebase_info(&ticks_to_nanos);
        f64 frequency = (1e9 * ticks_to_nanos.denom) / (f64)ticks_to_nanos.numer;
        
        platform.time.running = 0.0;
        platform.time.now = platform.time.start = mach_absolute_time() / frequency;
        
        Game::Instance game;
        Platform::GameConfig config;
        Game::start(game, config, platform);
        
        if ((config.requestFlags & (Platform::RequestFlags::PollMouse)) != 0) {
            memset(platform.input.mouse.last, 0, sizeof(u8) * ::Input::Mouse::Keys::COUNT);
            memset(platform.input.mouse.current, 0, sizeof(u8) * ::Input::Mouse::Keys::COUNT);
        }
        
        do
        {
            @autoreleasepool {
                
                if (platform.time.now >= config.nextFrame) {
                    // Input
                    {
                        // todo: figure out mouse
                        if ((config.requestFlags & (Platform::RequestFlags::PollMouse)) != 0) {
                            ::Input::Mouse::resetState(mousePollData);
                        }
                        NSEvent *event = nil;
                        do {
                            event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                       untilDate:nil
                                                          inMode:NSDefaultRunLoopMode
                                                         dequeue:YES];
                            NSEventType eventType = [event type];
                            switch (eventType) {
                                case NSEventTypeKeyUp:
                                case NSEventTypeKeyDown: {
                                    int hotkeyMask = NSEventModifierFlagCommand | NSEventModifierFlagOption | NSEventModifierFlagControl | NSEventModifierFlagCapsLock;
                                    if ([event modifierFlags] & hotkeyMask) {
                                        // Handle events like cmd+q etc
                                        [NSApp sendEvent:event];
                                        break;
                                    }
                                    namespace KB = ::Input::Keyboard;
                                    const KB::Keys::Enum key = keyboardPollData.mapping.mapping[[event keyCode]];
                                    bool state = eventType == NSEventTypeKeyDown;
                                    keyboardPollData.queue.keyStates[key] = state;
                                } break;
                                case NSEventTypeMouseMoved:
                                case NSEventTypeLeftMouseDragged:
                                case NSEventTypeRightMouseDragged:
                                case NSEventTypeOtherMouseDragged: {
                                    // todo: won't work when hiding cursor
                                    // todo: doesn't work while pressing mouse buttons
                                    NSPoint pos = [event locationInWindow];
                                    mousePollData.x = pos.x;
                                    mousePollData.y = pos.y;
                                } break;
                                case NSEventTypeLeftMouseDown:
                                case NSEventTypeLeftMouseUp:
                                case NSEventTypeRightMouseDown:
                                case NSEventTypeRightMouseUp: {
                                    namespace MS = ::Input::Mouse;
                                    MS::Keys::Enum key = (eventType == NSEventTypeLeftMouseDown || eventType == NSEventTypeLeftMouseUp) ? MS::Keys::BUTTON_LEFT : MS::Keys::BUTTON_RIGHT;
                                    bool state = eventType == NSEventTypeLeftMouseDown || eventType == NSEventTypeRightMouseDown;
                                    mousePollData.keyStates[key] = state;
                                } break;
                                    
                                case NSEventTypeScrollWheel: {
                                    float scrollx = [event deltaX];
                                    float scrolly = [event deltaY];
                                    mousePollData.scrollx = scrollx;
                                    mousePollData.scrolly = scrolly;
                                } break;
                                default: {
                                    [NSApp sendEvent:event];
                                } break;
                            }
//                            [NSApp sendEvent:event];
                        } while (event);
                        
                        [NSApp updateWindows];
                        
                        // Input
                        if ((config.requestFlags & (Platform::RequestFlags::PollKeyboard)) != 0) {
                            ::Input::Keyboard::pollState(platform.input.keyboard, keyboardPollData.queue);
                        }
                        if ((config.requestFlags & (Platform::RequestFlags::PollMouse)) != 0) {
                            ::Input::Mouse::pollState(platform.input.mouse, mousePollData);
                            
                        }
                        //                            for (u32 i = 0; i < platform.input.padCount; i++) {
                        //                                ::Input::Gamepad::pollState(platform.input.pads[i], keyboardPollData.queue, keyboardPadMappings[i]);
                    }
                    
                    [openGLContext makeCurrentContext];
                    Game::update(game, config, platform);
                    [openGLContext flushBuffer];
                    
                }
                
                u64 now = mach_absolute_time();
                platform.time.now = now / frequency;
                platform.time.running = platform.time.now - platform.time.start;
            }

        } while (!config.quit);

        
//        [NSApp run];
        
    }

    return 1;
}
