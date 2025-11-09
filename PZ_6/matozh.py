import pandas as pd
import numpy as np

# Данные из файла
data = [
    1, 1, 2, 2, 3
]

# Преобразуем в numpy array для удобства
data_array = np.array(data)

# 1. Математическое ожидание (среднее)
mean_value = np.mean(data_array)
print(f"Математическое ожидание (среднее): {mean_value}")

# 2. Дисперсия (смещенная и несмещенная)
variance_biased = np.var(data_array)  # смещенная (делит на n)
variance_unbiased = np.var(data_array, ddof=1)  # несмещенная (делит на n-1)

print(f"Дисперсия (смещенная): {variance_biased}")
print(f"Дисперсия (несмещенная): {variance_unbiased}")

# 3. Подсчет повторяющихся значений
from collections import Counter

counter = Counter(data)
repeating_values = {value: count for value, count in counter.items() if count > 1}
unique_repeating_count = len(repeating_values)

print(f"\nКоличество повторяющихся уникальных значений: {unique_repeating_count}")
print("Повторяющиеся значения и их частота:")
for value, count in sorted(repeating_values.items()):
    print(f"  {value}: {count} раз(а)")

# 4. Альтернативный расчет "вручную" для проверки
n = len(data_array)
manual_mean = sum(data_array) / n
manual_variance_unbiased = sum((x - manual_mean) ** 2 for x in data_array) / (n - 1)

print(f"\nПроверка вручную:")
print(f"Среднее: {manual_mean}")
print(f"Дисперсия (несмещенная): {manual_variance_unbiased}")