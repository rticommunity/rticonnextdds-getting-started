#include <iostream>
#include "chocolate_factoryPlugin.h"

namespace chocolate_factory
{

inline void print_station_kind(StationKind station_kind)
{
    switch(station_kind) {
    case INVALID_CONTROLLER:
        std::cout << "INVALID_CONTROLLER";
        break;
    case SUGAR_CONTROLLER:
        std::cout << "SUGAR_CONTROLLER";
        break;
    case COCOA_BUTTER_CONTROLLER:
        std::cout << "COCOA_BUTTER_CONTROLLER";
        break;
    case VANILLA_CONTROLLER:
        std::cout << "VANILLA_CONTROLLER";
        break;
    case MILK_CONTROLLER:
        std::cout << "MILK_CONTROLLER";
        break;
    case TEMPERING_CONTROLLER:
        std::cout << "TEMPERING_CONTROLLER";
        break;
    }
}

inline void print_lot_status_kind(LotStatusKind lot_status_kind)
{
    switch(lot_status_kind)
    {
    case WAITING:
        std::cout << "WAITING";
        break;
    case PROCESSING:
        std::cout << "PROCESSING";
        break;
    case COMPLETED:
        std::cout << "COMPLETED";
        break;
    }
}

inline void print_chocolate_lot_data(const ChocolateLotState& sample)
{
    std::cout << "[" << "lot_id: " << sample.lot_id << ", " << "station: ";
    print_station_kind(sample.station);
    std::cout << ", next_station: ";
    print_station_kind(sample.next_station);
    std::cout << ", lot_status: ";
    print_lot_status_kind(sample.lot_status);
    std::cout << "]" << std::endl;
}

}