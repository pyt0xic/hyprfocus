#include "Log.hpp"
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/helpers/WLClasses.hpp>

#include "Globals.hpp"
#include <hyprland/src/plugins/PluginAPI.hpp>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/animation/AnimationManager.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>
#include <hyprland/src/config/values/types/StringValue.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/desktop/DesktopTypes.hpp>

#include "Flash.hpp"
#include "Shrink.hpp"

PHLWINDOW g_pPreviouslyFocusedWindow = nullptr;
bool g_bMouseWasPressed = false;
bool g_bWorkspaceChanged = false;

std::unordered_map<std::string, std::unique_ptr<IFocusAnimation>> g_mAnimations;

// V2 plugin config values. Kept as globals so we can read them at any time.
static SP<Config::Values::CIntValue>    g_pEnabled;
static SP<Config::Values::CIntValue>    g_pAnimateWorkspaceChange;
static SP<Config::Values::CIntValue>    g_pAnimateFloating;
static SP<Config::Values::CStringValue> g_pFocusAnimation;

// Lifetime backing for the const char* name fields.
static std::vector<std::string> g_configNames;

static const char *keepName(const std::string &s) {
  g_configNames.emplace_back(s);
  return g_configNames.back().c_str();
}

static bool OnSameWorkspace(PHLWINDOW pWindow1, PHLWINDOW pWindow2) {
  if (pWindow1 == pWindow2) {
    return true;
  } else if (pWindow1 == nullptr || pWindow2 == nullptr) {
    return false;
  } else if (pWindow1->workspaceID() == pWindow2->workspaceID()) {
    return true;
  } else {
    return false;
  }
}

void flashWindow(PHLWINDOW pWindow) {
  if (!g_pFocusAnimation)
    return;
  const std::string focusAnimation = g_pFocusAnimation->value();
  hyprfocus_log(LOG, "Flashing window with animation: {}", focusAnimation);

  auto it = g_mAnimations.find(focusAnimation);
  if (it != g_mAnimations.end()) {
    hyprfocus_log(LOG, "Calling onWindowFocus for animation {}", focusAnimation);
    it->second->onWindowFocus(pWindow, PHANDLE);
  }
}

SDispatchResult flashCurrentWindow(std::string) {
  hyprfocus_log(LOG, "Flashing current window");
  SDispatchResult result = {
      .passEvent = false,
      .success = true,
  };
  if (!g_pEnabled || !g_pEnabled->value()) {
    const std::string message = "HyprFocus is disabled";
    hyprfocus_log(LOG, "HyprFocus is disabled");
    result.success = false;
    result.error = message;
    return result;
  }

  if (g_pPreviouslyFocusedWindow == nullptr) {
    const std::string message = "No previously focused window";
    hyprfocus_log(LOG, "No previously focused window");
    result.success = false;
    result.error = message;
    return result;
  }
  result.passEvent = true;
  flashWindow(g_pPreviouslyFocusedWindow);
  return result;
}

static void onActiveWindowChange(PHLWINDOW PWINDOW, Desktop::eFocusReason reason) {
  try {
    hyprfocus_log(LOG, "Active window changed");

    if (!g_pEnabled || !g_pAnimateFloating || !g_pAnimateWorkspaceChange) {
      hyprfocus_log(ERR, "hyprfocus config values not registered");
      return;
    }

    if (!g_pEnabled->value()) {
      hyprfocus_log(LOG, "HyprFocus is disabled");
      return;
    }

    if (PWINDOW == nullptr) {
      hyprfocus_log(LOG, "Window is null");
      return;
    }

    if (PWINDOW == g_pPreviouslyFocusedWindow) {
      hyprfocus_log(LOG, "Window is the same as the previous one");
      return;
    }

    if (PWINDOW->m_isFloating && !g_pAnimateFloating->value()) {
      hyprfocus_log(LOG, "Floating window, not animating");
      g_pPreviouslyFocusedWindow = PWINDOW;
      return;
    }

    if (!g_pAnimateWorkspaceChange->value() &&
        !OnSameWorkspace(PWINDOW, g_pPreviouslyFocusedWindow)) {
      hyprfocus_log(LOG, "Workspace changed, not animating");
      g_pPreviouslyFocusedWindow = PWINDOW;
      return;
    }

    flashWindow(PWINDOW);
    g_pPreviouslyFocusedWindow = PWINDOW;

  } catch (std::exception &e) {
    hyprfocus_log(ERR, "Error: {}", e.what());
  }
}

static void onMouseButton(IPointer::SButtonEvent e, Event::SCallbackInfo& info) {
  try {
    hyprfocus_log(LOG, "Mouse button state: {}", (int)e.state);
    g_bMouseWasPressed = (int)e.state == 1;

  } catch (std::exception &e) {
    hyprfocus_log(ERR, "Error: {}", e.what());
  }
}

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() { return HYPRLAND_API_VERSION; }

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
  PHANDLE = handle;

  g_pEnabled = makeShared<Config::Values::CIntValue>(
      keepName("plugin:hyprfocus:enabled"), "Enable hyprfocus",
      (Config::INTEGER)0);
  HyprlandAPI::addConfigValueV2(PHANDLE, g_pEnabled);

  g_pAnimateWorkspaceChange = makeShared<Config::Values::CIntValue>(
      keepName("plugin:hyprfocus:animate_workspacechange"),
      "Animate on workspace change", (Config::INTEGER)1);
  HyprlandAPI::addConfigValueV2(PHANDLE, g_pAnimateWorkspaceChange);

  g_pAnimateFloating = makeShared<Config::Values::CIntValue>(
      keepName("plugin:hyprfocus:animate_floating"), "Animate floating windows",
      (Config::INTEGER)1);
  HyprlandAPI::addConfigValueV2(PHANDLE, g_pAnimateFloating);

  g_pFocusAnimation = makeShared<Config::Values::CStringValue>(
      keepName("plugin:hyprfocus:focus_animation"), "Focus animation name",
      std::string("flash"));
  HyprlandAPI::addConfigValueV2(PHANDLE, g_pFocusAnimation);

  HyprlandAPI::addDispatcherV2(PHANDLE, "animatefocused", &flashCurrentWindow);

  g_mAnimations["flash"] = std::make_unique<CFlash>();
  g_mAnimations["shrink"] = std::make_unique<CShrink>();
  g_mAnimations["none"] = std::make_unique<IFocusAnimation>();

  for (auto &[name, pAnimation] : g_mAnimations) {
    pAnimation->init(PHANDLE, name);
    hyprfocus_log(LOG, "Registered animation: {}", name);
  }

  HyprlandAPI::reloadConfig();
  if (Animation::mgr())
    Animation::mgr()->tick();
  hyprfocus_log(LOG, "Reloaded config");

  // Register callbacks using new Event Bus API
  static auto P1 = Event::bus()->m_events.window.active.listen(onActiveWindowChange);
  hyprfocus_log(LOG, "Registered active window change callback");

  static auto P2 = Event::bus()->m_events.input.mouse.button.listen(onMouseButton);
  hyprfocus_log(LOG, "Registered mouse button callback");

  HyprlandAPI::reloadConfig();

  return {"hyprfocus", "animate windows on focus", "Vortex", "2.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {}
