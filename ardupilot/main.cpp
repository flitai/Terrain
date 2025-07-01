#include "Standalone_AP_Terrain.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_with_dat_files>" << std::endl;
        return 1;
    }

    std::string terrain_dir = argv[1];
    TerrainDatabase db;

    std::cout << "Loading terrain database from: " << terrain_dir << std::endl;
    size_t loaded_count = db.load_from_directory(terrain_dir);
    std::cout << "Loaded " << loaded_count << " terrain tiles." << std::endl;
    
    if (loaded_count == 0) {
        return 1;
    }
    
    // --- 查询示例 ---
    struct QueryPoint {
        std::string name;
        double lat;
        double lon;
    };

    std::vector<QueryPoint> points = {
        {"Mount Everest area", 27.9881, 86.9250},
        {"Kathmandu", 27.7172, 85.3240},
        {"Invalid point (Ocean)", 20.0, 90.0}
    };

    for (const auto& p : points) {
        int16_t elevation = 0;
        std::cout << "\nQuerying for " << p.name << " (" << p.lat << ", " << p.lon << ")" << std::endl;
        if (db.get_elevation(p.lat, p.lon, elevation)) {
            std::cout << "  -> Success! Elevation: " << elevation << " meters." << std::endl;
        } else {
            std::cout << "  -> Failed. No terrain data available for this location." << std::endl;
        }
    }

    return 0;
}