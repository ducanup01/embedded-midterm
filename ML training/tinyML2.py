import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler
import tensorflow as tf
import numpy as np
from sklearn.metrics import classification_report, confusion_matrix

PREFIX = "embedded_AI"

# 1️⃣ Load dataset
data = pd.read_csv("merged_data.csv")

# 2️⃣ Split features (X) and labels (y)
X = data[["brightness", "temperature", "humidity", "motion_detected"]].values
y = data["output"].values

# 3️⃣ Normalize X between 0 and 1
scaler = MinMaxScaler()
X_scaled = scaler.fit_transform(X)

# 4️⃣ Split into train and test sets
X_train, X_test, y_train, y_test = train_test_split(
    X_scaled, y, test_size=0.2, random_state=42
)

# 5️⃣ Define model
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(4,)),
    tf.keras.layers.Dense(16, activation='relu'),
    tf.keras.layers.Dense(8, activation='relu'),
    tf.keras.layers.Dense(1, activation='sigmoid')
])

model.compile(
    loss="binary_crossentropy",
    optimizer="adam",
    metrics=["accuracy"]
)

# 6️⃣ Train model
model.fit(X_train, y_train, epochs=50, validation_data=(X_test, y_test))

# 7️⃣ Save .h5 model
model.save(PREFIX + ".h5")

# 8️⃣ Convert to TensorFlow Lite model
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

with open(PREFIX + ".tflite", "wb") as f:
    f.write(tflite_model)

# 9️⃣ Convert to C header (.h)
tflite_path = PREFIX + ".tflite"
output_header_path = PREFIX + ".h"

with open(tflite_path, "rb") as tflite_file:
    tflite_content = tflite_file.read()

hex_lines = [
    ', '.join([f'0x{byte:02x}' for byte in tflite_content[i:i + 12]])
    for i in range(0, len(tflite_content), 12)
]
hex_array = ',\n  '.join(hex_lines)

with open(output_header_path, "w") as header_file:
    header_file.write('const unsigned char model[] = {\n  ')
    header_file.write(f'{hex_array}\n')
    header_file.write('};\n\n')

# 🔟 Save scaler constants for ESP-side normalization
scaling_info = {
    "min": scaler.data_min_.tolist(),
    "max": scaler.data_max_.tolist()
}
np.save(PREFIX + "_scaler.npy", scaling_info)

print("✅ Model trained and saved as:")
print(f"   - {PREFIX}.h5")
print(f"   - {PREFIX}.tflite")
print(f"   - {PREFIX}.h")
print(f"   - {PREFIX}_scaler.npy (contains min/max for normalization)")

print("\n📏 Scaling constants:")
print("Min values:", scaler.data_min_)
print("Max values:", scaler.data_max_)

# Predict on test set
y_pred_prob = model.predict(X_test)
y_pred = (y_pred_prob > 0.5).astype(int)  # threshold = 0.5

# Compute metrics
print("\n📊 Classification Report:")
print(classification_report(y_test, y_pred))

print("\n🧩 Confusion Matrix:")
print(confusion_matrix(y_test, y_pred))
