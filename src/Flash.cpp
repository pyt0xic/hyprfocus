#include "Flash.hpp"
#include "Globals.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/managers/animation/AnimationManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

void CFlash::init(HANDLE pHandle, std::string animationName) {
  IFocusAnimation::init(pHandle, animationName);
  addConfigValue(pHandle, "flash_opacity", Hyprlang::FLOAT{0.5f});
}

void CFlash::setup(HANDLE pHandle, std::string animationName) {
  // IFocusAnimation::setup(pHandle, animationName);
  // static const auto *flash_opacity =
  //     (Hyprlang::FLOAT *const *)(getConfigValue(pHandle, "flash_opacity")
  //                                    ->getDataStaticPtr());
  // g_fFlashOpacity = **flash_opacity;
  // hyprfocus_log(Log::INFO, "Flash opacity: {}", g_fFlashOpacity);
  // static const auto *active_opacity =
  //     (Hyprlang::FLOAT *const *)(HyprlandAPI::getConfigValue(
  //                                    pHandle, "decoration:active_opacity")
  //                                    ->getDataStaticPtr());
  // g_fActiveOpacity = **active_opacity;
  // hyprfocus_log(Log::INFO, "Active opacity: {}", g_fActiveOpacity);
}

void CFlash::onWindowFocus(PHLWINDOW pWindow, HANDLE pHandle) {
  hyprfocus_log(Log::INFO, "Flash onWindowFocus start");
  IFocusAnimation::onWindowFocus(pWindow, pHandle);

  static const auto *flash_opacity =
      (Hyprlang::FLOAT *const *)(getConfigValue(pHandle, "flash_opacity")
                                     ->getDataStaticPtr());
  *pWindow->m_alpha = **flash_opacity;
  pWindow->m_alpha->setConfig(m_sFocusInAnimConfig);
  pWindow->m_alpha->setCallbackOnEnd([this, pWindow, pHandle](CWeakPointer<CBaseAnimatedVariable> pAnim) {
    static const auto *active_opacity =
        (Hyprlang::FLOAT *const *)(HyprlandAPI::getConfigValue(
                                       pHandle, "decoration:active_opacity")
                                       ->getDataStaticPtr());
    *pWindow->m_alpha = **active_opacity;
    pWindow->m_alpha->setConfig(m_sFocusOutAnimConfig);
  });
}
