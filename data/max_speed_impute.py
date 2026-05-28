import pandas as pd

# 1. Cargar el dataset de aristas
edges = pd.read_csv("edges.csv")

# Forzar maxspeed a numérico (convirtiendo textos raros a NaN) y homogeneizar los nulos a 0
edges["maxspeed"] = pd.to_numeric(edges["maxspeed"], errors="coerce").fillna(0)

# 2. Identificar qué filas tienen velocidad 0 (nuestra condición a reparar)
missing_speed = edges["maxspeed"] == 0

# 3. Calcular la mediana por fclass utilizando SOLO las velocidades válidas (> 0)
valid_speeds = edges[~missing_speed]
medians_by_fclass = valid_speeds.groupby("fclass")["maxspeed"].median()

# 4. Mapear las medianas calculadas a las filas que tienen velocidad 0
# Esto buscará la mediana correspondiente al fclass de cada fila sin velocidad
imputed_values = edges.loc[missing_speed, "fclass"].map(medians_by_fclass)

# 5. Identificar casos donde no existía una mediana previa (por ejemplo, todas las calles de ese fclass tenían 0)
unresolved = imputed_values.isna()
unresolved_count = unresolved.sum()

# Llenar con 0 aquellos que no pudieron ser mapeados para dejarlos intactos
imputed_values = imputed_values.fillna(0)

# 6. Actualizar la columna original solo en las filas modificadas
edges.loc[missing_speed, "maxspeed"] = imputed_values

# 7. Sobrescribir el CSV con los datos limpios
edges.to_csv("edges.csv", index=False)

# --- Reporte final por consola ---
print("--- Resumen de Imputación de Maxspeed ---")
print(f"Registros totales en el CSV: {len(edges)}")
print(f"Velocidades imputadas con éxito: {missing_speed.sum() - unresolved_count}")
print(f"Casos sin mediana (dejados en 0 para revisión manual): {unresolved_count}")

if unresolved_count > 0:
    print("\nTipos de vía (fclass) que no tienen ninguna velocidad registrada en el dataset para sacar mediana:")
    fclass_unresolved = edges.loc[missing_speed & unresolved, "fclass"].unique()
    for fc in fclass_unresolved:
        count = (edges["fclass"] == fc).sum()
        print(f" - {fc} ({count} registros)")
