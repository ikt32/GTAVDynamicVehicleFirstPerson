#include "UI.hpp"

#include <inc/natives.h>
#include <algorithm>

namespace {
    const size_t maxStringLength = 99;
    int notificationHandle = 0;
}

void UI::Notify(const std::string& message) {
    Notify(message, true);
}

void UI::Notify(const std::string& message, bool removePrevious) {
    int* prevNotification = nullptr;
    if (removePrevious) {
        prevNotification = &notificationHandle;
    }

    if (prevNotification != nullptr && *prevNotification != 0) {
        HUD::THEFEED_REMOVE_ITEM(*prevNotification);
    }
    HUD::BEGIN_TEXT_COMMAND_THEFEED_POST("STRING");

    HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(message.c_str());

    int id = HUD::END_TEXT_COMMAND_THEFEED_POST_TICKER(false, false);
    if (prevNotification != nullptr) {
        *prevNotification = id;
    }
}

void UI::ShowHelpText(const std::string& message) {
    HUD::BEGIN_TEXT_COMMAND_DISPLAY_HELP("CELL_EMAIL_BCON");

    for (size_t i = 0; i < message.size(); i += maxStringLength) {
        size_t npos = std::min(maxStringLength, static_cast<int>(message.size()) - i);
        HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(message.substr(i, npos).c_str());
    }

    HUD::END_TEXT_COMMAND_DISPLAY_HELP(0, false, false, -1);
}

void UI::ShowText(float x, float y, float scale, const std::string& text) {
    HUD::SET_TEXT_FONT(0);
    HUD::SET_TEXT_SCALE(scale, scale);
    HUD::SET_TEXT_COLOUR(255, 255, 255, 255);
    HUD::SET_TEXT_WRAP(0.0, 1.0);
    HUD::SET_TEXT_CENTRE(0);
    HUD::SET_TEXT_OUTLINE();
    HUD::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
    HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text.c_str());
    HUD::END_TEXT_COMMAND_DISPLAY_TEXT({ x, y }, 0);
}
