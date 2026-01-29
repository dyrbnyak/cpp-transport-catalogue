#include <iostream>
#include <string>
#include <chrono>

#include "input_reader.h"
#include "stat_reader.h"

class LogDuration {
public:
    // заменим имя типа std::chrono::steady_clock
    // с помощью using для удобства
    using Clock = std::chrono::steady_clock;

    LogDuration(std::string name_op): name_op_{name_op} {
    }

    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;

        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        std::cerr << name_op_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }

private:
    const Clock::time_point start_time_ = Clock::now();
    std::string name_op_;
};

using namespace std;

int main(){
    TransportCatalogue catalogue;

    int base_request_count;
    cin >> base_request_count >> ws;

    {

        InputReader reader;
        {
            LogDuration guard("Test_New_logic");
            reader.ReadRequests(cin, base_request_count);
            reader.ApplyCommands(catalogue);
        }
    }

    detail::ProcessStatRequests(cin, catalogue, cout);
}
