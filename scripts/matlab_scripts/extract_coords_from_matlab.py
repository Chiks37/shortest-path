import sys
import os
import scipy.io
import numpy as np

def find_coords_by_shape(obj, num_nodes):
    """Рекурсивно ищет массив с координатами размера (N, 2) или (2, N)"""
    if isinstance(obj, np.ndarray):
        if obj.ndim == 2:
            if obj.shape[0] == num_nodes and obj.shape[1] in (2, 3):
                return obj
            elif obj.shape[1] == num_nodes and obj.shape[0] in (2, 3):
                return obj.T
        if obj.dtype.names:
            for name in obj.dtype.names:
                res = find_coords_by_shape(obj[name], num_nodes)
                if res is not None: return res
        elif obj.dtype == object:
            for item in obj.flat:
                res = find_coords_by_shape(item, num_nodes)
                if res is not None: return res
    elif isinstance(obj, dict):
        for key, val in obj.items():
            res = find_coords_by_shape(val, num_nodes)
            if res is not None: return res
    return None

def main():
    if len(sys.argv) < 2:
        print("Использование: python extract_coords.py NAME.mat")
        sys.exit(1)

    mat_filename = sys.argv[1]
    base_name = os.path.splitext(os.path.basename(mat_filename))[0]
    out_filename = f"{base_name}_nodes_mapping.txt"

    try:
        data = scipy.io.loadmat(mat_filename)
    except Exception as e:
        print(f"Ошибка при чтении файла {mat_filename}: {e}")
        sys.exit(1)

    try:
        problem = data['Problem'][0][0]
    except KeyError:
        print("Ошибка: В файле нет структуры 'Problem'.")
        sys.exit(1)

    # 1. Узнаем количество вершин из матрицы A
    num_nodes = None
    if 'A' in problem.dtype.names:
        A = problem['A']
        while isinstance(A, np.ndarray) and A.dtype == object and A.size == 1:
            A = A[0]
        if hasattr(A, 'shape'):
            num_nodes = A.shape[0]

    if num_nodes is None:
        print("Не удалось определить количество вершин (матрица A не найдена).")
        sys.exit(1)

    # 2. Ищем координаты
    coords = None

    # Попытка А: Стандартный путь SuiteSparse
    if 'aux' in problem.dtype.names:
        aux = problem['aux']
        while isinstance(aux, np.ndarray) and aux.dtype == object and aux.size == 1:
            aux = aux[0]
        if hasattr(aux, 'dtype') and aux.dtype.names and 'coord' in aux.dtype.names:
            coords = aux['coord']
            while isinstance(coords, np.ndarray) and coords.dtype == object and coords.size == 1:
                coords = coords[0]

    # Попытка Б: Умный поиск по всему файлу
    if coords is None:
        coords = find_coords_by_shape(data, num_nodes)

    # 3. Сохраняем результат
    if coords is not None:
        try:
            with open(out_filename, "w") as f:
                for idx, row in enumerate(coords):
                    x = row[0]
                    y = row[1]
                    f.write(f"{idx} 0 0 0 {x} {y}\n")
            print(f"Координаты узлов успешно сохранены в {out_filename}")
        except Exception as e:
            print(f"Ошибка при записи в файл: {e}")
    else:
        print(f"Ошибка: Не удалось найти координаты в файле {mat_filename}.")
        print(f"Ожидался массив размера ({num_nodes}, 2).")
        print("Доступные поля в структуре Problem:", problem.dtype.names)
        print("\nПохоже, авторы SuiteSparse забыли включить координаты именно в этот .mat файл.")
        print("Решение: Скачайте оригинальный файл координат (.co) с сайта DIMACS:")
        print("http://www.diag.uniroma1.it//challenge9/download.shtml")

if __name__ == "__main__":
    main()