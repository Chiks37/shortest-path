import sys
import os
import gzip

def process_file(co_filename):
    # Извлекаем директорию из пути файла, чтобы сохранить результат туда же
    dir_name = os.path.dirname(os.path.abspath(co_filename))
    
    # Формируем красивое имя для выходного файла
    base_name = os.path.basename(co_filename).replace('.co.gz', '').replace('.co', '')
    # Если имя слишком длинное (типа USA-road-d.USA), можно жестко задать 'road_usa'
    if "USA" in base_name:
        base_name = "road_usa"
        
    out_filename = os.path.join(dir_name, f"{base_name}_nodes_mapping.txt")

    # Автоматически определяем, сжат ли файл
    open_func = gzip.open if co_filename.endswith('.gz') else open

    print(f"Читаем файл {co_filename}...")
    
    try:
        with open_func(co_filename, 'rt') as f, open(out_filename, 'w') as out_f:
            for line in f:
                # Строки с координатами начинаются с буквы 'v'
                if line.startswith('v '):
                    parts = line.split()
                    
                    # parts[0] = 'v'
                    # parts[1] = ID вершины (начинается с 1)
                    # parts[2] = Долгота (X) * 1 000 000
                    # parts[3] = Широта (Y) * 1 000 000
                    
                    node_id = int(parts[1])
                    idx = node_id - 1  # Переводим в 0-based индекс для совпадения с матрицей
                    
                    # В DIMACS координаты умножены на миллион, чтобы быть целыми числами.
                    # Делим обратно, чтобы получить нормальные float координаты.
                    x = float(parts[2]) / 1000000.0
                    y = float(parts[3]) / 1000000.0
                    
                    # Формат строки: idx 0 0 0 x y
                    out_f.write(f"{idx} 0 0 0 {x} {y}\n")
                    
        print(f"Готово! Координаты узлов успешно сохранены в {out_filename}")
        return out_filename
        
    except FileNotFoundError:
        print(f"Ошибка: Файл {co_filename} не найден.")
    except Exception as e:
        print(f"Произошла ошибка: {e}")
        
    return None

def main():
    if len(sys.argv) < 2:
        print("Использование: python parse_dimacs_co.py <файл.co или файл.co.gz>")
        sys.exit(1)

    co_filename = sys.argv[1]
    process_file(co_filename)

if __name__ == "__main__":
    main()