# How to change fonts

Use one of these functions to replace the default fonts;

VDP_loadFont(const TileSet *font, TransferMethod tm);
or 
VDP_loadFontData(const u32 *font, u16 length, TransferMethod tm);

Just remember to to set the appropriate optimizations on the tileset in rescomp or you will get messy fonts