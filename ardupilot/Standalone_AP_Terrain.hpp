#ifndef STANDALONE_AP_TERRAIN_HPP
#define STANDALONE_AP_TERRAIN_HPP

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <memory>

// 前向声明
class TerrainTile;

/**
 * @class TerrainDatabase
 * @brief 管理一个地形瓦片数据库，是与外部交互的主要接口。
 *
 * 这个类可以从一个目录加载所有的 .DAT 文件，并提供一个统一的接口
 * 来查询任何经纬度的高程。
 */
class TerrainDatabase {
public:
    TerrainDatabase();

    /**
     * @brief 从指定目录加载所有 .DAT 地形文件。
     * @param directory_path 包含 .DAT 文件的目录路径。
     * @return 加载成功的瓦片数量。
     */
    size_t load_from_directory(const std::string& directory_path);

    /**
     * @brief 根据经纬度获取高程。
     *
     * 该函数会自动计算坐标所属的瓦片，并从已加载的数据库中查找。
     * @param latitude 纬度 (-90 to 90)。
     * @param longitude 经度 (-180 to 180)。
     * @param elevation[out] 用于存储查询到的高程值（米）。
     * @return 如果找到了对应的瓦片且查询成功，返回 true。
     */
    bool get_elevation(double latitude, double longitude, int16_t& elevation) const;

private:
    // 使用一个 map 来存储所有加载的瓦片
    // key 是一个唯一的整数，由经纬度计算而来，方便快速查找
    std::map<uint32_t, std::unique_ptr<TerrainTile>> tiles;

    /**
     * @brief 将经纬度打包成一个唯一的 32 位整数 key。
     * @param lat 整数纬度。
     * @param lon 整数经度。
     * @return 唯一的 key。
     */
    uint32_t pack_latlon_to_key(int lat, int lon) const;
};


/**
 * @class TerrainTile
 * @brief 负责处理单个 .DAT 文件的加载和解析。
 *
 * 这个类由 TerrainDatabase 内部使用。
 */
class TerrainTile {
public:
    // 允许 TerrainDatabase 访问私有成员
    friend class TerrainDatabase;

    TerrainTile() = default;

    /**
     * @brief 加载一个 .DAT 文件。
     * @param filepath 文件路径。
     * @return 加载成功返回 true。
     */
    bool load(const std::string& filepath);

private:
    // .DAT 文件格式的常量定义
    static const uint16_t GRID_DIM = 36;
    static const uint16_t SUBGRID_DIM = 10;
    static const int16_t TERRAIN_SUBGRID_MSG = 32000;

    bool parse_filename(const std::string& filepath);
    bool get_elevation_from_data(double latitude, double longitude, int16_t& elevation) const;

    int tile_lat{0};
    int tile_lon{0};
    std::vector<int16_t> file_data;
};

#endif // STANDALONE_AP_TERRAIN_HPP