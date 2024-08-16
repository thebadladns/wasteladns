
#import <Cocoa/Cocoa.h>
#import <mach/mach_time.h> // for mach_absolute_time

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
//#include "../../main.cpp"
//#ifndef UNITYBUILD
//#include "core.h"
//#include "input.h"
//#endif

static CFBundleRef glFramework;
void loadGLFramework()
{
    glFramework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
}
static void* getGLProcAddress(const char* procname)
{
    CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                       procname,
                                                       kCFStringEncodingASCII);

    void* symbol = CFBundleGetFunctionPointerForName(glFramework, symbolName);

    CFRelease(symbolName);

    return symbol;
}


int main(int , char** ) {
    @autoreleasepool {
        
        // load framework to access opengl functions through glad
        loadGLFramework();
        
        [NSApplication sharedApplication];
        
        // Need for menu bar and window focus
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        [NSApp setPresentationOptions:NSApplicationPresentationDefault];
        [NSApp activateIgnoringOtherApps:YES];
        
        // Press and Hold prevents some keys from emitting repeated characters
        NSDictionary* defaults = @{@"ApplePressAndHoldEnabled":@NO};
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
        
        Platform::State platform = {};
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
            
            NSView* contentView = [window contentView];
            
            NSOpenGLPixelFormatAttribute glAttributes[] = {
                NSOpenGLPFAAccelerated, NSOpenGLPFAClosestPolicy,
                NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
                NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute) 24,
                NSOpenGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute) 8,
                NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute) 24,
                NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute) 8,
                NSOpenGLPFADoubleBuffer,
                NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute) 0,
                0
            };
            NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:glAttributes];
            
            openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
            
            GLint swap = 1;
            [openGLContext setValues:&swap forParameter:NSOpenGLContextParameterSwapInterval];
            [openGLContext setView:contentView];
            [openGLContext makeCurrentContext];
        
            gladLoadGLLoader(getGLProcAddress);
            
            [(NSWindow*)window center];
            [window orderFrontRegardless];
            
            [NSApp finishLaunching];
            
            NSSize windowSize = [contentView frame].size;
            
            f32 scale = [contentView convertSizeToBacking:CGSizeMake(1,1)].width;
            platform.screen.window_width = windowSize.width * scale;
            platform.screen.window_height = windowSize.height * scale;
            platform.screen.width = config.game_width;
            platform.screen.height = config.game_height;
            platform.screen.desiredRatio = platform.screen.width / (f32)platform.screen.height;
            platform.screen.fullscreen = config.fullscreen;
            __DEBUGEXP(platform.screen.text_scale = scale);
        }

        struct MouseQueue {
            u8 buttons[::Input::Mouse::Keys::COUNT];
            f32 x, y;
            f32 scrollx, scrolly;
        };
        u8 keyboard_queue[::Input::Keyboard::Keys::COUNT]{};
        MouseQueue mouse_queue = {};
        const int hotkeyMask = NSEventModifierFlagCommand | NSEventModifierFlagOption | NSEventModifierFlagControl | NSEventModifierFlagCapsLock;
        
        mach_timebase_info_data_t ticks_to_nanos;
        mach_timebase_info(&ticks_to_nanos);
        f64 frequency = (1e9 * ticks_to_nanos.denom) / (f64)ticks_to_nanos.numer;
        
        platform.time.running = 0.0;
        platform.time.now = platform.time.start = mach_absolute_time() / frequency;
        
        Game::Instance game;
        Platform::GameConfig config;
        Game::start(game, config, platform);
        
        do
        {
            @autoreleasepool {
                
                if (platform.time.now >= config.nextFrame) {
                    // Input
                    {
                        namespace KB = ::Input::Keyboard;
                        namespace MS = ::Input::Mouse;
                        
                        NSEvent *event = nil;
                        do {
                            event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                       untilDate:nil
                                                          inMode:NSDefaultRunLoopMode
                                                         dequeue:YES];
                            NSEventType eventType = [event type];
                            
                            switch (eventType) {
                                case NSEventTypeKeyUp: {
                                    // Handle events like cmd+q etc
                                    if ([event modifierFlags] & hotkeyMask) { [NSApp sendEvent:event]; break; }
                                    ::Input::queueKeyUp(keyboard_queue, platform.input.keyboard.current, (KB::Keys::Enum)[event keyCode]);
                                } break;
                                case NSEventTypeKeyDown: {
                                    // Handle events like cmd+q etc
                                    if ([event modifierFlags] & hotkeyMask) { [NSApp sendEvent:event]; break; }
                                    ::Input::queueKeyDown(keyboard_queue, platform.input.keyboard.current, (KB::Keys::Enum)[event keyCode]);
                                } break;
                                case NSEventTypeMouseMoved:
                                case NSEventTypeLeftMouseDragged:
                                case NSEventTypeRightMouseDragged:
                                case NSEventTypeOtherMouseDragged: {
                                    // todo: won't work when hiding cursor
                                    // todo: doesn't work while pressing mouse buttons
                                    NSPoint pos = [event locationInWindow];
                                    mouse_queue.x = pos.x;
                                    mouse_queue.y = platform.screen.height-pos.y;
                                } break;
                                case NSEventTypeLeftMouseDown:
                                case NSEventTypeRightMouseDown: {
                                    MS::Keys::Enum keycode = (MS::Keys::Enum)((eventType >> 1) & 0x1);
                                    ::Input::queueKeyDown(mouse_queue.buttons, platform.input.mouse.current, keycode);
                                } break;
                                case NSEventTypeLeftMouseUp:
                                case NSEventTypeRightMouseUp: {
                                    MS::Keys::Enum keycode = (MS::Keys::Enum)((eventType >> 2) & 0x1);
                                    ::Input::queueKeyUp(mouse_queue.buttons, platform.input.mouse.current, keycode);
                                } break;
                                    
                                case NSEventTypeScrollWheel: {
                                    float scrollx = [event deltaX];
                                    float scrolly = [event deltaY];
                                    mouse_queue.scrollx = scrollx;
                                    mouse_queue.scrolly = scrolly;
                                } break;
                                default: {
                                    [NSApp sendEvent:event];
                                } break;
                            }
                        } while (event);
                        
                        [NSApp updateWindows];
                        
                        // Input
                        memcpy(platform.input.keyboard.last, platform.input.keyboard.current, sizeof(u8) * KB::Keys::COUNT);
                        for (int i = 0; i < KB::Keys::COUNT; i++) {
                            ::Input::unqueueKey(platform.input.keyboard.current, keyboard_queue, i);
                        }
                        memcpy(platform.input.mouse.last, platform.input.mouse.current, sizeof(u8) * MS::Keys::COUNT);
                        for (int i = 0; i < MS::Keys::COUNT; i++) {
                            ::Input::unqueueKey(platform.input.mouse.current, mouse_queue.buttons, i);
                        }
                        platform.input.mouse.dx = mouse_queue.x - platform.input.mouse.x;
                        platform.input.mouse.dy = mouse_queue.y - platform.input.mouse.y;
                        platform.input.mouse.x = mouse_queue.x;
                        platform.input.mouse.y = mouse_queue.y;
                        platform.input.mouse.scrolldx = mouse_queue.scrollx;
                        platform.input.mouse.scrolldy = mouse_queue.scrolly;
                        mouse_queue.scrollx = 0.f;
                        mouse_queue.scrolly = 0.f;
//                      todo: gamepad
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

    }

    return 1;
}
