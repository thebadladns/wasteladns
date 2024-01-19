#ifndef __WASTELADNS_GAMEPLAY_H__
#define __WASTELADNS_GAMEPLAY_H__

namespace Gameplay
{
    namespace Movement
    {
        struct Control {
            Vec2 localInput;
            f32 mag;
        };
        struct Controller {
            f32 speed;
        };
        struct ControlButtons {
            const ::Input::Keyboard::Keys::Enum up, down, left, right;
        };
        struct State {
            Transform transform;
            Control control;
            Controller movementController;
        };
        void process(Control& control, const ::Input::Keyboard::State& kb, const ControlButtons& buttons, const f32 dt) {

            const Vec2 prevLocalInput = control.localInput;

            Vec2 currentControl;
            currentControl.x = roundf(prevLocalInput.x);
            currentControl.y = roundf(prevLocalInput.y);

            Vec2 controlChange;
            controlChange.x = kb.pressed(buttons.right) * 1.f + kb.pressed(buttons.left) * -1.f;
            controlChange.y = kb.pressed(buttons.up) * 1.f + kb.pressed(buttons.down) * -1.f;

            Vec2 localInput;
            const bool downr = kb.down(buttons.right), downl = kb.down(buttons.left);
            if (controlChange.x != 0.f && currentControl.x != controlChange.x) {
                localInput.x = controlChange.x;
            }
            else if (downr && (currentControl.x > 0.f || !downl)) {
                localInput.x = 1.f;
            }
            else if (downl && (currentControl.x < 0.f || !downr)) {
                localInput.x = -1.f;
            }
            else {
                localInput.x = 0.f;
            }
            const bool downu = kb.down(buttons.up), downd = kb.down(buttons.down);
            if (controlChange.y != 0.f && currentControl.y != controlChange.y) {
                localInput.y = controlChange.y;
            }
            else if (downu && (currentControl.y > 0.f || !downd)) {
                localInput.y = 1.f;
            }
            else if (downd && (currentControl.y < 0.f || !downu)) {
                localInput.y = -1.f;
            }
            else {
                localInput.y = 0.f;
            }

            f32 mag = Math::mag(localInput);
            if (mag > Math::eps<f32>) {
                localInput = Math::invScale(localInput, mag);
                mag = Math::min(mag, 1.f);
            }

            control.localInput = localInput;
            control.mag = mag;
        }

        void process_cameraRelative(Transform& transform, Controller& controller, const Control& control, const Transform& camera, const f32 dt) {

            const float maxspeed = 15.f;

            Transform33 movementTransform = Math::fromUpTowardsFront(transform.up, camera.front);
            const Vec3& front = movementTransform.front;
            const Vec3& right = movementTransform.right;

            // Interpolate input angles
            float effectiveLocalInputRad = 0.f;
            const float camCurrentRad = Math::orientation(camera.front.xy);
            const float playerCurrentRad = Math::orientation(transform.front.xy);
            const float camPlayerCurrentRad = Math::subtractShort(playerCurrentRad, camCurrentRad);
            const float camInputRad = Math::orientation(control.localInput);
            const float localInputRad = Math::subtractShort(camInputRad, camPlayerCurrentRad);
            effectiveLocalInputRad = Math::eappr(effectiveLocalInputRad, localInputRad, 0.14f, dt);
            const float effectiveCamInputRad = Math::wrap(camPlayerCurrentRad + effectiveLocalInputRad);
            const Vec2 effectiveLocalInput = Math::direction(effectiveCamInputRad);

            // Default to idle decceleration
            f32 speedtimehorizon = 0.1f;
            f32 targetspeed = 0.f;
            if (control.mag > Math::eps<f32>) {
                const f32 absLocalInputRad = Math::abs(localInputRad);
                const f32 minturndeltatomove = Math::pi<f32> / 2.f;
                if (absLocalInputRad < minturndeltatomove) {
                    // Some friction on acceleration based on turn strength
                    targetspeed = maxspeed * control.mag;
                    f32 turnfrictionfactor = Math::clamp(Math::abs(localInputRad) / minturndeltatomove, 0.f, 1.f);
                    speedtimehorizon = Math::lerp(turnfrictionfactor, 0.014f, 0.3f);
                }
                else {
                    // Stop on tight turns
                    speedtimehorizon = -1.f;
                    targetspeed = 0.f;
                }
            }
            if (speedtimehorizon > 0.f) {
                controller.speed = Math::eappr(controller.speed, targetspeed, speedtimehorizon, dt);
            }
            else {
                controller.speed = 0.f;
            }

            // Facing update
            if (control.mag > Math::eps<f32>) {
                const Vec3 cameraRelativeFacingInput(effectiveLocalInput, 0.f);
                const Vec3 worldFacingInput = Math::add(Math::scale(front, cameraRelativeFacingInput.y), Math::scale(right, cameraRelativeFacingInput.x));
                Transform33 t = Math::fromUpTowardsFront(transform.up, worldFacingInput);
                transform.front = t.front;
                transform.right = t.right;
                transform.up = t.up;
            }

            // Movement update
            if (controller.speed > Math::eps<f32>) {
                Vec3 cameraRelativeMovementInput;
                if (control.mag > Math::eps<f32>) {
                    cameraRelativeMovementInput = Vec3(control.localInput, 0.f);
                }
                else {
                    cameraRelativeMovementInput = Vec3(Math::direction(camPlayerCurrentRad), 0.f);
                }
                const f32 translation = controller.speed * dt;
                const Vec3 worldMovementInput = Math::add(Math::scale(front, cameraRelativeMovementInput.y), Math::scale(right, cameraRelativeMovementInput.x));
                const Vec3 worldVelocity = Math::scale(worldMovementInput, translation);
                Vec3 pos = transform.pos;
                pos = Math::add(pos, worldVelocity);
                transform.pos = pos;
            }
        }
    }
    namespace Orbit
    {
        struct State {
            Vec3 eulers;
            Vec3 offset;
            Vec3 origin;
            float scale;
        };
        void process(State& controller, const ::Input::Mouse::State& mouse)
        {
            if (mouse.down(::Input::Mouse::Keys::BUTTON_LEFT))
            {
                constexpr f32 rotationSpeed = 0.01f;
                constexpr f32 rotationEps = 0.01f;
                f32 speedx = mouse.dx * rotationSpeed;
                f32 speedy = mouse.dy * rotationSpeed;
                if (Math::abs(speedx) > rotationEps || Math::abs(speedy) > rotationEps) {
                    controller.eulers.x = Math::wrap(controller.eulers.x + speedx);
                    controller.eulers.z = Math::wrap(controller.eulers.z - speedy);
                }
            }
            if (mouse.down(::Input::Mouse::Keys::BUTTON_RIGHT))
            {
                constexpr f32 panSpeed = 0.1f;
                constexpr f32 panEps = 0.01f;
                f32 speedx = mouse.dx * panSpeed;
                f32 speedy = mouse.dy * panSpeed;
                if (Math::abs(speedx) > panEps || Math::abs(speedy) > panEps) {
                    controller.offset.x -= speedx;
                    controller.offset.z += speedy;
                }

            }
            if (mouse.scrolldy != 0) {
                const f32 scrollSpeed = 0.1f;
                controller.scale -= mouse.scrolldy * scrollSpeed;
            }
        }
        void process(Transform& transform, const State& controller) {
            // orbit around mesh
            Transform rotationAndScale = Math::fromPositionScaleAndRotationEulers(Math::negate(controller.origin), controller.scale, controller.eulers);

            // translate to centroid 
            Transform cameraTranslation;
            Math::identity4x4(cameraTranslation);
            cameraTranslation.pos = controller.offset;

            transform = {};
            transform.matrix = Math::mult(rotationAndScale.matrix, cameraTranslation.matrix);
        }
    }
}

#endif // __WASTELADNS_GAMEPLAY_H__
