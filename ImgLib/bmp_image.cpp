#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>
#include <iostream>

using namespace std;

namespace img_lib {

static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

PACKED_STRUCT_BEGIN BitmapFileHeader {
    BitmapFileHeader(int width, int height, int stride) {
        bfSize = static_cast<uint32_t>(14 + 40 + stride * height);
    }
    char bfType[2] = {'B', 'M'};
    uint32_t bfSize = {};
    uint32_t bfReserved = 0;
    uint32_t bfOffBits = 54;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    BitmapInfoHeader(int width, int height, int stride)
        : biWidth(width), biHeight(height) {
        biSizeImage = GetBMPStride(width) * height;
    }
    uint32_t biSize = 40; 
    int32_t biWidth = {};
    int32_t biHeight = {};
    uint16_t biPlanes = 1;
    uint16_t biBitCount = 24;
    uint32_t biCompression = 0;
    uint32_t biSizeImage = {};
    int32_t biXPelsPerMeter = 11811;
    int32_t biYPelsPerMeter = 11811;
    int32_t biClrUsed = 0;
    int32_t biClrImportant = 0x1000000;
}
PACKED_STRUCT_END

bool SaveBMP(const Path& file, const Image& image) {
    const int w = image.GetWidth();
    const int h = image.GetHeight();
    const int stride = GetBMPStride(w);
    BitmapFileHeader file_header { w, h, stride };
    BitmapInfoHeader info_header { w, h, stride };

    ofstream out(file, ios::binary);
    out.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    out.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));
    std::vector<char> buff(GetBMPStride(w));
    
    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(reinterpret_cast<const char*>(buff.data()), buff.size());
    }

    return out.good();
}


Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    if (!ifs.is_open()) {
        return {};
    }
    char sign[2];
    ifs.read(sign, 2);
    ifs.seekg(0, ios::beg);
    if (sign[0] != 'B' || sign[1] != 'M') {
        ifs.close();
        return {};
    }
    int w;
    int h;
    ifs.ignore(18);

    ifs.read(reinterpret_cast<char*>(&w), sizeof(w));
    ifs.read(reinterpret_cast<char*>(&h), sizeof(h));

    ifs.ignore(28);

    int stride = GetBMPStride(w);
    Image result(stride / 3, h, Color::Black());
    std::vector<char> buff(w * 3);

    for (int y = result.GetHeight() - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), stride);

        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_libe
