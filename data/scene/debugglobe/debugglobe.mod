return {
    -- DebugGlobe module
    {   
        Name = "DebugGlobe",
        Parent = "Root",
        Renderable = {
            Type = "RenderableGlobe",
            Radii = {6378137.0, 6378137.0, 6356752.314245}, -- Earth's radii
            SegmentsPerPatch = 64,
            Textures = {
                ColorTextures = {
                    {
                        Name = "VIIRS_SNPP_CorrectedReflectance_TrueColor",
                        FilePath = "map_service_configs/VIIRS_SNPP_CorrectedReflectance_TrueColor.xml"
                    },
                    {
                        Name = "MODIS_Terra_CorrectedReflectance_TrueColor",
                        FilePath = "map_service_configs/MODIS_Terra_CorrectedReflectance_TrueColor.xml"
                    },
                    {
                        Name = "ESRI Imagery World 2D",
                        FilePath = "map_service_configs/ESRI_Imagery_World_2D.wms",
                    },
                    {
                        Name = "MODIS_Water_Mask",
                        FilePath = "map_service_configs/MODIS_Water_Mask.xml"
                    },
                    {
                        Name = "Coastlines",
                        FilePath = "map_service_configs/Coastlines.xml",
                    },
                    {
                        Name = "Reference_Features",
                        FilePath = "map_service_configs/Reference_Features.xml",
                    },
                    {
                        Name = "Reference_Labels",
                        FilePath = "map_service_configs/Reference_Labels.xml",
                    },
                },
                HeightMaps = {
                    {
                        Name = "Terrain tileset",
                        FilePath = "map_service_configs/TERRAIN.wms",
                    },
                },
            },
        },
        GuiName = "/Solar/Planets/DebugGlobe"
    },

}
