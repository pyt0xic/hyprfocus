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

  if (!pWindow || !m_pFlashOpacity)
    return;

  const float flashOpacity = m_pFlashOpacity->value();

  auto &alphaVar = pWindow->alpha(Desktop::View::WINDOW_ALPHA_ACTIVE);
  if (!alphaVar)
    return;

  *alphaVar = flashOpacity;
  alphaVar->setConfig(m_sFocusInAnimConfig);
  alphaVar->setCallbackOnEnd(
      [this, pWindow](CWeakPointer<CBaseAnimatedVariable> pAnim) {
        // Read decoration:active_opacity from the host (non-plugin) config.
        const auto reply =
            Config::mgr()->getConfigValue("decoration:active_opacity");
        float activeOpacity = 1.f;
        if (reply.dataptr) {
          activeOpacity = **(Config::FLOAT *const *)reply.dataptr;
        }

        if (!pWindow)
          return;
        auto &alphaVar = pWindow->alpha(Desktop::View::WINDOW_ALPHA_ACTIVE);
        if (!alphaVar)
          return;
        *alphaVar = activeOpacity;
        alphaVar->setConfig(m_sFocusOutAnimConfig);
      });
}
