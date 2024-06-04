#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
// working, now to add telegram bot api to send message to user
#include <tgbot/tgbot.h>
#include <libconfig.h++>

// v1.1

// variables

// telegram
std::string telegramToken;
int chat_id = 0;

// filename
std::string filename;

// get the variables from a .config file
void get_config()
{
    // check if config file exists
    std::ifstream file("config.cfg");
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

    // telegram
    std::cout << "Reading Telegram variables" << std::endl;
    telegramToken = cfg.lookup("telegram_token").c_str();
    chat_id = cfg.lookup("telegram_chat_id");
    // filename
    filename = cfg.lookup("file_name").c_str();

    std::cout << "Config file read" << std::endl;
}

// Callback function to write fetched data into a string
size_t writeCallback(void *contents, size_t size, size_t nmemb, std::string *data)
{
    data->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch external IP address using libcurl
std::string getExternalIP()
{
    std::cout << "Fetching external IP address..." << std::endl;
    std::string ipAddress;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ipAddress);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    if (ipAddress == "")
    {
        std::cerr << "Failed to fetch IP address." << std::endl;
        return std::string();
    }
    return ipAddress;
}
// Function to read stored IP address from a file
std::string readStoredIP()
{
    std::cout << "Reading stored IP address..." << std::endl;
    std::ifstream file(filename);
    std::string storedIP;
    if (file.is_open())
    {
        std::getline(file, storedIP);
        file.close();
    }
    return storedIP;
}

// Function to write new IP address to a file
void writeNewIP(const std::string &newIP)
{
    std::ofstream file(filename);
    if (file.is_open())
    {
        file << newIP;
        file.close();
    }
}

int main()
{
    // Initialize libcurl
    get_config();
    curl_global_init(CURL_GLOBAL_ALL);
    TgBot::Bot bot(telegramToken);
    // Get external IP
    std::string externalIP = getExternalIP();
    if (externalIP == "")
    {
        return 1;
    }
    // i got an malfunction where the ip read as  "error code: *""
    // already got error 522 and 524 and got it stored in the file instead of the ip
    // so if the ip starts with "error code: " then retry the function or exit
    if (externalIP.find("error code: ") == 0)
    {
        std::cout << "Error: " << externalIP << std::endl;
        return 1;
    }
    // Read stored IP
    std::string storedIP = readStoredIP();

    // Compare IPs
    if (externalIP != storedIP)
    {
        // Write new IP and notify
        writeNewIP(externalIP);
        std::cout << "New IP detected: " << externalIP << std::endl;
        std::cout << "Notifying via Telegram..." << std::endl;
        // Add code to notify here (e.g., sending an email or notification)
        bot.getApi().sendMessage(chat_id, "New IP detected: " + externalIP);
        exit(0);
    }
    else
    {
        std::cout << "No change in IP." << std::endl;
        exit(0);
    }

    // Cleanup libcurl
    curl_global_cleanup();

    try
    {
        bot.getApi().deleteWebhook();
        TgBot::TgLongPoll longPoll(bot);
        while (true)
        {
            longPoll.start();
        }
    }
    catch (const TgBot::TgException &e)
    {
        std::cerr << "Telegram bot error: " << e.what() << std::endl;
    }

    return 0;
}
