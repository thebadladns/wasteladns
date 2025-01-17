#ifndef __WASTELADNS_GAMEPLAY_H__
#define __WASTELADNS_GAMEPLAY_H__

namespace game
{
struct MovementInput{
    float2 localInput;
    f32 mag;
};
struct MovementState {
    f32 speed;
};
struct DirectionButtons {
    const ::input::keyboard::Keys::Enum up, down, left, right;
};
struct MovementController {
    Transform transform;
    MovementInput control;
    MovementState state;
};
void movementInputFromKeys(MovementInput& control, const ::input::keyboard::State& kb, const DirectionButtons& buttons) {

    const float2 prevLocalInput = control.localInput;

    float2 currentControl;
    currentControl.x = roundf(prevLocalInput.x);
    currentControl.y = roundf(prevLocalInput.y);

    float2 controlChange;
    controlChange.x = kb.pressed(buttons.right) * 1.f + kb.pressed(buttons.left) * -1.f;
    controlChange.y = kb.pressed(buttons.up) * 1.f + kb.pressed(buttons.down) * -1.f;

    float2 localInput;
    const bool downr = kb.down(buttons.right), downl = kb.down(buttons.left);
    if (controlChange.x != 0.f && currentControl.x != controlChange.x) {
        localInput.x = controlChange.x;
    } else if (downr && (currentControl.x > 0.f || !downl)) {
        localInput.x = 1.f;
    } else if (downl && (currentControl.x < 0.f || !downr)) {
        localInput.x = -1.f;
    } else {
        localInput.x = 0.f;
    }
    const bool downu = kb.down(buttons.up), downd = kb.down(buttons.down);
    if (controlChange.y != 0.f && currentControl.y != controlChange.y) {
        localInput.y = controlChange.y;
    } else if (downu && (currentControl.y > 0.f || !downd)) {
        localInput.y = 1.f;
    } else if (downd && (currentControl.y < 0.f || !downu)) {
        localInput.y = -1.f;
    } else {
        localInput.y = 0.f;
    }

    f32 mag = math::mag(localInput);
    if (mag > math::eps32) {
        localInput = math::invScale(localInput, mag);
        mag = math::min(mag, 1.f);
    }

    control.localInput = localInput;
    control.mag = mag;
}
void movementInputFromPad(MovementInput& control, const ::input::gamepad::State& pad) {

    float2 localInput;
    localInput.x = pad.sliders[::input::gamepad::Sliders::AXIS_X_LEFT];
    localInput.y = pad.sliders[::input::gamepad::Sliders::AXIS_Y_LEFT];
    f32 mag = math::mag(localInput);
    if (mag > math::eps32) {
        localInput = math::invScale(localInput, mag);
        mag = math::min(mag, 1.f);
    }
    control.localInput = localInput;
    control.mag = mag;
}
void updateMovement_cameraRelative(Transform& transform, MovementState& controller, const MovementInput& control, const Transform& camera, const f32 dt) {

    const f32 maxspeed = 15.f;

    Transform33 movementTransform = math::fromUpTowardsFront(transform.up, camera.front);
    const float3& front = movementTransform.front;
    const float3& right = movementTransform.right;

    // Interpolate input angles
    f32 effectiveLocalInputRad = 0.f;
    const f32 camCurrentRad = math::orientation(camera.front.xy);
    const f32 playerCurrentRad = math::orientation(transform.front.xy);
    const f32 camPlayerCurrentRad = math::subtractShort(playerCurrentRad, camCurrentRad);
    const f32 camInputRad = math::orientation(control.localInput);
    const f32 localInputRad = math::subtractShort(camInputRad, camPlayerCurrentRad);
    effectiveLocalInputRad = math::eappr(effectiveLocalInputRad, localInputRad, 0.14f, dt);
    const f32 effectiveCamInputRad = math::wrap(camPlayerCurrentRad + effectiveLocalInputRad);
    const float2 effectiveLocalInput = math::direction(effectiveCamInputRad);

    // Default to idle decceleration
    f32 speedtimehorizon = 0.1f;
    f32 targetspeed = 0.f;
    if (control.mag > math::eps32) {
        const f32 absLocalInputRad = math::abs(localInputRad);
        const f32 minturndeltatomove = math::pi32 / 2.f;
        if (absLocalInputRad < minturndeltatomove) {
            // Some friction on acceleration based on turn strength
            targetspeed = maxspeed * control.mag;
            f32 turnfrictionfactor = math::clamp(math::abs(localInputRad) / minturndeltatomove, 0.f, 1.f);
            speedtimehorizon = math::lerp(turnfrictionfactor, 0.014f, 0.3f);
        } else {
            // Stop on tight turns
            speedtimehorizon = -1.f;
            targetspeed = 0.f;
        }
    }
    if (speedtimehorizon > 0.f) {
        controller.speed = math::eappr(controller.speed, targetspeed, speedtimehorizon, dt);
    } else {
        controller.speed = 0.f;
    }

    // Facing update
    if (control.mag > math::eps32) {
        const float3 cameraRelativeFacingInput(effectiveLocalInput, 0.f);
        const float3 worldFacingInput = math::add(math::scale(front, cameraRelativeFacingInput.y), math::scale(right, cameraRelativeFacingInput.x));
        Transform33 t = math::fromUpTowardsFront(transform.up, worldFacingInput);
        transform.front = t.front;
        transform.right = t.right;
        transform.up = t.up;
    }

    // Movement update
    if (controller.speed > math::eps32) {
        float3 cameraRelativeMovementInput;
        if (control.mag > math::eps32) {
            cameraRelativeMovementInput = float3(control.localInput, 0.f);
        } else {
            cameraRelativeMovementInput = float3(math::direction(camPlayerCurrentRad), 0.f);
        }
        const f32 translation = controller.speed * dt;
        const float3 worldMovementInput = math::add(math::scale(front, cameraRelativeMovementInput.y), math::scale(right, cameraRelativeMovementInput.x));
        const float3 worldVelocity = math::scale(worldMovementInput, translation);
        float3 pos = transform.pos;
        pos = math::add(pos, worldVelocity);
        transform.pos = pos;
    }
}

struct OrbitInput {
    float3 eulers;
    float3 minEulers;
    float3 maxEulers;
    float3 offset;
    float3 origin;
    f32 scale;
    f32 maxScale;
    f32 minScale;
};

void orbitInputFromPad(OrbitInput& controller, const ::input::gamepad::State& pad)
{
    constexpr f32 rotationSpeed = 0.1f;
    constexpr f32 rotationEps = 0.01f;
    f32 speedx = pad.sliders[::input::gamepad::Sliders::AXIS_X_RIGHT] * rotationSpeed;
    f32 speedy = -pad.sliders[::input::gamepad::Sliders::AXIS_Y_RIGHT] * rotationSpeed;
    if (math::abs(speedx) > rotationEps || math::abs(speedy) > rotationEps) {
        controller.eulers.x = math::clamp(math::wrap(controller.eulers.x - speedy), -math::halfpi32, 0.f); // Positive 0 is below ground
        controller.eulers.z = math::wrap(controller.eulers.z - speedx);
    }
    controller.eulers = math::clamp(controller.eulers, controller.minEulers, controller.maxEulers);

    const f32 scrollSpeed = 0.1f;
    controller.scale -= scrollSpeed * pad.sliders[::input::gamepad::Sliders::TRIGGER_RIGHT];
    controller.scale += scrollSpeed * pad.sliders[::input::gamepad::Sliders::TRIGGER_LEFT];
    controller.scale = math::clamp(controller.scale, controller.minScale, controller.maxScale);
}
void orbitInputFromMouse(OrbitInput& controller, const ::input::mouse::State& mouse)
{
    if (mouse.down(::input::mouse::Keys::BUTTON_LEFT)) {
        constexpr f32 rotationSpeed = 0.01f;
        constexpr f32 rotationEps = 0.01f;
        f32 speedx = mouse.dx * rotationSpeed;
        f32 speedy = mouse.dy * rotationSpeed;
        if (math::abs(speedx) > rotationEps || math::abs(speedy) > rotationEps) {
            controller.eulers.x = math::wrap(controller.eulers.x - speedy);
            controller.eulers.z = math::wrap(controller.eulers.z - speedx);
        }
    }
    controller.eulers = math::clamp(controller.eulers, controller.minEulers, controller.maxEulers);
    if (mouse.down(::input::mouse::Keys::BUTTON_RIGHT)) {
        constexpr f32 panSpeed = 0.1f;
        constexpr f32 panEps = 0.01f;
        f32 speedx = mouse.dx * panSpeed;
        f32 speedy = mouse.dy * panSpeed;
        if (math::abs(speedx) > panEps || math::abs(speedy) > panEps) {
            controller.offset.x -= speedx;
            controller.offset.z += speedy;
        }
    }
    if (mouse.scrolldy != 0) {
        const f32 scrollSpeed = 0.1f;
        controller.scale -= mouse.scrolldy * scrollSpeed;
    }
    controller.scale = math::clamp(controller.scale, controller.minScale, controller.maxScale);
}
void updateOrbit(Transform& transform, const OrbitInput& controller) {
    float3 localOffset(controller.offset.x, controller.scale * controller.offset.y, controller.offset.z);
    transform = math::fromOffsetAndOrbit(localOffset, math::negate(controller.eulers));
}
}

#endif // __WASTELADNS_GAMEPLAY_H__
