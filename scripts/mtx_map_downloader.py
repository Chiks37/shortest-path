import osmnx as ox
import networkx as nx
import scipy.io
import json
import os

def download_graph(place_name):
    """
    Скачиваем дорожный граф города (только дороги для автомобилей)
    и упрощаем его до DiGraph.
    """
    print(f"Скачиваем граф для {place_name}...")
    G = ox.graph_from_place(place_name, network_type='drive')
    
    # OSMnx возвращает мульти-ориентированный граф (MultiDiGraph), 
    # так как между двумя перекрестками может быть несколько дорог.
    # Для классических алгоритмов (Дейкстра) его часто упрощают до обычного DiGraph:
    G_simple = ox.convert.to_digraph(G, weight="length")
    
    print(f"Граф загружен: Узлов: {len(G_simple.nodes)}, Ребер: {len(G_simple.edges)}")
    return G_simple


def save_graph_data(G_simple, graph_folder, graph_name):
    """
    Сохраняем матрицу смежности графа в .mtx и соответствие узлов координатам в .json.
    """
    os.makedirs(graph_folder, exist_ok=True)
    
    # 1. Получаем матрицу смежности из графа. 
    # В качестве значений берем 'length' (длину дороги в метрах)
    adj_matrix = nx.to_scipy_sparse_array(G_simple, weight='length')

    # 2. Сохраняем в формат Matrix Market (.mtx)
    mtx_path = os.path.join(graph_folder, f"{graph_name}.mtx")
    scipy.io.mmwrite(mtx_path, adj_matrix)
    print(f"Граф сохранен в {mtx_path}")

    # 3. Сохраняем соответствие индексов матрицы и реальных координат/ID узлов
    nodes_list = list(G_simple.nodes(data=True))
    node_mapping = {}
    for i, (node_id, data) in enumerate(nodes_list):
        node_mapping[i] = {
            "osm_id": node_id,
            "y": data['y'],  # Широта (Latitude)
            "x": data['x']   # Долгота (Longitude)
        }

    json_path = os.path.join(graph_folder, f"{graph_name}_nodes_mapping.json")
    with open(json_path, "w") as f:
        json.dump(node_mapping, f, indent=4)
    print(f"Координаты узлов сохранены в {json_path}")

    txt_path = os.path.join(graph_folder, f"{graph_name}_nodes_mapping.txt")
    with open(txt_path, "w") as f:
        for idx, data in node_mapping.items():
            f.write(f"{idx} {data['osm_id']} {data['y']} {data['x']}\n")
    print(f"Координаты узлов сохранены в {txt_path}")

if __name__ == "__main__":
    place_name = "Кстово, Нижегородская область, Россия"
    graph_name = "Nizhny_Novgorod"
    graph_folder = f"graphs/UNN/{graph_name}"
    
    G_simple = download_graph(place_name)
    save_graph_data(G_simple, graph_folder, graph_name)