import osmnx as ox
import networkx as nx
import scipy.io
import json

# 1. Скачиваем дорожный граф любого города (только дороги для автомобилей)
# Можно указать район, город или координаты
place_name = "Piedmont, California, USA"
print(f"Скачиваем граф для {place_name}...")
G = ox.graph_from_place(place_name, network_type='drive')

# 2. OSMnx возвращает мульти-ориентированный граф (MultiDiGraph), 
# так как между двумя перекрестками может быть несколько дорог.
# Для классических алгоритмов (Дейкстра) его часто упрощают до обычного DiGraph:
G_simple = ox.convert.to_digraph(G, weight="length")

print(f"Граф загружен: Узлов: {len(G_simple.nodes)}, Ребер: {len(G_simple.edges)}")

# 1. Получаем матрицу смежности из графа. 
# В качестве значений берем 'length' (длину дороги в метрах)
adj_matrix = nx.to_scipy_sparse_array(G_simple, weight='length')

# 2. Сохраняем в формат Matrix Market (.mtx)
graph_folder = "graphs"
graph_name = "road_graph"
scipy.io.mmwrite(f"{graph_folder}/{graph_name}.mtx", adj_matrix)
print(f"Граф сохранен в {graph_folder}/{graph_name}.mtx")

# 3. Сохраняем соответствие индексов матрицы и реальных координат/ID узлов
nodes_list = list(G_simple.nodes(data=True))
node_mapping = {}
for i, (node_id, data) in enumerate(nodes_list):
    node_mapping[i] = {
        "osm_id": node_id,
        "y": data['y'], # Широта (Latitude)
        "x": data['x']  # Долгота (Longitude)
    }

with open(f"{graph_folder}/{graph_name}_nodes_mapping.json", "w") as f:
    json.dump(node_mapping, f, indent=4)
print(f"Координаты узлов сохранены в {graph_folder}/{graph_name}_nodes_mapping.json")