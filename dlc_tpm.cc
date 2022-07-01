// g++ --std=c++17 -O3 -I./cpr/include -I./cpr/build/_deps/curl-src/include -I./cpr/build/cpr_generated_includes dlc_tpm.cc -o dlc_tpm -L./cpr/build/lib -lcpr
// LD_LIBRARY_PATH=./c:qpr/build/lib ./dlc_tpm


#include <cpr/cpr.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::cout;
using std::endl;

std::string endpoint_url = "http://localhost:8080/invocations";
int iter = 100;

void readArgs(int argc, char** argv) {
    vector<string> args(argv + 1, argv + argc);

    // Loop over command-line args
    // (Actually I usually use an ordinary integer loop variable and compare
    // args[i] instead of *i -- don't tell anyone! ;)
    for (auto i = args.begin(); i != args.end(); ++i) {
        if (*i == "-h" || *i == "--help") {
            cout << "Syntax: dlc_tpm --iter <iter> --url <url>" << endl;
            exit(1);
        } else if (*i == "--url") {
            endpoint_url = *++i;
        } else if (*i == "--iter") {
            iter = stoi(*++i);
        }
    }
    cout << "Endpoint: " << endpoint_url << "\n";
    cout << "Iter: " << iter << "\n";
}

int main(int argc, char** argv) {
    /*cpr::Response r = cpr::Get(cpr::Url{"https://api.github.com/repos/whoshuu/cpr/contributors"},
                      cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                      cpr::Parameters{{"anon", "true"}, {"key", "value"}});
    std::cout << r.status_code << "\n";                  // 200
    r.header["content-type"];       // application/json; charset=utf-8
    std::cout << r.text << "\n";                         // JSON text string
    */
    readArgs(argc, argv);
    cpr::Url url{endpoint_url};
    std::ifstream t("test.json");
    std::stringstream buffer;
    buffer << t.rdbuf();
    t.close();
    std::string body = buffer.str();
    for(int i = 0; i < iter; i++) {
      cpr::Response resp = cpr::Post(url, cpr::Header{{"Content-Type", "application/json"}}, cpr::Body{body});
      std::cout << resp.status_code << "\n";
      //std::cout << resp.text << "\n";
    }
    std::cout << "Done" << "\n";
    return 0;
}
