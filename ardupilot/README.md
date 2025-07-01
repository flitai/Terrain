# ArduPilot 地形数据解析工具

将 ArduPilot 固件中高度集成的 `AP_Terrain` 库改造成一个独立的、不依赖 ArduPilot 生态（如 AP\_HAL、AP\_Common 等）的 C++ 库，可以在自己的项目中使用它来解析 `.DAT` 文件。这个版本提取了 `AP_Terrain` 的核心逻辑，并用标准 C++ 库替代了所有 ArduPilot 的特定依赖。

这个改造后的库将包含两个主要部分：

1.  `TerrainTile`: 负责加载和解析ArduPilot格式的单个 `.DAT` 瓦片文件。
2.  `TerrainDatabase`: 负责管理一个地形瓦片的集合，可以从一个目录加载所有 `.DAT` 文件，并根据经纬度自动选择正确的瓦片进行查询。这更接近 `AP_Terrain` 的实际功能。

### 1. 头文件 (`Standalone_AP_Terrain.hpp`)

这个头文件定义了 `TerrainTile` 和 `TerrainDatabase` 两个类的接口。

### 2\. 实现文件 (`Standalone_AP_Terrain.cpp`)

这个文件包含了所有具体的实现逻辑。

### 3\. 如何编译和使用 (`main.cpp` 示例)

```cpp
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
```

**编译命令示例 (需要 C++17 或更高版本):**

```bash
# -lstdc++fs 可能在某些旧版本的 GCC/Clang 上需要
g++ main.cpp Standalone_AP_Terrain.cpp -o query_terrain -std=c++17 -lstdc++fs
```

### 关键改造点和设计说明

1.  **去除所有ArduPilot依赖**：代码中所有的 `AP_HAL`、`AP_Common` 等头文件和相关函数调用都已被移除。
2.  **使用标准C++库**：文件I/O使用了 `fstream`，目录遍历使用了 C++17 的 `filesystem`，数据存储使用了 `vector` 和 `map`，指针管理使用了智能指针 `unique_ptr`。
3.  **分层设计**：代码被设计为 `TerrainDatabase` (数据库) 和 `TerrainTile` (瓦片) 两层。这种设计更接近 `AP_Terrain` 的原意，使其能够轻松管理一个包含成百上千个 `.DAT` 文件的目录。
4.  **独立的错误处理**：使用 `bool` 返回值和 `std::cerr` 进行简单的错误报告，而不是依赖 ArduPilot 的日志系统。
5.  **易于集成**：您只需要将 `Standalone_AP_Terrain.hpp` 和 `Standalone_AP_Terrain.cpp` 两个文件加入到您的项目中，就可以像使用任何普通C++库一样使用它。

这个改造后的库成为一个独立的工具，可以在自己的应用程序中方便地利用 ArduPilot 的地形数据。