#include "IFocusAnimation.hpp"
#include "Globals.hpp"

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprlang.hpp>

void IFocusAnimation::init(HANDLE pHandle, std::string animationName) {
  m_szAnimationName = animationName;
  hyprfocus_log(Log::INFO, "Initializing focus animation: {}", animationName);

  HyprlandAPI::addConfigValue(pHandle, configPrefix() + "in_bezier",
                              Hyprlang::STRING{"default"});
  HyprlandAPI::addConfigValue(pHandle, configPrefix() + "out_bezier",
                              Hyprlang::STRING{"default"});

  HyprlandAPI::addConfigValue(pHandle, configPrefix() + "in_speed",
                              Hyprlang::FLOAT{1.f});
  HyprlandAPI::addConfigValue(pHandle, configPrefix() + "out_speed",
                              Hyprlang::FLOAT{5.f});

  m_sFocusInAnimConfig =
      (g_pConfigManager->getAnimationPropertyConfig("global"));
  m_sFocusInAnimConfig->internalEnabled = 1;
  m_sFocusInAnimConfig->internalStyle =
      std::string("hyprfocus_") + animationName + std::string("_in");
  m_sFocusInAnimConfig->pValues = m_sFocusInAnimConfig;

  m_sFocusOutAnimConfig =
      (g_pConfigManager->getAnimationPropertyConfig("global"));
  m_sFocusOutAnimConfig->internalEnabled = 1;
  m_sFocusOutAnimConfig->internalStyle =
      std::string("hyprfocus_") + animationName + std::string("_out");
  m_sFocusOutAnimConfig->pValues = CWeakPointer<SAnimationPropertyConfig>(m_sFocusOutAnimConfig);
}

void IFocusAnimation::setup(HANDLE pHandle, std::string animationName) {
  // hyprfocus_log(Log::INFO, "Setting up focus animation: {}", animationName);
}

void IFocusAnimation::onWindowFocus(PHLWINDOW pWindow, HANDLE pHandle) {
  hyprfocus_log(Log::INFO, "Base callback for animation: {}", m_szAnimationName);
  static const auto *inBezier =
      (Hyprlang::STRING const *)(HyprlandAPI::getConfigValue(
                                     pHandle, configPrefix() + "in_bezier")
                                     ->getDataStaticPtr());
  static const auto *inSpeed =
      (Hyprlang::FLOAT *const *)(HyprlandAPI::getConfigValue(
                                     pHandle, configPrefix() + "in_speed")
                                     ->getDataStaticPtr());
  static const auto *outBezier =
      (Hyprlang::STRING const *)(HyprlandAPI::getConfigValue(
                                     pHandle, configPrefix() + "out_bezier")
                                     ->getDataStaticPtr());
  static const auto *outSpeed =
      (Hyprlang::FLOAT *const *)(HyprlandAPI::getConfigValue(
                                     pHandle, configPrefix() + "out_speed")
                                     ->getDataStaticPtr());
  m_sFocusInAnimConfig->internalBezier = *inBezier;
  m_sFocusInAnimConfig->internalSpeed = **inSpeed;

  m_sFocusOutAnimConfig->internalBezier = *outBezier;
  m_sFocusOutAnimConfig->internalSpeed = **outSpeed;

  hyprfocus_log(Log::INFO, "In bezier: {} In speed: {} Out bezier: {} Out speed: {}",
                m_sFocusInAnimConfig->internalBezier,
                m_sFocusInAnimConfig->internalSpeed,
                m_sFocusOutAnimConfig->internalBezier,
                m_sFocusOutAnimConfig->internalSpeed);
}

void IFocusAnimation::addConfigValue(HANDLE pHandle, std::string name,
                                     Hyprlang::CConfigValue sValue) {
  HyprlandAPI::addConfigValue(pHandle, configPrefix() + name, sValue);
  hyprfocus_log(Log::INFO, "Added config value: {}", configPrefix() + name);
}

Hyprlang::CConfigValue *IFocusAnimation::getConfigValue(HANDLE pHandle,
                                                        std::string name) {
  hyprfocus_log(Log::INFO, "Getting config value: {}", configPrefix() + name);
  return HyprlandAPI::getConfigValue(pHandle, configPrefix() + name);
}
