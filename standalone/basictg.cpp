#include <iostream>
#include <tgbot/tgbot.h>


int main() {
    TgBot::Bot bot("TELEGRAM_BOT_TOKEN");

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hello, I'm your Telegram bot!");
        bot.getApi().sendMessage(message->chat->id, "this is the chat id: " + std::to_string(message->chat->id));
    });

    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "I don't understand what you're saying.");
    });

    try {
        bot.getApi().deleteWebhook();
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (const TgBot::TgException& e) {
        std::cerr << "Telegram bot error: " << e.what() << std::endl;
    }

    return 0;
}