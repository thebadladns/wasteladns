#ifndef __WASTELADNS_INPUT_H__
#define __WASTELADNS_INPUT_H__

namespace Input {

	namespace DebugSet {
		enum Keys : u8 {
			ESC = 1 << 0
			, EXIT = ESC
			, COUNT = 2
			, CLEAR = 0
		};
		typedef s32 Mapping[Keys::COUNT];
		extern const Mapping mapping = {
			GLFW_KEY_ESCAPE
		};
	};

	template<typename _Set, const s32* _mapping>
	struct DigitalState {
		bool down(_Set key) const {
			return (current & key) != 0;
		}
		bool up(_Set key) const {
			return (current & key) == 0;
		}
		bool released(_Set key) const {
			return (current & key) == 0 && (last & key) != 0;
		}
		bool pressed(_Set key) const {
			return (current & key) != 0 && (last & key) == 0;
		}
		void pollState(GLFWwindow* window) {
			last = current;
			current = _Set::CLEAR;
			for (int i = 0; i < _Set::COUNT; i++) {
				s32 keyState = glfwGetKey(window, _mapping[i]);
				s32 keyOn = (keyState == GLFW_PRESS) || (keyState == GLFW_REPEAT);
				current = (_Set)(current | (keyOn << i));
			}
		}

		_Set last;
		_Set current;
	};

	struct KeyboardState {
		bool down(s32 key) {
			return current[key] != 0;
		}
		bool up(s32 key) {
			return current[key] == 0;
		}
		bool released(s32 key) {
			return current[key] == 0 && last[key] != 0;
		}
		bool pressed(s32 key) {
			return current[key] != 0 && last[key] == 0;
		}
		void pollState(GLFWwindow* window) {
			memcpy(last, current, GLFW_KEY_LAST);
			memset(current, 0, GLFW_KEY_LAST);
			for (int i = 0; i < GLFW_KEY_LAST; i++) {
				current[i] = glfwGetKey(window, i);
			}
		}

		u8 last[GLFW_KEY_LAST];
		u8 current[GLFW_KEY_LAST];
	};
};

#endif // __WASTELADNS_INPUT_H__