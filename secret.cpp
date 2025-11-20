// #include <cstdint>
#include <array>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <openssl/md5.h>
// #include <curl/curl.h> // sudo apt-get install libcurl4-openssl-dev

#include <curl/curl.h>

// You extract the cert/SPKI hash and embed it in the app (the “pin”).
// SHA2-256(stdin)= 48d7c1893f99a98db921d2db62549fccd6547bf9b13b18271ad8b5481996e726

std::string host = "127.0.0.1";
uint16_t port = 1337;
std::string hsecret = "5e452bedae47caead78bd67ab22b15bc";

std::string md5_hex(const std::string& data) {
    unsigned char digest[MD5_DIGEST_LENGTH];

    // Compute MD5
    MD5(reinterpret_cast<const unsigned char*>(data.data()),
        data.size(),
        digest);

    // Convert to hex string
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        oss << std::setw(2) << static_cast<unsigned int>(digest[i]);
    }
    return oss.str();
}

static size_t writer(char *data, size_t size, size_t nmemb,
                     std::string *writerData)
{
  if(writerData == NULL)
    return 0;
  std::string hash;
  hash = md5_hex(std::string(data, size * nmemb));
    //  std::cout << "MD5 hash: " << hash << "\n";
    //  std::cout << hsecret << "\n";
    //  std::cout << "Full response: " << std::string(data, size * nmemb) << "\n";

  writerData->append(hash, 0, hash.size());
  return size * nmemb;
}

int main(){

    std::cout << "Starting client..." << std::endl;
    std::string url = "https://" + host + ":" + std::to_string(port) + "/secret";

    CURL *curl;

    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if(res)
        return (int)res;

    curl = curl_easy_init();

    std::cout << "Performing request to " << url << std::endl;
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // these two options reads from response. Writes is somewhat standart, 
    // but could be used to also implement tcp based protocols
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    // https://curl.se/libcurl/c/CURLOPT_PINNEDPUBLICKEY.html
    // base64 encoded sha256 hash of the DER-encoded public key
    curl_easy_setopt(curl, 
                     CURLOPT_PINNEDPUBLICKEY, 
                     "sha256//SNfBiT+ZqY25IdLbYlSfzNZUe/mxOxgnGti1SBmW5yY="
                    );
                    
    

    // SEND ME A SECRET
    while (true)
    {
        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
        // std::cout << "Received hash: " << response << "\n";

            if(hsecret == response) {
                std::cout << "Thanks for the secret ;)\n";
            }
        } else {
            std::cerr << "curl error: " << curl_easy_strerror(res) << "\n";
            std::cerr << "Err code: " << res << "\n";
            return 1;
        }
        response.erase(0, response.size());
        // sleep for 2 seconds
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
    }
    
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
