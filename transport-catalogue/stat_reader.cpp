#include <iostream>
#include <string>
#include <set>
#include <iomanip>

#include "stat_reader.h"

using namespace std;



void detail::ParseAndPrintStat(const TransportCatalogue& transport_catalogue, string_view request, ostream& output) {
    const TypeRequestAndDescription type_request_and_type_request = SeparateRequest(request);

    if(type_request_and_type_request.type_request == "Bus"){
        PrintBusInfo(output, transport_catalogue, type_request_and_type_request);
    } else if(type_request_and_type_request.type_request == "Stop"){
        PrintStopInfo(output, transport_catalogue, type_request_and_type_request);
    }
}


detail::TypeRequestAndDescription detail::SeparateRequest(string_view request){
    //Удаляем пробелы в начале и конце
    size_t start = request.find_first_not_of(" ");
    size_t end = request.find_last_not_of(" ");
    string_view trim_str = request.substr(start, end - start + 1);

    //Забираем тип запроса
    end = trim_str.find_first_of(" ");
    string_view type_request = trim_str.substr(0,end);


    //Удаляем лишнее
    trim_str = trim_str.substr(end);
    start = trim_str.find_first_not_of(" ");
    end = trim_str.find_last_not_of(" ");

    //Забирем описание запроса
    string_view description = trim_str.substr(start, end - start + 1);

    return {type_request, description};
}

void detail::ProcessStatRequests(istream& input, const TransportCatalogue& catalogue, ostream& output){
    int stat_request_count;
    cin >> stat_request_count >> ws;

    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(input, line);
        detail::ParseAndPrintStat(catalogue, line, output);
    }
}

void detail::PrintBusInfo(ostream& output, const TransportCatalogue& transport_catalogue, const TypeRequestAndDescription& type_request_and_type_request){
    if(transport_catalogue.FindBus(type_request_and_type_request.description) != nullptr){
        BusInfo bus_info = transport_catalogue.GetInfo(transport_catalogue.FindBus(type_request_and_type_request.description));
        output << "Bus " << type_request_and_type_request.description
               << ": "
               << bus_info.stops_on_rote << " stops on route, "
               << bus_info.unique_stops << " unique stops, "
               << bus_info.route_length << " route length, "
               << setprecision(6) << bus_info.curvature << " curvature"
               << '\n';

    } else{
        output << "Bus " << type_request_and_type_request.description << ": not found" << '\n';

    }

}

void detail::PrintStopInfo(ostream &output,
                           const TransportCatalogue &transport_catalogue,
                           const TypeRequestAndDescription &request) {
    const string_view stop_name = request.description;

    if (const auto* stop = transport_catalogue.FindStop(stop_name); stop != nullptr) {
        const set<string> buses = transport_catalogue.GetBusByStop(stop_name);

        if (buses.empty()) {
            output << "Stop " << stop_name << ": no buses\n";
        } else {
            output << "Stop " << stop_name << ": buses";
            for (const auto& bus : buses) {
                output << ' ' << bus;
            }
            output << '\n';
        }
    } else {
        output << "Stop " << stop_name << ": not found\n";
    }
}
