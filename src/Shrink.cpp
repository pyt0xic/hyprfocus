#include "Shrink.hpp"

#include "Log.hpp"
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/managers/animation/AnimationManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

void CShrink::init(HANDLE pHandle, std::string animationName) {
  IFocusAnimation::init(pHandle, "shrink");
  addConfigValue(pHandle, "shrink_percentage", Hyprlang::FLOAT{0.5f});
}

void CShrink::setup(HANDLE pHandle, std::string animationName) {
  // IFocusAnimation::setup(pHandle, animationName);
  // static const auto *shrinkPercentage =
  //     (Hyprlang::FLOAT *const *)(getConfigValue(pHandle, "shrink_percentage")
  //                                    ->getDataStaticPtr());
  // g_fShrinkPercentage = **shrinkPercentage;
  // hyprfocus_log(Log::INFO, "Shrink percentage: {}", g_fShrinkPercentage);
}

void CShrink::onWindowFocus(PHLWINDOW pWindow, HANDLE pHandle) {
  std::string currentAnimStyle =
      pWindow->m_realSize->getConfig()->internalStyle;
  hyprfocus_log(Log::INFO, "Current animation style: {}", currentAnimStyle);
  if ((currentAnimStyle == "popout" || currentAnimStyle == "popin") &&
      pWindow->m_realSize->isBeingAnimated()) {
    hyprfocus_log(Log::INFO, "Shrink: Window is already being animated, skipping");
    return;
  }

  IFocusAnimation::onWindowFocus(pWindow, pHandle);

  pWindow->m_realSize->setConfig(m_sFocusOutAnimConfig);
  pWindow->m_realPosition->setConfig(m_sFocusOutAnimConfig);

  g_pAnimationManager->createAnimation(1.0f, m_sShrinkAnimation, 
                                     m_sFocusOutAnimConfig, pWindow, AVARDAMAGE_ENTIRE);

  static const auto *shrinkPercentage =
      (Hyprlang::FLOAT *const *)(getConfigValue(pHandle, "shrink_percentage")
                                     ->getDataStaticPtr());
  hyprfocus_log(Log::INFO, "Shrink percentage: {}", **shrinkPercentage);
  m_sShrinkAnimation->setValue(**shrinkPercentage);

  m_sShrinkAnimation->setUpdateCallback(
      [this, pWindow](CWeakPointer<CBaseAnimatedVariable> pShrinkAnimation) {
        const auto GOALPOS = pWindow->m_realPosition->goal();
        const auto GOALSIZE = pWindow->m_realSize->goal();

        const auto *PANIMATION =
            (CAnimatedVariable<float> *)pShrinkAnimation.get();

        pWindow->m_realSize->setValue(GOALSIZE * PANIMATION->value());
        pWindow->m_realPosition->setValue(GOALPOS + GOALSIZE / 2.f -
                                           pWindow->m_realSize->value() / 2.f);
      });

  m_sShrinkAnimation->setCallbackOnEnd(
      [this, pWindow](CWeakPointer<CBaseAnimatedVariable> pShrinkAnimation) {
        ((CAnimatedVariable<float> *)(pShrinkAnimation.get()))
            ->resetAllCallbacks();
      });
}
