#include "Flash.hpp"
#include "Globals.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

void CFlash::init(HANDLE pHandle, std::string animationName) {
  IFocusAnimation::init(pHandle, animationName);
  m_pFlashOpacity = registerFloat(pHandle, configPrefix() + "flash_opacity", 0.5f);
}

void CFlash::setup(HANDLE pHandle, std::string animationName) {
  // No-op: values are read at use time via the SP<CFloatValue> handles.
}

void CFlash::onWindowFocus(PHLWINDOW pWindow, HANDLE pHandle) {
  hyprfocus_log(LOG, "Flash onWindowFocus start");
  IFocusAnimation::onWindowFocus(pWindow, pHandle);

  if (!pWindow || !m_pFlashOpacity || !m_sFocusInAnimConfig ||
      !m_sFocusOutAnimConfig)
    return;

  auto &alphaVar = pWindow->alpha(Desktop::View::WINDOW_ALPHA_ACTIVE);
  if (!alphaVar)
    return;

  const float flashOpacity = m_pFlashOpacity->value();
  const PHLWINDOWREF windowRef = pWindow;

  // Config must be applied before assigning the goal so the flash uses our
  // bezier/speed instead of leftover fadeSwitch settings.
  alphaVar->setConfig(m_sFocusInAnimConfig);
  *alphaVar = flashOpacity;

  alphaVar->setCallbackOnEnd([windowRef](CWeakPointer<CBaseAnimatedVariable>) {
    auto pWindow = windowRef.lock();
    if (!pWindow || !Desktop::View::validMapped(pWindow))
      return;

    // Restore ACTIVE opacity via Hyprland's own decoration path (rules +
    // active/inactive/fullscreen opacity), instead of raw config double-deref.
    pWindow->updateDecorationValues();
  });
}
