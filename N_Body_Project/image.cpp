#include "QuadTree.h"
#include <fstream>

class Bitmap {
    private:
        int width, height;
        vector<unsigned char> image;

        int getFileSize() const {
            return 54 + 3 * width * height; // 54 bytes for header, 3 bytes per pixel
        }

    public:
    Bitmap(int w, int h) : width(w), height(h) {
        image.resize(3 * width * height);
        fill(image.begin(), image.end(), 0); // Start with a black image
    }

    void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            // Coordinate out of bounds
            return;
        }
        int index = (x + y * width) * 3;
        image[index + 0] = b;
        image[index + 1] = g;
        image[index + 2] = r;
    }

    void drawBigDot(int centerX, int centerY, int dotSize, unsigned char r, unsigned char g, unsigned char b) {
    // Calculate half the size of the dot for offsetting the draw position
    int halfSize = dotSize / 2;
    for(int y = -halfSize; y <= halfSize; ++y) {
        for(int x = -halfSize; x <= halfSize; ++x) {
            setPixel(centerX + x, centerY + y, r, g, b);
        }
    }
}
    void save(const string& filename) {
        ofstream file(filename, ios::out | ios::binary);

        if (file) {
            unsigned char fileHeader[14] = {
                'B', 'M', // Magic identifier
                0, 0, 0, 0, // File size in bytes
                0, 0, // Reserved
                0, 0, // Reserved
                54, 0, 0, 0 // Header size
            };

            int fileSize = getFileSize();
            fileHeader[2] = (unsigned char)(fileSize      );
            fileHeader[3] = (unsigned char)(fileSize >>  8);
            fileHeader[4] = (unsigned char)(fileSize >> 16);
            fileHeader[5] = (unsigned char)(fileSize >> 24);

            unsigned char infoHeader[40] = {
                40, 0, 0, 0, // Info header size
                0, 0, 0, 0, // Width
                0, 0, 0, 0, // Height
                1, 0,       // Number of color planes
                24, 0,      // Bits per pixel
                0, 0, 0, 0, // Compression
                0, 0, 0, 0, // Image size
                0, 0, 0, 0, // Horizontal resolution
                0, 0, 0, 0, // Vertical resolution
                0, 0, 0, 0, // Colors in color table
                0, 0, 0, 0, // Important color count
            };

            infoHeader[ 4] = (unsigned char)(width      );
            infoHeader[ 5] = (unsigned char)(width >>  8);
            infoHeader[ 6] = (unsigned char)(width >> 16);
            infoHeader[ 7] = (unsigned char)(width >> 24);
            infoHeader[ 8] = (unsigned char)(height      );
            infoHeader[ 9] = (unsigned char)(height >>  8);
            infoHeader[10] = (unsigned char)(height >> 16);
            infoHeader[11] = (unsigned char)(height >> 24);

            file.write(reinterpret_cast<char*>(fileHeader), 14);
            file.write(reinterpret_cast<char*>(infoHeader), 40);
            file.write(reinterpret_cast<char*>(image.data()), image.size());

            file.close();  
        }
    }
};