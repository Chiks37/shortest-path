import sys
import os
import argparse

def convert_coord_to_mapping(input_filepath: str):
    """
    Reads a NAME_coord.mtx file and creates a NAME_node_mapping.txt 
    in the same directory with format:
    id 0 0 0 x y
    """
    if not input_filepath.endswith('_coord.mtx'):
        print(f"Предупреждение: ожидалось, что файл заканчивается на '_coord.mtx', получено: {input_filepath}")
        
    output_filepath = input_filepath.replace('_coord.mtx', '_node_mapping.txt')
    # Если замена не произошла (например, файл называется иначе), просто добавим суффикс
    if output_filepath == input_filepath:
        output_filepath = os.path.splitext(input_filepath)[0] + '_node_mapping.txt'

    try:
        with open(input_filepath, 'r') as fin:
            lines = [line.strip() for line in fin if line.strip() and not line.startswith('%')]
            
        if not lines:
            print("Файл пуст или содержит только комментарии.")
            return None
            
        header = lines[0].split()
        num_rows = int(header[0])
        num_cols = int(header[1]) if len(header) > 1 else 1
        
        data = lines[1:]
        
        # MatrixMarket array format is typically column-major.
        # So first num_rows are X, next num_rows are Y
        
        with open(output_filepath, 'w') as fout:
            for i in range(num_rows):
                if num_cols >= 2:
                    # Column-major array format
                    x = data[i]
                    y = data[i + num_rows]
                elif len(data[i].split()) >= 2:
                    # In case it's not array format but coordinate format or just rows
                    parts = data[i].split()
                    x = parts[0]
                    y = parts[1]
                else:
                    x = data[i]
                    y = "0"
                    
                fout.write(f"{i} 0 0 0 {x} {y}\n")
                    
        print(f"Успешно конвертировано: {output_filepath}")
        return output_filepath
    except Exception as e:
        print(f"Ошибка при обработке {input_filepath}: {e}")
        return None

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Конвертация файла NAME_coord.mtx в NAME_node_mapping.txt")
    parser.add_argument("input_file", help="Путь к входному файлу NAME_coord.mtx")
    
    args = parser.parse_args()
    
    if not os.path.exists(args.input_file):
        print(f"Ошибка: Файл {args.input_file} не найден.")
        sys.exit(1)
        
    convert_coord_to_mapping(args.input_file)
