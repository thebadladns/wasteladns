#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
//#include "../../main.cpp"
//#ifndef UNITYBUILD
//#include "core.h"
//#include "input.h"
//#endif

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

static void valueChangedGamepadCallback(void *context, IOReturn result, void *sender, IOHIDReportType type, uint32_t reportID, uint8_t *report, CFIndex reportLength) {
  
    namespace Pad = ::Input::Gamepad;
    
    ::Platform::Input& ctx = *(::Platform::Input*)context;
    IOHIDDeviceRef device = (IOHIDDeviceRef)sender;
    
    u32 padToUpdate = ctx.padCount;
    for (u32 padidx = 0; padidx < ctx.padCount; padidx++) {
        if (ctx.pads[padidx].deviceHandle == device) {
            padToUpdate = padidx;
            break;
        }
    }
    
    if (padToUpdate >= COUNT_OF(ctx.pads)) return;
    Pad::State& pad = ctx.pads[padToUpdate];
    
    // this will be a new pad, check the type
    if (padToUpdate == ctx.padCount) {
        pad = {};

        u64 vendorID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)) unsignedIntegerValue];
        u64 productID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)) unsignedIntegerValue];
        
        const u64 sonyVendorID = 0x054C;
        const u64 ds4Gen1ProductID = 0x05C4;
        const u64 ds4Gen2ProductID = 0x09CC;
        const u64 microsoftVendorID = 0x045e;
        const u64 xbox360wireless3rdparty1 = 0x02a9;
        const u64 xbox360wireless3rdparty2 = 0x0291;
        const u64 xbox360wireless = 0x028e;
        if (vendorID == sonyVendorID && (productID == ds4Gen1ProductID || productID == ds4Gen2ProductID)) {
            pad.type = Pad::Type::DUALSHOCK4;
        } else if (vendorID == microsoftVendorID && (productID == xbox360wireless3rdparty1 || productID == xbox360wireless3rdparty2 || productID == xbox360wireless)) {
            pad.type = Pad::Type::XBOX360;
        } else {
            pad.type = Pad::Type::NES_8BITDO;
        }
    }
    
    // handle Dualshock4 separately, since it doesn't specify HID gamepad usages in the report
    if (pad.type == ::Input::Gamepad::Type::DUALSHOCK4) {

        struct ControllerData {
            u8 axis_left_x;
            u8 axis_left_y;
            u8 axis_right_x;
            u8 axis_right_y;
            u8 axis_dpad : 4;
            bool button_square : 1;
            bool button_cross : 1;
            bool button_circle : 1;
            bool button_triangle : 1;
            bool button_left_1 : 1;
            bool button_right_1 : 1;
            bool button_left_2 : 1;
            bool button_right_2 : 1;
            bool button_share : 1;
            bool button_options : 1;
            bool button_left_3 : 1;
            bool button_right_3 : 1;
            bool button_ps : 1;
            bool button_touch : 1;
            u8 sequence_number : 6;
            u8 axis_left_2;
            u8 axis_right_2;
            // for remaining buttons check https://github.com/chromium/chromium/blob/main/device/gamepad/dualshock4_controller.cc#L44
        };
        
        u8* rawdata = report;
        rawdata = &(rawdata[1]);
        if (reportID == 0x11) { // Bluetooth on Windows, adds two bytes
            rawdata = &(rawdata[2]);
        }
        
        ControllerData& controller = *(ControllerData*)rawdata;
        u16 keys = 0;
        if (controller.button_square) keys |= Pad::KeyMask::BUTTON_W;
        if (controller.button_cross) keys |= Pad::KeyMask::BUTTON_S;
        if (controller.button_circle) keys |= Pad::KeyMask::BUTTON_E;
        if (controller.button_triangle) keys |= Pad::KeyMask::BUTTON_N;
        if (controller.button_options) keys |= Pad::KeyMask::START;
        if (controller.button_share) keys |= Pad::KeyMask::SELECT;
        if (controller.button_left_3) keys |= Pad::KeyMask::LEFT_THUMB;
        if (controller.button_right_3) keys |= Pad::KeyMask::RIGHT_THUMB;
        if (controller.button_left_1) keys |= Pad::KeyMask::L1;
        if (controller.button_right_1) keys |= Pad::KeyMask::R1;
        if (controller.button_left_2) keys |= Pad::KeyMask::L2;
        if (controller.button_right_2) keys |= Pad::KeyMask::R2;
        switch (controller.axis_dpad) { // todo: simplify?
        case 0: keys |= (Pad::KeyMask::DPAD_UP); break;
        case 1: keys |= (Pad::KeyMask::DPAD_UP) | (Pad::KeyMask::DPAD_RIGHT); break;
        case 2: keys |= (Pad::KeyMask::DPAD_RIGHT); break;
        case 3: keys |= (Pad::KeyMask::DPAD_RIGHT) | (Pad::KeyMask::DPAD_DOWN); break;
        case 4: keys |= (Pad::KeyMask::DPAD_DOWN); break;
        case 5: keys |= (Pad::KeyMask::DPAD_DOWN) | (Pad::KeyMask::DPAD_LEFT); break;
        case 6: keys |= (Pad::KeyMask::DPAD_LEFT); break;
        case 7: keys |= (Pad::KeyMask::DPAD_LEFT) | (Pad::KeyMask::DPAD_UP); break;
        }
        pad.last_keys = pad.curr_keys;
        pad.curr_keys = keys;

        auto normalizeAxis = [](u8 v) {
            f32 rawAxis = (2.0f * v / 255.f) - 1.0f;
            if (Math::abs(rawAxis) < 0.05f) rawAxis = 0.f;
            return rawAxis;
            };
        pad.sliders[Pad::Sliders::AXIS_X_LEFT] = normalizeAxis(controller.axis_left_x);
        pad.sliders[Pad::Sliders::AXIS_Y_LEFT] = -normalizeAxis(controller.axis_left_y);
        pad.sliders[Pad::Sliders::AXIS_X_RIGHT] = normalizeAxis(controller.axis_right_x);
        pad.sliders[Pad::Sliders::AXIS_Y_RIGHT] = -normalizeAxis(controller.axis_right_y);
        pad.sliders[Pad::Sliders::TRIGGER_LEFT] = controller.axis_left_2 / 255.f;
        pad.sliders[Pad::Sliders::TRIGGER_RIGHT] = controller.axis_right_2 / 255.f;
    }
    else {
    }
    
    // If the pad is new, and we got button input from it, finish creation
    if (padToUpdate == ctx.padCount) {
        bool validpad = pad.curr_keys != 0;
        for (u32 slider = 0; validpad && slider < Pad::Sliders::COUNT; slider++) {
            if (Math::abs(pad.sliders[slider]) > 2.f) { // TODO: properly ignore bad inputs
                validpad = false;
            }
        }

        if (validpad) {
            ctx.padCount++; // acknowledge the current pad
            pad.deviceHandle = device;
            __DEBUGEXP(strncpy(pad.name, Pad::names[pad.type], sizeof(pad.name)));

#if __DEBUG
            u64 vendorID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)) unsignedIntegerValue];
            u64 productID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)) unsignedIntegerValue];
            Platform::debuglog("Registered new pad 0x%lx 0x%lx\n", vendorID, productID);
#endif
        }
    }
}

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
        AppDelegate* appDelegate = [[AppDelegate alloc] init];
        [NSApp setDelegate:appDelegate];
        
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
        const f32 actualWindowHeight = [[window contentView] frame].size.height;
        
        // gamepad // todo: custom allocators?
        {
            IOHIDManagerRef HIDManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
            
            IOHIDManagerRegisterInputReportCallback(HIDManager, &valueChangedGamepadCallback, &platform.input);
            // todo: handle return != kIOReturnSuccess
            IOHIDManagerOpen(HIDManager, kIOHIDOptionsTypeNone);
            IOHIDManagerSetDeviceMatchingMultiple(HIDManager, (__bridge CFArrayRef)@[
                @{@(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_GamePad)},
                @{@(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_Joystick)},
                @{@(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_MultiAxisController)},
            ]);
          
            IOHIDManagerScheduleWithRunLoop(HIDManager,
                                            CFRunLoopGetMain(),
                                            kCFRunLoopDefaultMode);
        }
        
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
                                                       untilDate:[NSDate distantPast]
                                                          inMode:NSDefaultRunLoopMode
                                                         dequeue:YES];
                            NSEventType eventType = [event type];
                            bool event_passthrough = true;
                            switch (eventType) {
                                case NSEventTypeKeyUp: {
                                    if ([event modifierFlags] & hotkeyMask) { break; } // Handle events like cmd+q etc
                                    ::Input::queueKeyUp(keyboard_queue, platform.input.keyboard.current, (KB::Keys::Enum)[event keyCode]);
                                    event_passthrough = false; // disable key error sound
                                } break;
                                case NSEventTypeKeyDown: {
                                    if ([event modifierFlags] & hotkeyMask) { break; } // Handle events like cmd+q etc
                                    ::Input::queueKeyDown(keyboard_queue, platform.input.keyboard.current, (KB::Keys::Enum)[event keyCode]);
                                    event_passthrough = false; // disable key error sound
                                } break;
                                case NSEventTypeMouseMoved:
                                case NSEventTypeLeftMouseDragged:
                                case NSEventTypeRightMouseDragged:
                                case NSEventTypeOtherMouseDragged: {
                                    // todo: won't work when hiding cursor
                                    // todo: doesn't work while pressing mouse buttons
                                    NSPoint pos = [event locationInWindow];
                                    mouse_queue.x = pos.x;
                                    mouse_queue.y = actualWindowHeight-pos.y;
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
                                default: break;
                            }
                            if (event_passthrough) { // passthrough to handle things like title bar buttons
                                [NSApp sendEvent:event];
                            }
                        } while (event);
                        [NSApp updateWindows];
                        config.quit = [appDelegate applicationHasTerminated];
                        
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
                        // todo: gamepad
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
