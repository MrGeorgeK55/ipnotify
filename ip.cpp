#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
// working, now to add telegram bot api to send message to user
#include <tgbot/tgbot.h>

TgBot::Bot bot("TELEGRAM_BOT_TOKEN");
int chat_id = 123456789; // replace with your chat id


// v1.0

// Callback function to write fetched data into a string
size_t writeCallback(void *contents, size_t size, size_t nmemb, std::string *data) {
    data->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch external IP address using libcurl
std::string getExternalIP() {
    std::string ipAddress;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ipAddress);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    if (ipAddress == "") {
        std::cerr << "Failed to fetch IP address." << std::endl;
        return std::string();
    }
    return ipAddress;
}
// Function to read stored IP address from a file
std::string readStoredIP() {
    std::ifstream file("stored_ip.txt");
    std::string storedIP;
    if (file.is_open()) {
        std::getline(file, storedIP);
        file.close();
    }
    return storedIP;
}

// Function to write new IP address to a file
void writeNewIP(const std::string& newIP) {
    std::ofstream file("stored_ip.txt");
    if (file.is_open()) {
        file << newIP;
        file.close();
    }
}

int main() {
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);

    // Get external IP
    std::string externalIP = getExternalIP();
    if (externalIP == "") {
        return 1;
    }
    // Read stored IP
    std::string storedIP = readStoredIP();

    // Compare IPs
    if (externalIP != storedIP) {
        // Write new IP and notify
        writeNewIP(externalIP);
        std::cout << "New IP detected: " << externalIP << std::endl;
        // Add code to notify here (e.g., sending an email or notification)
        bot.getApi().sendMessage(chat_id, "New IP detected: " + externalIP);
        exit(0);
    } else {
        std::cout << "No change in IP." << std::endl;
        exit(0);
    }

    // Cleanup libcurl
    curl_global_cleanup();

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
