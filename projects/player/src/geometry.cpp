
#include <iostream>
#include <cmath>

#include "geometry.hpp"

/*
    Vertex data:
         - Vertex coordinates
         - Texture coordinates
         - Normals
         - Color

    Indices
*/

// noiseSet -----------------------------------------------------------------

noiseSet::noiseSet( unsigned int NumOctaves,
                    float Lacunarity,
                    float Persistance,
                    float Scale,
                    float Multiplier,
                    unsigned CurveDegree,
                    float OffsetX,
                    float OffsetY,
                    FastNoiseLite::NoiseType NoiseType,
                    bool addRandomOffset,
                    unsigned int Seed )
{
    // Set values
    noiseType       = NoiseType;
    numOctaves      = NumOctaves;
    lacunarity      = Lacunarity;
    persistance     = Persistance;
    scale           = Scale;
    multiplier      = Multiplier;
    curveDegree     = CurveDegree;
    offsetX         = OffsetX;
    offsetY         = OffsetY;
    seed            = Seed;
    octaveOffsets   = new float[numOctaves][2];


    // Clamp values
    if(lacunarity < 1) lacunarity  = 1;
    if(persistance < 0) persistance = 0;
    else
        if (persistance > 1) persistance = 1;
    if(scale <= 0) scale = 0.01f;

    // Set noise type
    noise.SetNoiseType(noiseType);

    // Set offsets for each octave
    if(addRandomOffset)
    {
        std::mt19937_64 engine;
        engine.seed(seed);
        std::uniform_int_distribution<int> distribution(-10000, 10000);

        for(size_t i = 0; i < numOctaves; i++)
        {
            octaveOffsets[i][0] = distribution(engine) + offsetX;
            octaveOffsets[i][1] = distribution(engine) + offsetY;
        }
    }
    else
    {
        for(size_t i = 0; i < numOctaves; i++)
        {
            octaveOffsets[i][0] = offsetX;
            octaveOffsets[i][1] = offsetY;
        }
    }

    // Get maximum noise
    maxHeight = 0;
    float amplitude = 1;

    for(int i = 0; i < numOctaves; i++)
    {
        maxHeight += 1 * amplitude;
        amplitude *= persistance;
    }

    maxHeight *= scale * multiplier;
}

noiseSet::~noiseSet()
{
    delete[] octaveOffsets;
}

noiseSet::noiseSet(const noiseSet& obj)
{
    noise       = obj.noise;
    noiseType   = obj.noiseType;
    numOctaves  = obj.numOctaves;
    lacunarity  = obj.lacunarity;
    persistance = obj.persistance;
    scale       = obj.scale;
    multiplier  = obj.multiplier;
    curveDegree = obj.curveDegree;
    offsetX     = obj.offsetX;
    offsetY     = obj.offsetY;
    seed        = obj.seed;
    
    maxHeight     = obj.maxHeight;

    //delete[] octaveOffsets;
    octaveOffsets = new float[numOctaves][2];
    for(size_t i = 0; i < numOctaves; i++)
    {
        octaveOffsets[i][0] = obj.octaveOffsets[i][0];
        octaveOffsets[i][1] = obj.octaveOffsets[i][1];
    }
}

noiseSet& noiseSet::operator = (const noiseSet& obj)
{
    noise       = obj.noise;
    noiseType   = obj.noiseType;
    numOctaves  = obj.numOctaves;
    lacunarity  = obj.lacunarity;
    persistance = obj.persistance;
    scale       = obj.scale;
    multiplier  = obj.multiplier;
    curveDegree = obj.curveDegree;
    offsetX     = obj.offsetX;
    offsetY     = obj.offsetY;
    seed        = obj.seed;

    maxHeight     = obj.maxHeight;

    delete[] octaveOffsets;
    octaveOffsets = new float[numOctaves][2];
    for(size_t i = 0; i < numOctaves; i++)
    {
        octaveOffsets[i][0] = obj.octaveOffsets[i][0];
        octaveOffsets[i][1] = obj.octaveOffsets[i][1];
    }

    return *this;
}

bool noiseSet::operator != (const noiseSet& obj)
{
    if( noiseType   != obj.noiseType ||
        numOctaves  != obj.numOctaves ||
        lacunarity  != obj.lacunarity ||
        persistance != obj.persistance ||
        scale       != obj.scale ||
        multiplier  != obj.multiplier ||
        curveDegree != obj.curveDegree ||
        offsetX     != obj.offsetX ||
        offsetY     != obj.offsetY ||
        seed        != obj.seed )
        return true;

    return false;
}

std::ostream& operator << (std::ostream& os, const noiseSet& obj)
{
    os << "\n----------"
       << "\nNoise type: "   << obj.noiseType
       << "\nNum. octaves: " << obj.numOctaves
       << "\nLacunarity: "   << obj.lacunarity
       << "\nPersistance: "  << obj.persistance
       << "\nScale: "        << obj.scale
       << "\nMultiplier: "   << obj.multiplier
       << "\nCurve degree: " << obj.curveDegree
       << "\nOffset X: "     << obj.offsetX
       << "\nOffset Y: "     << obj.offsetY
       << "\nSeed: "         << obj.seed
       << "\nMax. height: "  << obj.maxHeight
       << std::endl;

    return os;
}

float noiseSet::GetNoise(float X, float Y)
{
    float result = 0;
    float frequency = 1, amplitude = 1;

    for(int i = 0; i < numOctaves; i++)
    {
        X = (X / scale) * frequency + octaveOffsets[i][0];
        Y = (Y / scale) * frequency + octaveOffsets[i][1];

        //float curveFactor = std::pow(basicNoise, curveDegree);
        result += ((1 + noise.GetNoise(X, Y)) / 2) * amplitude;     // noise.GetNoise() returns in range [-1, 1]. We convert it to [0, 1]

        frequency *= lacunarity;
        amplitude *= persistance;
    }

    result = result * scale * multiplier;
    result *= std::pow(result/maxHeight, curveDegree);

    return result;
}

float        noiseSet::getMaxHeight()   const { return maxHeight; };

unsigned     noiseSet::getNoiseType()   const { return noiseType; }
unsigned     noiseSet::getNumOctaves()  const { return numOctaves; }
float        noiseSet::getLacunarity()  const { return lacunarity; }
float        noiseSet::getPersistance() const { return persistance; }
float        noiseSet::getScale()       const { return scale; }
float        noiseSet::getMultiplier()  const { return multiplier; }
float        noiseSet::getCurveDegree() const { return curveDegree; }
float        noiseSet::getOffsetX()     const { return offsetX; }
float        noiseSet::getOffsetY()     const { return offsetY; }
unsigned int noiseSet::getSeed()        const { return seed; }
float*       noiseSet::getOffsets()     const { return &octaveOffsets[0][0]; }

void noiseSet::noiseTester(size_t size)
{
    float max = 0, min = 0;
    float noise;

    std::cout << "Size: " << size << std::endl;

    for(size_t i = 1; i != size; i++)
    {
        std::cout << "%: " << 100 * (float)i/size << "     \r";
        for(size_t j = 1; j != size; j++)
        {
            noise = GetNoise(i, j);
            if      (noise > max) max = noise;
            else if (noise < min) min = noise;
        }
    }
}

// terrainGenerator -----------------------------------------------------------------

terrainGenerator::terrainGenerator()
{
    numVertexX = 0;
    numVertexY = 0;
    numVertex  = 0;
    numIndices = 0;

    vertex     = nullptr;
    indices    = nullptr;
}

terrainGenerator::~terrainGenerator()
{
    if(vertex  != nullptr) delete[] vertex;
    if(indices != nullptr) delete[] indices;
}

terrainGenerator& terrainGenerator::operator = (const terrainGenerator& obj)
{
    numVertexX = obj.numVertexX;
    numVertexY = obj.numVertexY;
    numVertex  = obj.numVertex;
    numIndices = obj.numIndices;

    if(vertex != nullptr) delete[] vertex;
    vertex = new float[numVertex][8];
    for(unsigned i = 0; i < numVertex; ++i)
        for(unsigned j = 0; j < 8; ++j)
            vertex[i][j] = obj.vertex[i][j];

    if(indices != nullptr) delete[] indices;
    indices = new unsigned int[numIndices/3][3];
    for(unsigned i = 0; i < numIndices/3; ++i)
        for(unsigned j = 0; j < 3; ++j)
            indices[i][j] = obj.indices[i][j];

    return *this;
}

void terrainGenerator::computeTerrain(noiseSet &noise, float x0, float y0, float stride, unsigned numVertexX, unsigned numVertexY, float textureFactor)
{
    if (this->numVertexX != numVertexX || this->numVertexY != numVertexY)
    {
        this->numVertexX = numVertexX;
        this->numVertexY = numVertexY;
        this->numVertex  = numVertexX * numVertexY;
        this->numIndices = (numVertexX - 1) * (numVertexY - 1) * 2 * 3;

        delete[] vertex;
        vertex = new float[numVertex][8];
        delete[] indices;
        indices = new unsigned int[numIndices/3][3];
    }

    // Vertex data
    for (size_t y = 0; y < numVertexY; y++)
        for (size_t x = 0; x < numVertexX; x++)
        {
            size_t pos = getPos(x, y);

            // positions
            vertex[pos][0] = x0 + x * stride;
            vertex[pos][1] = y0 + y * stride;
            vertex[pos][2] = noise.GetNoise((float)vertex[pos][0], (float)vertex[pos][1]);

            // textures
            vertex[pos][3] = vertex[pos][0] * textureFactor;
            vertex[pos][4] = vertex[pos][1] * textureFactor;
        }

    // Normals
    computeGridNormals(vertex, numVertexX, numVertexY, stride, noise);

    // Indices
    size_t index = 0;

    for (size_t y = 0; y < numVertexY - 1; y++)
        for (size_t x = 0; x < numVertexX - 1; x++)
        {
            unsigned int pos = getPos(x, y);

            indices[index  ][0] = pos;
            indices[index  ][1] = pos + numVertexX + 1;
            indices[index++][2] = pos + numVertexX;

            indices[index  ][0] = pos;
            indices[index  ][1] = pos + 1;
            indices[index++][2] = pos + numVertexX + 1;
        }
}

void terrainGenerator::computeGridNormals(float (*vertex)[8], unsigned numVertexX, unsigned numVertexY, float stride, noiseSet &noise)
{
    // Initialize normals to 0
    unsigned numVertex = numVertexX * numVertexY;
    glm::vec3* tempNormals = new glm::vec3[numVertex];
    for (size_t i = 0; i < numVertex; i++) tempNormals[i] = glm::vec3(0.f, 0.f, 0.f);

    // Compute normals
    for (size_t y = 0; y < numVertexY - 1; y++)
        for (size_t x = 0; x < numVertexX - 1; x++)
        {
            /*
                In each iteration, we operate in each square of the grid (4 vertex):

                           Cside
                       (D)------>(C)
                        |         |
                  Dside |         | Bside
                        v         v
                       (A)------>(B)
                           Aside
             */

            // Vertex positions in the array
            size_t posA = getPos(x, y);
            size_t posB = getPos(x + 1, y);
            size_t posC = getPos(x + 1, y + 1);
            size_t posD = getPos(x, y + 1);

            // Vertex vectors
            glm::vec3 A = getVertex(posA);
            glm::vec3 B = getVertex(posB);
            glm::vec3 C = getVertex(posC);
            glm::vec3 D = getVertex(posD);

            // Vector representing each side
            glm::vec3 Aside = B - A;
            glm::vec3 Bside = B - C;
            glm::vec3 Cside = C - D;
            glm::vec3 Dside = A - D;

            // Normal computed for each vertex from the two side vectors it has attached
            glm::vec3 Anormal = glm::cross( Aside, -Dside);
            glm::vec3 Bnormal = glm::cross(-Bside, -Aside);
            glm::vec3 Cnormal = glm::cross(-Cside,  Bside);
            glm::vec3 Dnormal = glm::cross( Dside,  Cside);

            // Add to the existing normal of the vertex
            tempNormals[posA] += Anormal;
            tempNormals[posB] += Bnormal;
            tempNormals[posC] += Cnormal;
            tempNormals[posD] += Dnormal;
        }

    // Special cases: Vertex at the border
    for (size_t y = 1; y < numVertexY - 1; y++)
    {
        size_t pos;
        glm::vec3 up, down, left, right, center;

        // Left side:
        //     -Vertex vectors
        pos    = getPos(0, y);
        center = getVertex(pos);
        up     = getVertex(getPos(0, y + 1));
        down   = getVertex(getPos(0, y - 1));
        left   = glm::vec3( center.x - stride, center.y, noise.GetNoise(center.x - stride, center.y) );

        //     -Vector representing each side
        up   = up   - center;
        left = left - center;
        down = down - center;

        //     -Add normals to the existing normal of the vertex
        tempNormals[pos] += glm::cross(up, left) + glm::cross(left, down);

        // Right side:
        //     -Vertex vectors
        pos    = getPos(numVertexX-1, y);
        center = getVertex(pos);
        up     = getVertex(getPos(numVertexX-1, y + 1));
        down   = getVertex(getPos(numVertexX-1, y - 1));
        right  = glm::vec3( center.x + stride, center.y, noise.GetNoise(center.x + stride, center.y) );

        //     -Vector representing each side
        up    = up    - center;
        right = right - center;
        down  = down  - center;

        //     -Add normals to the existing normal of the vertex
        tempNormals[pos] += glm::cross(right, up) + glm::cross(down, right);
    }

    for (size_t x = 1; x < numVertexX - 1; x++)
    {
        size_t pos;
        glm::vec3 up, down, left, right, center;

        // Down side:
        //     -Vertex vectors
        pos    = getPos(x, 0);
        center = getVertex(pos);
        right  = getVertex(getPos(x + 1, 0));
        left   = getVertex(getPos(x - 1, 0));
        down   = glm::vec3( center.x, center.y - stride, noise.GetNoise(center.x, center.y - stride) );

        //     -Vector representing each side
        right = right - center;
        left  = left  - center;
        down  = down  - center;

        //     -Add normals to the existing normal of the vertex
        tempNormals[pos] += glm::cross(left, down) + glm::cross(down, right);

        // Upper side:
        //     -Vertex vectors
        pos    = getPos(x, numVertexY - 1);
        center = getVertex(pos);
        right  = getVertex(getPos(x + 1, numVertexY - 1));
        left   = getVertex(getPos(x - 1, numVertexY - 1));
        up     = glm::vec3( center.x, center.y + stride, noise.GetNoise(center.x, center.y + stride) );

        //     -Vector representing each side
        right = right - center;
        left  = left  - center;
        up    = up    - center;

        //     -Add normals to the existing normal of the vertex
        tempNormals[pos] += glm::cross(up, left) + glm::cross(right, up);
    }

    //     -Corners
    glm::vec3 topLeft, topRight, lowLeft, lowRight;
    glm::vec3 right, left, up, down;
    size_t pos;

    pos     = getPos(0, numVertexY-1);
    topLeft = getVertex(pos);
    right   = getVertex(getPos(1, numVertexY-1));
    down    = getVertex(getPos(0, numVertexY-2));
    up      = glm::vec3(topLeft.x, topLeft.y + stride, noise.GetNoise(topLeft.x, topLeft.y + stride));
    left    = glm::vec3(topLeft.x - stride, topLeft.y, noise.GetNoise(topLeft.x - stride, topLeft.y));

    right = right - topLeft;
    left  = left  - topLeft;
    up    = up    - topLeft;
    down  = down  - topLeft;

    tempNormals[pos] += glm::cross(right, up) + glm::cross(up, left) + glm::cross(left, down);

    pos      = getPos(numVertexX-1, numVertexY-1);
    topRight = getVertex(pos);
    down     = getVertex(getPos(numVertexX - 1, numVertexY-2));
    left     = getVertex(getPos(numVertexX - 2, numVertexY-1));
    right    = glm::vec3(topRight.x + stride, topRight.y, noise.GetNoise(topRight.x + stride, topRight.y));
    up       = glm::vec3(topRight.x, topRight.y + stride, noise.GetNoise(topRight.x, topRight.y + stride));


    right = right - topRight;
    left  = left  - topRight;
    up    = up    - topRight;
    down  = down  - topRight;

    tempNormals[pos] += glm::cross(down, right) + glm::cross(right, up) + glm::cross(up, left);

    pos      = getPos(0, 0);
    lowLeft  = getVertex(pos);
    right    = getVertex(getPos(1, 0));
    up       = getVertex(getPos(0, 1));
    down     = glm::vec3(lowLeft.x, lowLeft.y - stride, noise.GetNoise(lowLeft.x, lowLeft.y - stride));
    left     = glm::vec3(lowLeft.x - stride, lowLeft.y, noise.GetNoise(lowLeft.x - stride, lowLeft.y));

    right = right - lowLeft;
    left  = left  - lowLeft;
    up    = up    - lowLeft;
    down  = down  - lowLeft;

    tempNormals[pos] += glm::cross(up, left) + glm::cross(left, down) + glm::cross(down, right);

    pos      = getPos(numVertexX - 1, 0);
    lowRight = getVertex(pos);
    right    = glm::vec3(lowRight.x + stride, lowRight.y, noise.GetNoise(lowRight.x + 1, lowRight.y));
    up       = getVertex(getPos(numVertexX - 1, 1));
    down     = glm::vec3(lowRight.x, lowRight.y - stride, noise.GetNoise(lowRight.x, lowRight.y - stride));
    left     = getVertex(getPos(numVertexX - 2, 0));

    right = right - lowRight;
    left  = left  - lowRight;
    up    = up    - lowRight;
    down  = down  - lowRight;

    tempNormals[pos] += glm::cross(left, down) + glm::cross(down, right) + glm::cross(right, up);

    // Normalize the normals
    for (int i = 0; i < numVertex; i++)
    {
        tempNormals[i] = glm::normalize(tempNormals[i]);

        vertex[i][5] = tempNormals[i].x;
        vertex[i][6] = tempNormals[i].y;
        vertex[i][7] = tempNormals[i].z;
    }

    delete[] tempNormals;
}

unsigned terrainGenerator::getXside() const { return numVertexX; }
unsigned terrainGenerator::getYside() const { return numVertexY; }
unsigned terrainGenerator::getNumVertex() const { return numVertex; }
unsigned terrainGenerator::getNumIndices() const { return numIndices; }

size_t terrainGenerator::getPos(size_t x, size_t y) const { return y * numVertexX + x; }

glm::vec3 terrainGenerator::getVertex(size_t position) const
{
    return glm::vec3( vertex[position][0], vertex[position][1], vertex[position][2] );
}

// ----------------------------------------------------------------------------------

void fillAxis(float array[6][6], float sizeOfAxis)
{
    array[0][0] = 0.f;  // First vertex
    array[0][1] = 0.f;
    array[0][2] = 0.f;
    array[0][3] = 1.f;  // Color
    array[0][4] = 0.f;
    array[0][5] = 0.f;

    array[1][0] = sizeOfAxis;
    array[1][1] = 0.f;
    array[1][2] = 0.f;
    array[1][3] = 1.f;
    array[1][4] = 0.f;
    array[1][5] = 0.f;

    array[2][0] = 0.f;
    array[2][1] = 0.f;
    array[2][2] = 0.f;
    array[2][3] = 0.f;
    array[2][4] = 1.f;
    array[2][5] = 0.f;

    array[3][0] = 0.f;
    array[3][1] = sizeOfAxis;
    array[3][2] = 0.f;
    array[3][3] = 0.f;
    array[3][4] = 1.f;
    array[3][5] = 0.f;

    array[4][0] = 0.f;
    array[4][1] = 0.f;
    array[4][2] = 0.f;
    array[4][3] = 0.f;
    array[4][4] = 0.f;
    array[4][5] = 1.f;

    array[5][0] = 0.f;
    array[5][1] = 0.f;
    array[5][2] = sizeOfAxis;
    array[5][3] = 0.f;
    array[5][4] = 0.f;
    array[5][5] = 1.f;
}

void fillSea(float array[6][10], float height, float transparency, float x0, float y0, float x1, float y1)
{
    // 3 vertex position + 4 colors + 3 normal = 10 parameters

    float color[4] = { 0.1f, 0.1f, 0.8f, transparency };

    array[0][0] = x0;
    array[0][1] = y0;
    array[0][2] = height;
    array[0][3] = color[0];
    array[0][4] = color[1];
    array[0][5] = color[2];
    array[0][6] = color[3];
    array[0][7] = 0;
    array[0][8] = 0;
    array[0][9] = 1;

    array[1][0] = x1;
    array[1][1] = y0;
    array[1][2] = height;
    array[1][3] = color[0];
    array[1][4] = color[1];
    array[1][5] = color[2];
    array[1][6] = color[3];
    array[1][7] = 0;
    array[1][8] = 0;
    array[1][9] = 1;

    array[2][0] = x0;
    array[2][1] = y1;
    array[2][2] = height;
    array[2][3] = color[0];
    array[2][4] = color[1];
    array[2][5] = color[2];
    array[2][6] = color[3];
    array[2][7] = 0;
    array[2][8] = 0;
    array[2][9] = 1;

    array[3][0] = x1;
    array[3][1] = y0;
    array[3][2] = height;
    array[3][3] = color[0];
    array[3][4] = color[1];
    array[3][5] = color[2];
    array[3][6] = color[3];
    array[3][7] = 0;
    array[3][8] = 0;
    array[3][9] = 1;

    array[4][0] = x1;
    array[4][1] = y1;
    array[4][2] = height;
    array[4][3] = color[0];
    array[4][4] = color[1];
    array[4][5] = color[2];
    array[4][6] = color[3];
    array[4][7] = 0;
    array[4][8] = 0;
    array[4][9] = 1;

    array[5][0] = x0;
    array[5][1] = y1;
    array[5][2] = height;
    array[5][3] = color[0];
    array[5][4] = color[1];
    array[5][5] = color[2];
    array[5][6] = color[3];
    array[5][7] = 0;
    array[5][8] = 0;
    array[5][9] = 1;
}

void fillCubicSphere(float*** array, unsigned*** indices, float radius, unsigned numVertexPerSide, float R, float G, float B)
{
    float *** vertex = array;        //float array[6][numVertexPerSide * numVertexPerSide][6]
    unsigned *** ind = indices;      //unsigned indices[6][(numVertexPerSide-1)*(numVertexPerSide-1)*2][3]

    // Get position, color and normals

    for(size_t i = 0; i < numVertexPerSide; i++)
    {

    }




    // Get indices




}

