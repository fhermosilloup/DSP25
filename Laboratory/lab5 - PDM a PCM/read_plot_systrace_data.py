import pandas as pd
import matplotlib.pyplot as plt

# Load the CSV file without headers
filename = "SWV_Data_Trace.txt"

# Read CSV with semicolon as delimiter, ignoring the first and third columns
df = pd.read_csv(filename, delimiter=";", usecols=[1, 3], names=["PCM", "Timestamp"])

# Convert PCM values to integer
y = pd.to_numeric(df["PCM"], errors="coerce")

# Convert timestamps to miliseconds
x = 1000*pd.to_numeric(df["Timestamp"], errors="coerce")/168000000.0

# Plot PCM data over sample index
plt.figure(figsize=(10, 4))
plt.plot(x, y, label="PCM Data")
plt.xlabel("Time [ms]")
plt.ylabel("Amplitude")
plt.title("PCM Data Captured from SysTrace")
plt.legend()
plt.grid()
plt.show()