import geopandas as gpd
import pandas as pd
import numpy as np

# --- Carga y reproyección ---
gdf = gpd.read_file("bolivia_shp/gis_osm_roads_free_1.shp").to_crs(epsg=32719)

# --- Filtrar solo LineStrings y eliminar valores nulos ---
gdf = gdf[gdf.geometry.geom_type == "LineString"].copy()
gdf = gdf[gdf.geometry.notna()].copy()

# --- Eliminar duplicados basados en osm_id ---
# Conservamos solo la primera aparición de cada identificador vial
gdf = gdf.drop_duplicates(subset=["osm_id"], keep="first").copy()

# --- Extraer coordenadas de inicio y fin vectorialmente ---
start_coords = gdf.geometry.apply(lambda g: g.coords[0])
end_coords   = gdf.geometry.apply(lambda g: g.coords[-1])

# --- Construir nodos únicos sin loop ---
all_coords = pd.Series(
    pd.concat([start_coords, end_coords]).unique()
)
coord_to_id = {coord: i for i, coord in enumerate(all_coords)}

# --- Construir edges vectorialmente ---
edges = pd.DataFrame({
    "osm_id"    : gdf["osm_id"].values,
    "from_id"   : start_coords.map(coord_to_id).values,
    "to_id"     : end_coords.map(coord_to_id).values,
    "distance_m": gdf.geometry.length.round(2).values,
    "fclass"    : gdf["fclass"].values,
    "oneway"    : (gdf["oneway"].astype(str) == "T").astype(int).values,
    "maxspeed"  : gdf["maxspeed"].values,
})

# --- Construir nodes vectorialmente (Orden corregido a lat, lon) ---
coords_array = np.array(list(coord_to_id.keys()))
nodes = pd.DataFrame({
    "node_id": list(coord_to_id.values()),
    "lat"    : coords_array[:, 1],
    "lon"    : coords_array[:, 0],
})

# --- Escribir CSVs UNA SOLA VEZ al final ---
nodes.to_csv("nodes.csv", index=False)
edges.to_csv("edges.csv", index=False)

print(f"Nodos: {len(nodes)}, Aristas: {len(edges)}")
