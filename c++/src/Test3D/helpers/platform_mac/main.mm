
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

        ::Input::Keyboard::PollData keyboardPollData;
        keyboardPollData.queue = {};
        memset(platform.input.keyboard.last, 0, sizeof(u8) * ::Input::Keyboard::Keys::COUNT);
        memset(platform.input.keyboard.current, 0, sizeof(u8) * ::Input::Keyboard::Keys::COUNT);
        ::Input::Mouse::PollData mousePollData = {};
        platform.input.padCount = 0;
        
        mach_timebase_info_data_t ticks_to_nanos;
        mach_timebase_info(&ticks_to_nanos);
        f64 frequency = (1e9 * ticks_to_nanos.denom) / (f64)ticks_to_nanos.numer;
        
        platform.time.running = 0.0;
        platform.time.now = platform.time.start = mach_absolute_time() / frequency;
        
        Game::Instance game;
        Platform::GameConfig config;
        Game::start(game, config, platform);
        
        memset(platform.input.mouse.last, 0, sizeof(u8) * ::Input::Mouse::Keys::COUNT);
        memset(platform.input.mouse.current, 0, sizeof(u8) * ::Input::Mouse::Keys::COUNT);
        
        do
        {
            @autoreleasepool {
                
                if (platform.time.now >= config.nextFrame) {
                    // Input
                    {
                        // todo: figure out mouse
                        ::Input::Mouse::resetState(mousePollData);
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
                                    bool state = eventType == NSEventTypeKeyDown;
                                    keyboardPollData.queue.keyStates[(KB::Keys::Enum)[event keyCode]] = state;
                                } break;
                                case NSEventTypeMouseMoved:
                                case NSEventTypeLeftMouseDragged:
                                case NSEventTypeRightMouseDragged:
                                case NSEventTypeOtherMouseDragged: {
                                    // todo: won't work when hiding cursor
                                    // todo: doesn't work while pressing mouse buttons
                                    NSPoint pos = [event locationInWindow];
                                    mousePollData.x = pos.x;
                                    mousePollData.y = platform.screen.height-pos.y;
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
                        } while (event);
                        
                        [NSApp updateWindows];
                        
                        // Input
                        ::Input::Keyboard::pollState(platform.input.keyboard, keyboardPollData.queue);
                        ::Input::Mouse::pollState(platform.input.mouse, mousePollData);
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

    }

    return 1;
}
