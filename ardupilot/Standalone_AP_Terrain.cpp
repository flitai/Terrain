#include "Standalone_AP_Terrain.hpp"
#include <fstream>
#include <cmath>
#include <iostream>
#include <filesystem> // C++17, 用于目录遍历

// C++17 filesystem 在某些编译器上可能需要链接 -lstdc++fs
namespace fs = std::filesystem;

// --- TerrainTile 实现 ---

bool TerrainTile::load(const std::string& filepath) {
    if (!parse_filename(filepath)) {
        // 静默失败，因为在批量加载时可能有非 .DAT 文件
        return false;
    }

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size == 0 || size % 2 != 0) {
        return false;
    }

    file_data.resize(size / sizeof(int16_t));
    if (!file.read(reinterpret_cast<char*>(file_data.data()), size)) {
        return false;
    }
    return true;
}

bool TerrainTile::parse_filename(const std::string& filepath) {
    fs::path p(filepath);
    std::string filename = p.filename().string();
    
    if (filename.length() < 11 || (fs::path(filename).extension().string() != ".dat" && fs::path(filename).extension().string() != ".DAT")) {
        return false;
    }

    try {
        char ns_char = std::toupper(filename[0]);
        char ew_char = std::toupper(filename[3]);
        int lat_val = std::stoi(filename.substr(1, 2));
        int lon_val = std::stoi(filename.substr(4, 3));

        if ((ns_char != 'N' && ns_char != 'S') || (ew_char != 'E' && ew_char != 'W')) {
            return false;
        }
        tile_lat = (ns_char == 'S') ? -lat_val : lat_val;
        tile_lon = (ew_char == 'W') ? -lon_val : lon_val;
    } catch (const std::exception&) {
        return false;
    }
    return true;
}

bool TerrainTile::get_elevation_from_data(double latitude, double longitude, int16_t& elevation) const {
    double lat_frac = latitude - tile_lat;
    double lon_frac = longitude - tile_lon;
    
    int grid_y = static_cast<int>(lat_frac * GRID_DIM);
    int grid_x = static_cast<int>(lon_frac * GRID_DIM);

    if (grid_y >= GRID_DIM) grid_y = GRID_DIM - 1;
    if (grid_x >= GRID_DIM) grid_x = GRID_DIM - 1;

    size_t top_grid_offset = grid_y * GRID_DIM + grid_x;
    if (top_grid_offset >= file_data.size()) return false;
    
    int16_t top_value = file_data[top_grid_offset];

    if (top_value < TERRAIN_SUBGRID_MSG) {
        elevation = top_value;
        return true;
    } else {
        uint32_t subgrid_base_offset = top_value - TERRAIN_SUBGRID_MSG;
        double sub_lat_frac = (lat_frac * GRID_DIM) - grid_y;
        double sub_lon_frac = (lon_frac * GRID_DIM) - grid_x;
        int subgrid_y = static_cast<int>(sub_lat_frac * SUBGRID_DIM);
        int subgrid_x = static_cast<int>(sub_lon_frac * SUBGRID_DIM);

        if (subgrid_y >= SUBGRID_DIM) subgrid_y = SUBGRID_DIM - 1;
        if (subgrid_x >= SUBGRID_DIM) subgrid_x = SUBGRID_DIM - 1;

        size_t final_offset = subgrid_base_offset + subgrid_y * SUBGRID_DIM + subgrid_x;
        if (final_offset >= file_data.size()) return false;
        
        elevation = file_data[final_offset];
        return true;
    }
    return false;
}

// --- TerrainDatabase 实现 ---

TerrainDatabase::TerrainDatabase() {}

uint32_t TerrainDatabase::pack_latlon_to_key(int lat, int lon) const {
    // 将经纬度打包成一个唯一的 key
    // 纬度范围-90~90 (181个值), 经度-180~180 (361个值)
    // 为避免负数，进行偏移
    uint16_t lat_u = lat + 90;
    uint16_t lon_u = lon + 180;
    return (static_cast<uint32_t>(lat_u) << 16) | lon_u;
}

size_t TerrainDatabase::load_from_directory(const std::string& directory_path) {
    size_t count = 0;
    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {
            auto tile = std::make_unique<TerrainTile>();
            if (tile->load(entry.path().string())) {
                uint32_t key = pack_latlon_to_key(tile->tile_lat, tile->tile_lon);
                tiles[key] = std::move(tile);
                count++;
            }
        }
    }
    return count;
}

bool TerrainDatabase::get_elevation(double latitude, double longitude, int16_t& elevation) const {
    int lat_i = static_cast<int>(floor(latitude));
    int lon_i = static_cast<int>(floor(longitude));

    uint32_t key = pack_latlon_to_key(lat_i, lon_i);

    auto it = tiles.find(key);
    if (it == tiles.end()) {
        // 在数据库中没有找到对应的瓦片
        return false;
    }

    return it->second->get_elevation_from_data(latitude, longitude, elevation);
}