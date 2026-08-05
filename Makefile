SOURCE_FILES=$(wildcard src/*.cpp)

PKG_CFLAGS=`pkg-config --cflags pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon`

all:
	$(CXX) -shared -fPIC --no-gnu-unique $(SOURCE_FILES) -o hyprfocus.so -g $(PKG_CFLAGS) -std=c++2b -O2
	strip hyprfocus.so

debug:
	$(CXX) -shared -fPIC --no-gnu-unique $(SOURCE_FILES) -o hyprfocus.so -g $(PKG_CFLAGS) -std=c++2b -O2

clean:
	rm -f ./hyprfocus.so ./tests/test_smoke

# --- Tests ---
#
# `make test` runs the static regression check and the dlopen smoke test.
# It does NOT exercise Hyprland-side callbacks — for that, reload the
# plugin inside a running session and run ./tests/live_focus_probe.sh.

tests/test_smoke: tests/test_smoke.cpp
	$(CXX) tests/test_smoke.cpp -o tests/test_smoke -std=c++2b -O0 -g

.PHONY: test test-static test-smoke
test-static:
	@./tests/check_deprecated.sh

test-smoke: hyprfocus.so tests/test_smoke
	@./tests/test_smoke ./hyprfocus.so

test: test-static test-smoke
	@echo "all offline tests passed"
