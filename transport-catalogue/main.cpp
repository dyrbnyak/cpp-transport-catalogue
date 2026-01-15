#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main(){
    TransportCatalogue catalogue;

    int base_request_count;
    cin >> base_request_count >> ws;

    {
        InputReader reader;
        reader.ReadRequests(cin, base_request_count);
        reader.ApplyCommands(catalogue);
    }

    detail::ProcessStatRequests(cin, catalogue, cout);
}
