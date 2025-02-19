#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

@interface AppDelegate : NSObject<NSApplicationDelegate> { bool terminated; }
- (id)init;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (bool)applicationHasTerminated;
@end
@implementation AppDelegate
- (id)init {
    if (!(self = [super init])) { return nil; }
    self->terminated = false;
    return self;
}
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    self->terminated = true;
    return NSTerminateCancel;
}
- (bool)applicationHasTerminated { return self->terminated; }
@end

@interface MainWindowDelegate: NSObject<NSWindowDelegate>
@end
@implementation MainWindowDelegate
- (void)windowWillClose:(id)sender {
    [NSApp terminate:nil];
}
@end

int main(int , char** ) {
@autoreleasepool {
    
    platform::State platform = {};

    [NSApplication sharedApplication];
    AppDelegate* appDelegate = [[AppDelegate alloc] init];
    [NSApp setDelegate:appDelegate];
    
    // Need for menu bar and window focus
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    
    [NSApp setPresentationOptions:NSApplicationPresentationDefault];
    [NSApp activateIgnoringOtherApps:YES];
    
    // Press and Hold prevents some keys from emitting repeated characters
    NSDictionary* defaults = @{@"ApplePressAndHoldEnabled":@NO};
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
    
    id window;
    f32 windowScale = 1.f;
    NSOpenGLContext* openGLContext;
    {
        platform::LaunchConfig config;
        game::loadLaunchConfig(config);
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
                  initWithContentRect:NSMakeRect(0, 0, config.window_width, config.window_height) styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable backing:NSBackingStoreBuffered defer:NO];
        [window setReleasedWhenClosed:NO]; // since the autorelease pool will release it too
        [window setTitle:nstitle];
        
        MainWindowDelegate* windowDelegate = [[MainWindowDelegate alloc] init];
        [window setDelegate:windowDelegate];
        
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
        
        renderer::driver::loadGLExtensions();
        
        [(NSWindow*)window center];
        [window orderFrontRegardless];
        
        [NSApp finishLaunching];
        
        NSSize windowSize = [contentView frame].size;
        
        windowScale = [contentView convertSizeToBacking:CGSizeMake(1,1)].width;
        platform.screen.window_width = windowSize.width * windowScale;
        platform.screen.window_height = windowSize.height * windowScale;
        platform.screen.width = config.game_width;
        platform.screen.height = config.game_height;
        platform.screen.desiredRatio = platform.screen.width / (f32)platform.screen.height;
        platform.screen.fullscreen = config.fullscreen;
        platform.screen.window_scale = windowScale;
    }
    
    const int hotkeyMask = NSEventModifierFlagCommand | NSEventModifierFlagOption | NSEventModifierFlagControl | NSEventModifierFlagCapsLock;
    const f32 actualWindowHeight = [[window contentView] frame].size.height;
    {   // init mouse pos so we don't get a big delta movement on the first click
        NSPoint pos = [window  mouseLocationOutsideOfEventStream];
        platform.input.mouse.x = pos.x * windowScale;
        platform.input.mouse.y = (actualWindowHeight-pos.y) * windowScale;
    }
    // gamepad
    ::input::gamepad::init_hid_pads_mac(platform);
    
    mach_timebase_info_data_t ticks_to_nanos;
    mach_timebase_info(&ticks_to_nanos);
    const f64 frequency = (1e9 * ticks_to_nanos.denom) / (f64)ticks_to_nanos.numer;
    // todo: this is empirical, (1./sysconf(_SC_CLK_TCK)) is too coarse to be useful
    const f64 timerResolutionMs = 1.;
    
    platform.time.running = 0.0;
    platform.time.now = platform.time.start = mach_absolute_time() / frequency;
    
    game::Instance game;
    platform::GameConfig config;
    game::start(game, config, platform);
    
    do
    {
        @autoreleasepool {
        // Input
        {
        // propagate last state
        for (u32 i = 0; i < platform.input.padCount; i++)
        { platform.input.pads[i].last_keys = platform.input.pads[i].curr_keys; }
        memcpy(
            platform.input.keyboard.last, platform.input.keyboard.current,
            sizeof(u8)* ::input::keyboard::Keys::COUNT);
        memcpy(
            platform.input.mouse.last, platform.input.mouse.curr,
            sizeof(u8) * ::input::mouse::Keys::COUNT);
        const f32 mouse_prevx = platform.input.mouse.x, mouse_prevy = platform.input.mouse.y;
        platform.input.mouse.dx = platform.input.mouse.dy =
            platform.input.mouse.scrolldx = platform.input.mouse.scrolldy = 0.f;
        // gather current state
        NSUInteger modifierFlags = NSEvent.modifierFlags; // calls NSEventTypeFlagsChanged
        platform.input.keyboard.current[::input::keyboard::Keys::LEFT_SHIFT] =
            platform.input.keyboard.current[::input::keyboard::Keys::RIGHT_SHIFT] =
                (modifierFlags & NSEventModifierFlagShift) != 0;
        platform.input.keyboard.current[::input::keyboard::Keys::LEFT_ALT] =
            platform.input.keyboard.current[::input::keyboard::Keys::RIGHT_SHIFT] =
                (modifierFlags & NSEventModifierFlagOption) != 0;
        platform.input.keyboard.current[::input::keyboard::Keys::LEFT_CONTROL] =
            platform.input.keyboard.current[::input::keyboard::Keys::RIGHT_CONTROL] =
                (modifierFlags & NSEventModifierFlagControl) != 0;
        // consume events
        NSEvent *event = nil;
        do {
            event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantPast]
                                            inMode:NSDefaultRunLoopMode
                                            dequeue:YES];
            NSEventType eventType = [event type];
            bool event_passthrough = true;
            switch (eventType) {
                case NSEventTypeKeyUp: {
                    if ([event modifierFlags] & hotkeyMask) { break; } // Handle events like cmd+q etc
                    platform.input.keyboard.current[(::input::keyboard::Keys::Enum)[event keyCode]] = 0;
                    event_passthrough = false; // disable key error sound
                } break;
                case NSEventTypeKeyDown: {
                    if ([event modifierFlags] & hotkeyMask) { break; } // Handle events like cmd+q etc
                    platform.input.keyboard.current[(::input::keyboard::Keys::Enum)[event keyCode]] = 1;
                    event_passthrough = false; // disable key error sound
                } break;
                case NSEventTypeMouseMoved:
                case NSEventTypeLeftMouseDragged:
                case NSEventTypeRightMouseDragged:
                case NSEventTypeOtherMouseDragged: {
                    // todo: won't work when hiding cursor
                    NSPoint pos = [event locationInWindow];
                    platform.input.mouse.x = pos.x * windowScale;
                    platform.input.mouse.y = (actualWindowHeight-pos.y) * windowScale;
                    platform.input.mouse.dx = platform.input.mouse.x - mouse_prevx;
                    platform.input.mouse.dy = platform.input.mouse.y - mouse_prevy;
                } break;
                case NSEventTypeLeftMouseDown:
                case NSEventTypeRightMouseDown: {
                    ::input::mouse::Keys::Enum keycode =
                    (::input::mouse::Keys::Enum)((eventType >> 1) & 0x1);
                    platform.input.mouse.curr[keycode] = 1;
                } break;
                case NSEventTypeLeftMouseUp:
                case NSEventTypeRightMouseUp: {
                    ::input::mouse::Keys::Enum keycode =
                    (::input::mouse::Keys::Enum)((eventType >> 2) & 0x1);
                    platform.input.mouse.curr[keycode] = 0;
                } break;
                case NSEventTypeScrollWheel: {
                    float scrollx = [event deltaX];
                    float scrolly = [event deltaY];
                    platform.input.mouse.scrolldx += scrollx;
                    platform.input.mouse.scrolldy += scrolly;
                } break;
                default: break;
            }
            if (event_passthrough) { // passthrough to handle things like title bar buttons
                [NSApp sendEvent:event];
            }
        } while (event);
        [NSApp updateWindows];
        config.quit = [appDelegate applicationHasTerminated];
        } // Input
        
        // update and render game
        [openGLContext makeCurrentContext];
        game::update(game, config, platform);
        [openGLContext flushBuffer];
            
        // frame time handling: sleep a number of OS timer periods until we hit our
        // next target time: https://blog.bearcats.nl/perfect-sleep-function/
        u64 now = mach_absolute_time();
        platform.time.now = now / frequency;
        if (platform.time.now < config.nextFrame) {
            // sleep with 1ms granularity away from the target time
            const f64 sleepSeconds = config.nextFrame - platform.time.now;
            // round up desired sleep time, to compensate for any rounding up the scheduler may do
            const f64 sleepMs = sleepSeconds * 1000. - (timerResolutionMs + 0.02);
            s32 sleepPeriodCount = s32(sleepMs / timerResolutionMs);
            if (sleepPeriodCount > 0) {
                // todo: neither of these options is great, consider CVDisplayLink
                mach_wait_until(now + frequency * sleepPeriodCount * timerResolutionMs * 0.001);
                //usleep(useconds_t(1000. * sleepPeriodCount * timerResolutionMs));
            }
            do { // spin-lock until the next target time is hit
                u64 now = mach_absolute_time();
                platform.time.now = now / frequency;
            } while (platform.time.now < config.nextFrame);
        }
        platform.time.running = platform.time.now - platform.time.start;
        } // autorelease pool
    } while (!config.quit);
    
    }
    return 1;
}
