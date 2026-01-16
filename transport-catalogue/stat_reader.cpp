#include <iostream>
#include <string>
#include <set>
#include <algorithm>

#include "stat_reader.h"

using namespace std;



void detail::ParseAndPrintStat(const TransportCatalogue& transport_catalogue, string_view request, ostream& output) {
    TypeRequestAndDescription type_request_and_type_request = SeparateRequest(request);

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
    if(transport_catalogue.FindBus(type_request_and_type_request.description)){
        BusInfo bus_info = transport_catalogue.GetInfo(transport_catalogue.FindBus(type_request_and_type_request.description));
        output << "Bus " << type_request_and_type_request.description
               << ": "
               << bus_info.stops_on_rote << " stops on route, "
               << bus_info.unique_stops << " unique stops, "
               << bus_info.route_length << " route length"
               << endl;

    } else{
        output << "Bus " << type_request_and_type_request.description << ": not found" << endl;

    }

}

void detail::PrintStopInfo(ostream &output, const TransportCatalogue &transport_catalogue, const TypeRequestAndDescription &type_request_and_type_request){
    if(transport_catalogue.FindStop(type_request_and_type_request.description)){
        set<string> name_bus = transport_catalogue.GetBusByStop(type_request_and_type_request.description);

        if(name_bus.empty()){
            output << "Stop " << type_request_and_type_request.description << ": no buses" << endl;
            return;
        } else {
            output << "Stop " << type_request_and_type_request.description << ": buses";
            for(const auto& bus : name_bus){
                output << " " << bus;
            }
            output << endl;
        }

    } else{
        output << "Stop " << type_request_and_type_request.description << ": not found" << endl;

    }
}
