
#include "world.hpp"

// BinaryKey --------------------------------------------

BinaryKey::BinaryKey(int first, int second) { x = first; y = second; }

void BinaryKey::set(int first, int second) { x = first; y = second; }

bool BinaryKey::operator <( const BinaryKey &rhs ) const
{
    if( x < rhs.x) return true;
    if( x > rhs.x) return false;
    if( y < rhs.y) return true;
    if( y > rhs.y) return false;
    return false;
}

bool BinaryKey::operator ==( const BinaryKey &rhs ) const
{
    if(x == rhs.x && y == rhs.y) return true;
    return false;
}

// terrainChunks --------------------------------------------

int terrainChunks::getNumVertex()   { return vertexPerSide * vertexPerSide; }
int terrainChunks::getNumIndices()  { return (vertexPerSide-1) * (vertexPerSide-1) * 2 * 3; }
int terrainChunks::getMaxViewDist() { return maxViewDist; }

terrainChunks::terrainChunks(noiseSet noise, float maxViewDist, float chunkSize, unsigned vertexPerSide)
{
    updateTerrainParameters(noise, maxViewDist, chunkSize, vertexPerSide);
}

terrainChunks::~terrainChunks() { }

void terrainChunks::updateVisibleChunks(glm::vec3 viewerPos)
{
    // Viewer coordinates in chunk coordinates (origin at chunk (0, 0))
    int viewerChunkCoord_X = std::round(viewerPos.x / chunkSize);
    int viewerChunkCoord_Y = std::round(viewerPos.y / chunkSize);
    int viewerChunkCoord_Z = std::round(viewerPos.z / chunkSize);

    // Delete chunks out of range    
    typedef std::map<BinaryKey, terrainGenerator> dictionary;

    std::vector<dictionary::const_iterator> toErase;

    for(dictionary::const_iterator it = chunkDict.begin(); it != chunkDict.end(); ++it)
    {
        BinaryKey key = it->first;

        // Circular area
        float sqrtDistInChunks = (key.x-viewerChunkCoord_X)*(key.x-viewerChunkCoord_X) + (key.y-viewerChunkCoord_Y)*(key.y-viewerChunkCoord_Y);
        float maxSqrtDistInChunks = (chunksVisible + 0.5) * (chunksVisible + 0.5);

        if(sqrtDistInChunks > maxSqrtDistInChunks)
            toErase.push_back(it);

        // Square area
        //if(key.x < viewerChunkCoord_X - chunksVisible || key.x > viewerChunkCoord_X + chunksVisible ||
        //   key.y < viewerChunkCoord_Y - chunksVisible || key.y > viewerChunkCoord_Y + chunksVisible )
        //    toErase.push_back(it);
    }

    for(size_t i = 0; i < toErase.size(); ++i)
        chunkDict.erase(toErase[i]);

    // Save chunks in range
    terrainGenerator generator;

    for(int yOffset = viewerChunkCoord_Y - chunksVisible; yOffset <= viewerChunkCoord_Y + chunksVisible; yOffset++)
        for(int xOffset = viewerChunkCoord_X - chunksVisible; xOffset <= viewerChunkCoord_X + chunksVisible; xOffset++)
        {
            float sqrtDistInChunks = (xOffset-viewerChunkCoord_X)*(xOffset-viewerChunkCoord_X) + (yOffset-viewerChunkCoord_Y)*(yOffset-viewerChunkCoord_Y);
            float maxSqrtDistInChunks = (chunksVisible + 0.5) * (chunksVisible + 0.5);
            if(sqrtDistInChunks > maxSqrtDistInChunks)
                continue;                                       // if out of range, skip iteration

            BinaryKey chunkCoord(xOffset, yOffset);             // chunk name (key)

            if(chunkDict.find(chunkCoord) == chunkDict.end())   // if(chunk doesn't exist in chunkDict) add new chunk
            {
                generator.computeTerrain( noise,
                                          xOffset * chunkSize,
                                          yOffset * chunkSize,
                                          chunkSize/(vertexPerSide-1),
                                          vertexPerSide,
                                          vertexPerSide  );

                //chunkDict.insert( {chunkCoord, generator} );  // Doesn't require default constructor. If element already exists, insert does nothing
                chunkDict[chunkCoord] = generator;              // Requires default constructor
            }
        }

}

void terrainChunks::updateTerrainParameters(noiseSet noise, float maxViewDist, float chunkSize, unsigned vertexPerSide)
{
    chunkDict.clear();

    this->noise         = noise;
    this->maxViewDist   = maxViewDist;
    this->chunkSize     = chunkSize;
    this->chunksVisible = std::round(maxViewDist/chunkSize);
    this->vertexPerSide = vertexPerSide;
}

void terrainChunks::setNoise(noiseSet newNoise)
{
    this->noise = newNoise;
}
