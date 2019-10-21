#pragma once

#include "../../Eigen/Eigen/Core"


// xorshift fast random number generator
inline uint32_t xor128_02(void){
  static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
  uint32_t t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = (w ^ (w >> 19) ^ t ^ (t >> 8));
}


// arnold texture loading function
inline bool LoadTexture(const AtString path, void *pixelData){
    return AiTextureLoad(path, true, 0, pixelData);
}


struct arrayCompare{
    const float *values;
    inline arrayCompare(const float *_values) :values(_values) {}
    inline bool operator()(int _lhs, int _rhs) const{
        return values[_lhs] > values[_rhs];
    }
};


class imageData{
private:
    int x, y, nchannels;
    float *pixelData;
    float *cdfRow;
    float *cdfColumn;
    int *rowIndices;
    int *columnIndices;

public:
    imageData()
        : x(0), y(0), nchannels(0)
        , pixelData(0), cdfRow(0), cdfColumn(0)
        , rowIndices(0), columnIndices(0) {
    }

    ~imageData(){
        invalidate();
    }

    bool isValid() const{
        return (x * y * nchannels > 0 && nchannels >= 3);
    }

    void invalidate(){
        if (pixelData){
            AiAddMemUsage(-x * y * nchannels * sizeof(float), AtString("lentil"));
            AiFree(pixelData);
            pixelData = 0;
        }
        if (cdfRow){
            AiAddMemUsage(-y * sizeof(float), AtString("lentil"));
            AiFree(cdfRow);
            cdfRow = 0;
        }
        if (cdfColumn){
            AiAddMemUsage(-x * y * sizeof(float), AtString("lentil"));
            AiFree(cdfColumn);
            cdfColumn = 0;
        }
        if (rowIndices){
            AiAddMemUsage(-y * sizeof(int), AtString("lentil"));
            AiFree(rowIndices);
            rowIndices = 0;
        }
        if (columnIndices){
            AiAddMemUsage(-x * y * sizeof(int), AtString("lentil"));
            AiFree(columnIndices);
            columnIndices = 0;
        }
        x = y = nchannels = 0;
    }

    bool read(const char *bokeh_kernel_filename){
        invalidate();
        int64_t nbytes = 0;

        AiMsgInfo("[LENTIL] Reading image using Arnold API: %s", bokeh_kernel_filename);
        AtString path(bokeh_kernel_filename);

        unsigned int iw, ih, nc;
        if (!AiTextureGetResolution(path, &iw, &ih) || !AiTextureGetNumChannels(path, &nc)){ return false; }

        x = static_cast<int>(iw);
        y = static_cast<int>(ih);
        nchannels = static_cast<int>(nc);
        
        if (x != y){
            invalidate();
            AiMsgError("[LENTIL] Bokeh image is not square");
            return false;
        }

        nbytes = x * y * nchannels * sizeof(float);
        AiAddMemUsage(nbytes, AtString("lentil"));
        pixelData = (float*)AiMalloc(nbytes);

        if (!LoadTexture(path, pixelData)){
            invalidate();
            return false;
        }

        AiMsgInfo("[LENTIL] Bokeh Image Width: %d", x);
        AiMsgInfo("[LENTIL] Bokeh Image Height: %d", y);
        AiMsgInfo("[LENTIL] Bokeh Image Channels: %d", nchannels);
        AiMsgInfo("[LENTIL] Total amount of bokeh pixels to process: %d", x * y);

        // DEBUG_ONLY({
        //     // print out raw pixel data
        //     int npixels = x * y;
        //     for (int i = 0, j = 0; i < npixels; i++){
        //         std::cout << "[";
        //         for (int k = 0; k < nchannels; k++, j++){
        //             std::cout << pixelData[j];
        //             if (k + 1 < nchannels){
        //                 std::cout << ", ";
        //             }
        //         }
        //         std::cout << "]";
        //         if (i + 1 < npixels){
        //             std::cout << ", ";
        //         }
        //     }
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        // })

        bokehProbability();

        return true;
    }

    // Importance sampling
    void bokehProbability(){
        if (!isValid()){ return; }

        // initialize arrays
        int64_t nbytes = x * y * sizeof(float);
        int64_t totalTempBytes = 0;

        AiAddMemUsage(nbytes, AtString("lentil"));
        float *pixelValues = (float*)AiMalloc(nbytes);
        totalTempBytes += nbytes;

        AiAddMemUsage(nbytes, AtString("lentil"));
        float *normalizedPixelValues = (float*)AiMalloc(nbytes);
        totalTempBytes += nbytes;

        int npixels = x * y;
        int o1 = (nchannels >= 2 ? 1 : 0);
        int o2 = (nchannels >= 3 ? 2 : o1);
        float totalValue = 0.0f;

        // for every pixel, stuff going wrong here
        for (int i = 0, j = 0; i < npixels; ++i, j += nchannels){
            // store pixel value in array
            pixelValues[i] = pixelData[j] * 0.3f + pixelData[j + o1] * 0.59f + pixelData[j + o2] * 0.11f;
            totalValue += pixelValues[i];

            // DEBUG_ONLY(std::cout << "Pixel Luminance: " << i << " -> " << pixelValues[i] << std::endl);
        }

        // DEBUG_ONLY({
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "DEBUG: Total Pixel Value: " << totalValue << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        // })

        // normalize pixel values so sum = 1
        float invTotalValue = 1.0f / totalValue;
        float totalNormalizedValue = 0.0f;

        for (int i = 0; i < npixels; ++i){
            normalizedPixelValues[i] = pixelValues[i] * invTotalValue;

            totalNormalizedValue += normalizedPixelValues[i];

            // DEBUG_ONLY(std::cout << "Normalized Pixel Value: " << i << ": " << normalizedPixelValues[i] << std::endl);
        }

        // DEBUG_ONLY({
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "DEBUG: Total Normalized Pixel Value: " << totalNormalizedValue << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        // })

        // calculate sum for each row
        nbytes = y * sizeof(float);
        AiAddMemUsage(nbytes, AtString("lentil"));
        float *summedRowValues = (float*)AiMalloc(nbytes);
        totalTempBytes += nbytes;

        for (int i = 0, k = 0; i < y; ++i){

            summedRowValues[i] = 0.0f;

            for (int j = 0; j < x; ++j, ++k){

                summedRowValues[i] += normalizedPixelValues[k];
            }

            // DEBUG_ONLY(std::cout << "Summed Values row [" << i << "]: " << summedRowValues[i] << std::endl);
        }


        // DEBUG_ONLY({
        //     // calculate sum of all row values, just to debug
        //     float totalNormalizedRowValue = 0.0f;
        //     for (int i = 0; i < y; ++i){
        //         totalNormalizedRowValue += summedRowValues[i];
        //     }
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "Debug: Summed Row Value: " << totalNormalizedRowValue << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        // })


        // make array of indices
        nbytes = y * sizeof(int);
        AiAddMemUsage(nbytes, AtString("lentil"));
        rowIndices = (int*)AiMalloc(nbytes);

        for (int i = 0; i < y; ++i){
            rowIndices[i] = i;
        }

        std::sort(rowIndices, rowIndices + y, arrayCompare(summedRowValues));

        // DEBUG_ONLY({
        //     // print values
        //     for (int i = 0; i < y; ++i){
        //         std::cout << "PDF row [" << rowIndices[i] << "]: " << summedRowValues[rowIndices[i]] << std::endl;
        //     }
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        // })

        // For every row, add the sum of all previous row (cumulative distribution function)
        nbytes = y * sizeof(float);
        AiAddMemUsage(nbytes, AtString("lentil"));
        cdfRow = (float*)AiMalloc(nbytes);

        float prevVal = 0.0f;
        for (int i = 0; i < y; ++i){
            cdfRow[i] = prevVal + summedRowValues[rowIndices[i]];
            prevVal = cdfRow[i];

            // DEBUG_ONLY(std::cout << "CDF row [" << rowIndices[i] << "]: " << cdfRow[i] << std::endl);
        }

        // DEBUG_ONLY({
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        // })

        nbytes = npixels * sizeof(float);
        AiAddMemUsage(nbytes, AtString("lentil"));
        float *normalizedValuesPerRow = (float*)AiMalloc(nbytes);
        totalTempBytes += nbytes;

        // divide pixel values of each pixel by the sum of the pixel values of that row (Normalize)
        for (int r = 0, i = 0; r < y; ++r){
            for (int c = 0; c < x; ++c, ++i){
                // avoid division by 0
                if ((normalizedPixelValues[i] != 0) && (summedRowValues[r] != 0)){
                    normalizedValuesPerRow[i] = normalizedPixelValues[i] / summedRowValues[r];
                }
                else {
                    normalizedValuesPerRow[i] = 0;
                }

                // DEBUG_ONLY(std::cout << "Normalized Pixel value per row: " << i << ": " << normalizedValuesPerRow[i] << std::endl);
            }
        }

        // DEBUG_ONLY({
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        // })

        // sort column values from highest to lowest per row (probability density function)
        nbytes = npixels * sizeof(int);
        AiAddMemUsage(nbytes, AtString("lentil"));
        columnIndices = (int*)AiMalloc(nbytes);

        for (int i = 0; i < npixels; i++){
            columnIndices[i] = i;
        }

        for (int i = 0; i < npixels; i += x){
            std::sort(columnIndices + i, columnIndices + i + x, arrayCompare(normalizedValuesPerRow));
        }

        // DEBUG_ONLY({
        //     for (int i = 0; i < npixels; ++i){
        //         std::cout << "PDF column [" << columnIndices[i] << "]: " << normalizedValuesPerRow[columnIndices[i]] << std::endl;
        //     }

        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        // })

        // For every column per row, add the sum of all previous columns (cumulative distribution function)
        nbytes = npixels * sizeof(float);
        AiAddMemUsage(nbytes, AtString("lentil"));
        cdfColumn = (float*)AiMalloc(nbytes);

        for (int r = 0, i = 0; r < y; ++r){
            prevVal = 0.0f;

            for (int c = 0; c < x; ++c, ++i){
                cdfColumn[i] = prevVal + normalizedValuesPerRow[columnIndices[i]];
                prevVal = cdfColumn[i];

                // DEBUG_ONLY(std::cout << "CDF column [" << columnIndices[i] << "]: " << cdfColumn[i] << std::endl);
            }
        }

        // DEBUG_ONLY(std::cout << "----------------------------------------------" << std::endl);

        // Release and untrack memory
        AiAddMemUsage(-totalTempBytes, AtString("lentil"));
        AiFree(pixelValues);
        AiFree(normalizedPixelValues);
        AiFree(summedRowValues);
        AiFree(normalizedValuesPerRow);
    }

    // Sample image
    void bokehSample(float randomNumberRow, float randomNumberColumn, Eigen::Vector2d &lens, float stratification_r1, float stratification_r2){
        if (!isValid()){
            AiMsgWarning("Invalid bokeh image data.");
            lens(0) = 0.0;
            lens(1) = 0.0;
            return;
        }

        // print random number between 0 and 1
        // DEBUG_ONLY(std::cout << "RANDOM NUMBER ROW: " << randomNumberRow << std::endl);

        // find upper bound of random number in the array
        float *pUpperBound = std::upper_bound(cdfRow, cdfRow + y, randomNumberRow);

        int r = 0;
        pUpperBound >= (cdfRow + y) ? r = y - 1 : r = static_cast<int>(pUpperBound - cdfRow);

        // find actual pixel row
        int actualPixelRow = rowIndices[r];

        // recalculate pixel row so that the center pixel is (0,0) - might run into problems with images of dimensions like 2x2, 4x4, 6x6, etc
        int recalulatedPixelRow = actualPixelRow - ((x - 1) / 2);

        // DEBUG_ONLY({
        //     // print values
        //     std::cout << "INDEX IN CDF ROW: " << r << std::endl;
        //     std::cout << "ACTUAL PIXEL ROW: " << actualPixelRow << std::endl;
        //     std::cout << "RECALCULATED PIXEL ROW: " << recalulatedPixelRow << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "RANDOM NUMBER COLUMN: " << randomNumberColumn << std::endl;
        // })

        int startPixel = actualPixelRow * x;
        // DEBUG_ONLY(std::cout << "START PIXEL: " << startPixel << std::endl);


        // find upper bound of random number in the array
        float *pUpperBoundColumn = std::upper_bound(cdfColumn + startPixel, cdfColumn + startPixel + x, randomNumberColumn);

        int c = 0;
        pUpperBoundColumn >= cdfColumn + startPixel + x ? c = startPixel + x - 1 : c = static_cast<int>(pUpperBoundColumn - cdfColumn);

        // find actual pxel column
        int actualPixelColumn = columnIndices[c];
        int relativePixelColumn = actualPixelColumn - startPixel;
        int recalulatedPixelColumn = relativePixelColumn - ((y - 1) / 2);

        // DEBUG_ONLY({
        //     // print values
        //     std::cout << "INDEX IN CDF COLUMN: " << c << std::endl;
        //     std::cout << "ACTUAL PIXEL COLUMN: " << actualPixelColumn << std::endl;
        //     std::cout << "RELATIVE PIXEL COLUMN (starting from 0): " << relativePixelColumn << std::endl;
        //     std::cout << "RECALCULATED PIXEL COLUMN: " << recalulatedPixelColumn << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        //     std::cout << "----------------------------------------------" << std::endl;
        // })

        // to get the right image orientation, flip the x and y coordinates and then multiply the y values by -1 to flip the pixels vertically
        float flippedRow = static_cast<float>(recalulatedPixelColumn);
        float flippedColumn = recalulatedPixelRow * -1.0f;

        // send values back
        lens[0] = static_cast<float>(flippedRow) / static_cast<float>(x)* 2.0;
        lens[1] = static_cast<float>(flippedColumn) / static_cast<float>(y)* 2.0;

        stratification_r1 = (stratification_r1 - 0.5) * 2.0;
        stratification_r2 = (stratification_r2 - 0.5) * 2.0;

        lens[0] = lens(0) + stratification_r1/(double)x;
        lens[1] = lens(1) + stratification_r2/(double)y;
    }
};