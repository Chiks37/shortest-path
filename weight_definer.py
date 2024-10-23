import random

def add_random_number(filename):
  """
  Добавляет случайное число в диапазоне от 1 до 100 к каждому ряду чисел 
  в конце файла, пропуская первый ряд.
  """

  with open(filename, 'r+') as f:
    lines = f.readlines()
    f.seek(0)  # Перемещаем указатель в начало файла для перезаписи

    # Обрабатываем первый ряд (он содержит информацию, не числа)
    f.write(lines[0])

    # Обрабатываем остальные ряды, добавляя случайное число
    for i in range(1, len(lines)):
      parts = lines[i].split()
      if all(part.isdigit() for part in parts):  # Проверяем, есть ли в строке только числа
        random_number = random.randint(1, 100)
        f.write(' '.join(parts) + ' ' + str(random_number) + '\n')
      else:
        f.write(lines[i])

# Запускаем скрипт
add_random_number('graphs/test_graph.mtx')