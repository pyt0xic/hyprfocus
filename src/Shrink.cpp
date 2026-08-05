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
  if (!pWindow || !m_pShrinkPercentage || !m_sFocusOutAnimConfig)
    return;

  auto &sizeAnim = pWindow->sizeAnimation();
  auto &posAnim  = pWindow->positionAnimation();
  if (!sizeAnim || !posAnim)
    return;

  // getStyle() safely handles a missing/expired config; never deref getConfig().
  const std::string currentAnimStyle = sizeAnim->getStyle();
  hyprfocus_log(LOG, "Current animation style: {}", currentAnimStyle);
  if ((currentAnimStyle.starts_with("popin") ||
       currentAnimStyle.starts_with("popout")) &&
      sizeAnim->isBeingAnimated()) {
    hyprfocus_log(LOG, "Shrink: Window is already being animated, skipping");
    return;
  }

  IFocusAnimation::onWindowFocus(pWindow, pHandle);

  sizeAnim->setConfig(m_sFocusOutAnimConfig);
  posAnim->setConfig(m_sFocusOutAnimConfig);

  if (!Animation::mgr())
    return;

  Animation::mgr()->createAnimation(1.0f, m_sShrinkAnimation,
                                    m_sFocusOutAnimConfig, pWindow,
                                    AVARDAMAGE_ENTIRE);
  if (!m_sShrinkAnimation)
    return;

  const float shrinkPercentage = m_pShrinkPercentage->value();
  hyprfocus_log(LOG, "Shrink percentage: {}", shrinkPercentage);
  m_sShrinkAnimation->setValue(shrinkPercentage);

  // Weak window ref so callbacks do not keep a closed window alive.
  const PHLWINDOWREF windowRef = pWindow;

  m_sShrinkAnimation->setUpdateCallback(
      [windowRef](CWeakPointer<CBaseAnimatedVariable> pShrinkAnimation) {
        auto pWindow = windowRef.lock();
        if (!pWindow || !pShrinkAnimation)
          return;

        auto &sizeAnim = pWindow->sizeAnimation();
        auto &posAnim  = pWindow->positionAnimation();
        if (!sizeAnim || !posAnim)
          return;

        const auto *PANIMATION =
            dc<CAnimatedVariable<float> *>(pShrinkAnimation.get());
        if (!PANIMATION)
          return;

        const auto GOALPOS  = posAnim->goal();
        const auto GOALSIZE = sizeAnim->goal();
        const auto NEWSIZE  = GOALSIZE * PANIMATION->value();

        sizeAnim->setValue(NEWSIZE);
        posAnim->setValue(GOALPOS + GOALSIZE / 2.f - NEWSIZE / 2.f);
      });

  m_sShrinkAnimation->setCallbackOnEnd(
      [](CWeakPointer<CBaseAnimatedVariable> pShrinkAnimation) {
        if (auto *p = pShrinkAnimation.get())
          p->resetAllCallbacks();
      });
}
