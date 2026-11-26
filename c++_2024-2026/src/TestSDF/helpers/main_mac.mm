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
    
    platform::state = {};

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
        
        gfx::rhi::loadGLExtensions();
        
        [(NSWindow*)window center];
        [window orderFrontRegardless];
        
        [NSApp finishLaunching];
        
        NSSize windowSize = [contentView frame].size;
        
        windowScale = [contentView convertSizeToBacking:CGSizeMake(1,1)].width;
        platform::state.screen.window_width = windowSize.width * windowScale;
        platform::state.screen.window_height = windowSize.height * windowScale;
        platform::state.screen.width = config.game_width;
        platform::state.screen.height = config.game_height;
        platform::state.screen.desiredRatio = platform::state.screen.width / (f32)platform::state.screen.height;
        platform::state.screen.fullscreen = config.fullscreen;
        platform::state.screen.window_scale = windowScale;
    }
    
    const int hotkeyMask = NSEventModifierFlagCommand | NSEventModifierFlagOption | NSEventModifierFlagControl | NSEventModifierFlagCapsLock;
    const f32 actualWindowHeight = [[window contentView] frame].size.height;
    {   // init mouse pos so we don't get a big delta movement on the first click
        NSPoint pos = [window  mouseLocationOutsideOfEventStream];
        platform::state.input.mouse.x = pos.x * windowScale;
        platform::state.input.mouse.y = (actualWindowHeight-pos.y) * windowScale;
    }
    // gamepad
    ::input::gamepad::init_hid_pads_mac(
        ::platform::state.input.pads, &::platform::state.input.padCount);

    // Initialize page size, for virtual memory allocators
    allocator::pagesize = getpagesize();
    
    mach_timebase_info_data_t ticks_to_nanos;
    mach_timebase_info(&ticks_to_nanos);
    const f64 frequency = (1e9 * ticks_to_nanos.denom) / (f64)ticks_to_nanos.numer;
    // todo: this is empirical, (1./sysconf(_SC_CLK_TCK)) is too coarse to be useful
    const f64 timerResolutionMs = 1.;
    
    platform::state.time.running = 0.0;
    platform::state.time.now = platform::state.time.start = mach_absolute_time() / frequency;
    
    game::Instance game;
    platform::GameConfig config;
    game::start(game, config);
    
    do
    {
        @autoreleasepool {
        // Input
        {
        // propagate last state
        for (u32 i = 0; i < platform::state.input.padCount; i++)
        { platform::state.input.pads[i].last_keys = platform::state.input.pads[i].curr_keys; }
        memcpy(
            platform::state.input.keyboard.last, platform::state.input.keyboard.current,
            sizeof(u8)* ::input::keyboard::Keys::COUNT);
        memcpy(
            platform::state.input.mouse.last, platform::state.input.mouse.curr,
            sizeof(u8) * ::input::mouse::Keys::COUNT);
        const f32 mouse_prevx = platform::state.input.mouse.x;
        const f32 mouse_prevy = platform::state.input.mouse.y;
        platform::state.input.mouse.dx = platform::state.input.mouse.dy =
            platform::state.input.mouse.scrolldx = platform::state.input.mouse.scrolldy = 0.f;
        // gather current state
        NSUInteger modifierFlags = NSEvent.modifierFlags; // calls NSEventTypeFlagsChanged
        platform::state.input.keyboard.current[::input::keyboard::Keys::LEFT_SHIFT] =
            platform::state.input.keyboard.current[::input::keyboard::Keys::RIGHT_SHIFT] =
                (modifierFlags & NSEventModifierFlagShift) != 0;
        platform::state.input.keyboard.current[::input::keyboard::Keys::LEFT_ALT] =
            platform::state.input.keyboard.current[::input::keyboard::Keys::RIGHT_SHIFT] =
                (modifierFlags & NSEventModifierFlagOption) != 0;
        platform::state.input.keyboard.current[::input::keyboard::Keys::LEFT_CONTROL] =
            platform::state.input.keyboard.current[::input::keyboard::Keys::RIGHT_CONTROL] =
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
                    platform::state.input.keyboard.current[(::input::keyboard::Keys::Enum)[event keyCode]] = 0;
                    event_passthrough = false; // disable key error sound
                } break;
                case NSEventTypeKeyDown: {
                    if ([event modifierFlags] & hotkeyMask) { break; } // Handle events like cmd+q etc
                    platform::state.input.keyboard.current[(::input::keyboard::Keys::Enum)[event keyCode]] = 1;
                    event_passthrough = false; // disable key error sound
                } break;
                case NSEventTypeMouseMoved:
                case NSEventTypeLeftMouseDragged:
                case NSEventTypeRightMouseDragged:
                case NSEventTypeOtherMouseDragged: {
                    // todo: won't work when hiding cursor
                    NSPoint pos = [event locationInWindow];
                    platform::state.input.mouse.x = pos.x * windowScale;
                    platform::state.input.mouse.y = (actualWindowHeight-pos.y) * windowScale;
                    platform::state.input.mouse.dx = platform::state.input.mouse.x - mouse_prevx;
                    platform::state.input.mouse.dy = platform::state.input.mouse.y - mouse_prevy;
                } break;
                case NSEventTypeLeftMouseDown:
                case NSEventTypeRightMouseDown: {
                    ::input::mouse::Keys::Enum keycode =
                    (::input::mouse::Keys::Enum)((eventType >> 1) & 0x1);
                    platform::state.input.mouse.curr[keycode] = 1;
                } break;
                case NSEventTypeLeftMouseUp:
                case NSEventTypeRightMouseUp: {
                    ::input::mouse::Keys::Enum keycode =
                    (::input::mouse::Keys::Enum)((eventType >> 2) & 0x1);
                    platform::state.input.mouse.curr[keycode] = 0;
                } break;
                case NSEventTypeScrollWheel: {
                    float scrollx = [event deltaX];
                    float scrolly = [event deltaY];
                    platform::state.input.mouse.scrolldx += scrollx;
                    platform::state.input.mouse.scrolldy += scrolly;
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
        game::update(game, config);
        [openGLContext flushBuffer];
            
        // frame time handling: sleep a number of OS timer periods until we hit our
        // next target time: https://blog.bearcats.nl/perfect-sleep-function/
        u64 now = mach_absolute_time();
        platform::state.time.now = now / frequency;
        if (platform::state.time.now < config.nextFrame) {
            // sleep with 1ms granularity away from the target time
            const f64 sleepSeconds = config.nextFrame - platform::state.time.now;
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
                platform::state.time.now = now / frequency;
            } while (platform::state.time.now < config.nextFrame);
        }
        platform::state.time.running = platform::state.time.now - platform::state.time.start;
        } // autorelease pool
    } while (!config.quit);
    
    }
    return 1;
}
