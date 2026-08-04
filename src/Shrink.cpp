#include "Shrink.hpp"

#include "Log.hpp"
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/animation/AnimationManager.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

void CShrink::init(HANDLE pHandle, std::string animationName) {
  IFocusAnimation::init(pHandle, "shrink");
  m_pShrinkPercentage =
      registerFloat(pHandle, configPrefix() + "shrink_percentage", 0.5f);
}

void CShrink::setup(HANDLE pHandle, std::string animationName) {
  // No-op: values read at use time via SP<CFloatValue>.
}

void CShrink::onWindowFocus(PHLWINDOW pWindow, HANDLE pHandle) {
  if (!pWindow || !m_pShrinkPercentage)
    return;

  auto &sizeAnim = pWindow->sizeAnimation();
  auto &posAnim  = pWindow->positionAnimation();

  std::string currentAnimStyle = sizeAnim->getConfig()->internalStyle;
  hyprfocus_log(LOG, "Current animation style: {}", currentAnimStyle);
  if ((currentAnimStyle == "popout" || currentAnimStyle == "popin") &&
      sizeAnim->isBeingAnimated()) {
    hyprfocus_log(LOG, "Shrink: Window is already being animated, skipping");
    return;
  }

  IFocusAnimation::onWindowFocus(pWindow, pHandle);

  sizeAnim->setConfig(m_sFocusOutAnimConfig);
  posAnim->setConfig(m_sFocusOutAnimConfig);

  Animation::mgr()->createAnimation(1.0f, m_sShrinkAnimation,
                                    m_sFocusOutAnimConfig, pWindow,
                                    AVARDAMAGE_ENTIRE);

  const float shrinkPercentage = m_pShrinkPercentage->value();
  hyprfocus_log(LOG, "Shrink percentage: {}", shrinkPercentage);
  m_sShrinkAnimation->setValue(shrinkPercentage);

  m_sShrinkAnimation->setUpdateCallback(
      [this, pWindow](CWeakPointer<CBaseAnimatedVariable> pShrinkAnimation) {
        if (!pWindow)
          return;

        auto &sizeAnim = pWindow->sizeAnimation();
        auto &posAnim  = pWindow->positionAnimation();

        const auto GOALPOS  = posAnim->goal();
        const auto GOALSIZE = sizeAnim->goal();

        const auto *PANIMATION =
            (CAnimatedVariable<float> *)pShrinkAnimation.get();

        sizeAnim->setValue(GOALSIZE * PANIMATION->value());
        posAnim->setValue(GOALPOS + GOALSIZE / 2.f - sizeAnim->value() / 2.f);
      });

  m_sShrinkAnimation->setCallbackOnEnd(
      [this, pWindow](CWeakPointer<CBaseAnimatedVariable> pShrinkAnimation) {
        ((CAnimatedVariable<float> *)(pShrinkAnimation.get()))
            ->resetAllCallbacks();
      });
}
