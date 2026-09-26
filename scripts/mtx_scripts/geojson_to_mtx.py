import argparse
from pathlib import Path

import geopandas as gpd
import networkx as nx
from scipy.io import mmwrite


def geojson_to_mtx(geojson_path: str, output_path: str) -> None:
    gdf = gpd.read_file(geojson_path)

    G = nx.Graph()

    for _, row in gdf.iterrows():
        coords = list(row.geometry.coords)
        for i in range(len(coords) - 1):
            G.add_edge(coords[i], coords[i + 1])

    output_path = Path(output_path)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    A = nx.to_scipy_sparse_array(G)
    mmwrite(output_path, A)


def main() -> None:
    parser = argparse.ArgumentParser(description="Convert GeoJSON roads to MTX.")
    parser.add_argument("geojson", help="Path to the .geojson file.")
    parser.add_argument(
        "--output",
        default="graphs/qgis_roads.mtx",
        help="Path to the output .mtx file.",
    )
    args = parser.parse_args()
    geojson_to_mtx(args.geojson, args.output)


if __name__ == "__main__":
    main()