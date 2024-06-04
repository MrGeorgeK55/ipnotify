#include <stdio.h>
#include <tgbot/tgbot.h>
#include <libconfig.h++>
#include <fstream>
#include <iostream>
#include <string>
// telegram
std::string telegramToken;
int chat_id;

 
void get_config()
{
    // check if config file exists
    std::ifstream file("./config.cfg");
    if (!file)
    {
        std::cerr << "Error: Unable to open config file or file dont exist\n";
        exit(1);
    }

    std::cout << "Reading config file" << std::endl;
    libconfig::Config cfg;
    try
    {
        cfg.readFile("config.cfg");
    }
    catch (const libconfig::ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getLine() << " - " << pex.getError() << std::endl;
        exit(1);
    }
    // telegram token
    std::cout << "Reading telegram token" << std::endl;
    telegramToken = cfg.lookup("telegram_token").c_str();
    chat_id = cfg.lookup("telegram_chat_id");
    std::cout << "Config file read" << std::endl;
}


int main() {
    get_config();

    TgBot::Bot bot(telegramToken);
    bot.getApi().sendMessage(chat_id, "Seems like the server restarted");
   exit(0);
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
    return 0;
}