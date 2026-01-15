#include <iostream>
#include <string>
#include <set>
#include <algorithm>

#include "stat_reader.h"

using namespace std;



void detail::ParseAndPrintStat(const TransportCatalogue& transport_catalogue, string_view request, ostream& output) {

    pair<string_view, string_view> id_and_type_request = SeparateRequest(request);


    if(id_and_type_request.first == "Bus"){
        if(transport_catalogue.FindBus(id_and_type_request.second)){
            BusInfo bus_info = transport_catalogue.GetInfo(id_and_type_request.second);
            output << "Bus " << id_and_type_request.second
                   << ": "
                   << bus_info.stops_on_rote << " stops on route, "
                   << bus_info.unique_stops << " unique stops, "
                   << bus_info.route_length << " route length"
                   << endl;

        } else{
            output << "Bus " << id_and_type_request.second << ": not found" << endl;

        }

    } else if(id_and_type_request.first == "Stop"){
        if(transport_catalogue.FindStop(id_and_type_request.second)){
            set<string> name_bus = transport_catalogue.GetStopsByBus(id_and_type_request.second);

            if(name_bus.empty()){
                output << "Stop " << id_and_type_request.second << ": no buses" << endl;
                return;
            } else {
                output << "Stop " << id_and_type_request.second << ": buses";
                for(const auto& bus : name_bus){
                    output << " " << bus;
                }
                output << endl;
            }

        } else{
            output << "Stop " << id_and_type_request.second << ": not found" << endl;

        }
    }


}



pair<string_view, string_view> detail::SeparateRequest(string_view request){
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
